// SPDX-FileCopyrightText: 2017 - 2024 The Ginkgo authors
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef GKO_PUBLIC_CORE_FACTORIZATION_PAR_ILU_HPP_
#define GKO_PUBLIC_CORE_FACTORIZATION_PAR_ILU_HPP_


#include <memory>

#include <ginkgo/core/base/composition.hpp>
#include <ginkgo/core/base/lin_op.hpp>
#include <ginkgo/core/base/types.hpp>
#include <ginkgo/core/config/config.hpp>
#include <ginkgo/core/config/registry.hpp>
#include <ginkgo/core/matrix/csr.hpp>


namespace gko {
/**
 * @brief The Factorization namespace.
 *
 * @ingroup factor
 */
namespace factorization {


/**
 * ParILU is an incomplete LU factorization which is computed in parallel.
 *
 * $L$ is a lower unitriangular, while $U$ is an upper triangular matrix, which
 * approximate a given matrix $A$ with $A \approx LU$. Here, $L$ and $U$ have
 * the same sparsity pattern as $A$, which is also called ILU(0).
 *
 * The ParILU algorithm generates the incomplete factors iteratively, using a
 * fixed-point iteration of the form
 *
 * $
 * F(L, U) =
 * \begin{cases}
 *     \frac{1}{u_{jj}}
 *         \left(a_{ij}-\sum_{k=1}^{j-1}l_{ik}u_{kj}\right), \quad & i>j \\
 *     a_{ij}-\sum_{k=1}^{i-1}l_{ik}u_{kj}, \quad & i\leq j
 * \end{cases}
 * $
 *
 * In general, the entries of $L$ and $U$ can be iterated in parallel and in
 * asynchronous fashion, the algorithm asymptotically converges to the
 * incomplete factors $L$ and $U$ fulfilling $\left(R = A - L \cdot
 * U\right)\vert_\mathcal{S} = 0\vert_\mathcal{S}$ where $\mathcal{S}$ is the
 * pre-defined sparsity pattern (in case of ILU(0) the sparsity pattern of the
 * system matrix $A$). The number of ParILU sweeps needed for convergence
 * depends on the parallelism level: For sequential execution, a single sweep
 * is sufficient, for fine-grained parallelism, the number of sweeps necessary
 * to get a good approximation of the incomplete factors depends heavily on the
 * problem. On the OpenMP executor, 3 sweeps usually give a decent approximation
 * in our experiments, while GPU executors can take 10 or more iterations.
 *
 * The ParILU algorithm in Ginkgo follows the design of E. Chow and A. Patel,
 * Fine-grained Parallel Incomplete LU Factorization, SIAM Journal on Scientific
 * Computing, 37, C169-C193 (2015).
 *
 * @tparam ValueType  Type of the values of all matrices used in this class
 * @tparam IndexType  Type of the indices of all matrices used in this class
 *
 * @ingroup factor
 * @ingroup LinOp
 */
template <typename ValueType = default_precision, typename IndexType = int32>
class ParIlu : public Composition<ValueType> {
public:
    using value_type = ValueType;
    using index_type = IndexType;
    using matrix_type = matrix::Csr<ValueType, IndexType>;
    using l_matrix_type = matrix_type;
    using u_matrix_type = matrix_type;

    std::shared_ptr<const matrix_type> get_l_factor() const
    {
        // Can be `static_cast` since the type is guaranteed in this class
        return std::static_pointer_cast<const matrix_type>(
            this->get_operators()[0]);
    }

    std::shared_ptr<const matrix_type> get_u_factor() const
    {
        // Can be `static_cast` since the type is guaranteed in this class
        return std::static_pointer_cast<const matrix_type>(
            this->get_operators()[1]);
    }

    // Remove the possibility of calling `create`, which was enabled by
    // `Composition`
    template <typename... Args>
    static std::unique_ptr<Composition<ValueType>> create(Args&&... args) =
        delete;

    GKO_CREATE_FACTORY_PARAMETERS(parameters, Factory)
    {
        /**
         * The number of iterations the `compute` kernel will use when doing
         * the factorization. The default value `0` means `Auto`, so the
         * implementation decides on the actual value depending on the
         * resources that are available.
         */
        size_type GKO_FACTORY_PARAMETER_SCALAR(iterations, 0);

        /**
         * The `system_matrix`, which will be given to this factory, must be
         * sorted (first by row, then by column) in order for the algorithm
         * to work. If it is known that the matrix will be sorted, this
         * parameter can be set to `true` to skip the sorting (therefore,
         * shortening the runtime).
         * However, if it is unknown or if the matrix is known to be not sorted,
         * it must remain `false`, otherwise, the factorization might be
         * incorrect.
         */
        bool GKO_FACTORY_PARAMETER_SCALAR(skip_sorting, false);

        /**
         * Strategy which will be used by the L matrix. The default value
         * `nullptr` will result in the strategy `classical`.
         */
        std::shared_ptr<typename matrix_type::strategy_type>
            GKO_FACTORY_PARAMETER_SCALAR(l_strategy, nullptr);

        /**
         * Strategy which will be used by the U matrix. The default value
         * `nullptr` will result in the strategy `classical`.
         */
        std::shared_ptr<typename matrix_type::strategy_type>
            GKO_FACTORY_PARAMETER_SCALAR(u_strategy, nullptr);
    };
    GKO_ENABLE_LIN_OP_FACTORY(ParIlu, parameters, Factory);
    GKO_ENABLE_BUILD_METHOD(Factory);

    /**
     * Create the parameters from the property_tree.
     * Because this is directly tied to the specific type, the value/index type
     * settings within config are ignored and type_descriptor is only used
     * for children configs.
     *
     * @param config  the property tree for setting
     * @param context  the registry
     * @param td_for_child  the type descriptor for children configs. The
     *                      default uses the value/index type of this class.
     *
     * @return parameters
     */
    static parameters_type parse(
        const config::pnode& config, const config::registry& context,
        const config::type_descriptor& td_for_child =
            config::make_type_descriptor<ValueType, IndexType>());

protected:
    explicit ParIlu(const Factory* factory,
                    std::shared_ptr<const LinOp> system_matrix)
        : Composition<ValueType>(factory->get_executor()),
          parameters_{factory->get_parameters()}
    {
        if (parameters_.l_strategy == nullptr) {
            parameters_.l_strategy =
                std::make_shared<typename matrix_type::classical>();
        }
        if (parameters_.u_strategy == nullptr) {
            parameters_.u_strategy =
                std::make_shared<typename matrix_type::classical>();
        }
        generate_l_u(system_matrix, parameters_.skip_sorting,
                     parameters_.l_strategy, parameters_.u_strategy)
            ->move_to(this);
    }

    /**
     * Generates the incomplete LU factors, which will be returned as a
     * composition of the lower (first element of the composition) and the
     * upper factor (second element). The dynamic type of L is l_matrix_type,
     * while the dynamic type of U is u_matrix_type.
     *
     * @param system_matrix  the source matrix used to generate the factors.
     *                       @note: system_matrix must be convertible to a Csr
     *                              Matrix, otherwise, an exception is thrown.
     * @param skip_sorting  if set to `true`, the sorting will be skipped.
     *                      @note: If the matrix is not sorted, the
     *                             factorization fails.
     * @param l_strategy  Strategy, which will be used by the L matrix.
     * @param u_strategy  Strategy, which will be used by the U matrix.
     * @return  A Composition, containing the incomplete LU factors for the
     *          given system_matrix (first element is L, then U)
     */
    std::unique_ptr<Composition<ValueType>> generate_l_u(
        const std::shared_ptr<const LinOp>& system_matrix, bool skip_sorting,
        std::shared_ptr<typename matrix_type::strategy_type> l_strategy,
        std::shared_ptr<typename matrix_type::strategy_type> u_strategy) const;
};


}  // namespace factorization
}  // namespace gko


#endif  // GKO_PUBLIC_CORE_FACTORIZATION_PAR_ILU_HPP_
