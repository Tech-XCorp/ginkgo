/*******************************<GINKGO LICENSE>******************************
Copyright (c) 2017-2020, the Ginkgo authors
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************<GINKGO LICENSE>*******************************/

#ifndef GKO_CORE_PRECONDITIONER_ISAI_HPP_
#define GKO_CORE_PRECONDITIONER_ISAI_HPP_


#include <memory>


#include <ginkgo/core/base/composition.hpp>  // TODO: check if still needed
#include <ginkgo/core/base/exception_helpers.hpp>
#include <ginkgo/core/base/executor.hpp>
#include <ginkgo/core/base/lin_op.hpp>


/** TODO:
 * - implement only_l and only_u, or remove it
 * - Should it be possible to create a preconditioner exclusively for L or U?
 * - Write documentation for Isai class (referencing Hartwig's paper and
 *   mention left vs. right preconditioning)!
 */


namespace gko {
/**
 * @brief The Preconditioner namespace.
 *
 * @ingroup precond
 */
namespace preconditioner {


/**
 * Incomplete Sparse Approximate Inverse (ISAI)
 *
 * @ingroup isai
 * @ingroup precond
 * @ingroup LinOp
 */
template <typename ValueType = default_precision, typename IndexType = int32>
class Isai : public EnableLinOp<Isai<ValueType, IndexType>> {
    friend class EnableLinOp<Isai>;
    friend class EnablePolymorphicObject<Isai, LinOp>;

public:
    using value_type = ValueType;
    using index_type = IndexType;

    template <typename... Args>
    static std::unique_ptr<Composition<ValueType>> create(Args &&... args) =
        delete;

    GKO_CREATE_FACTORY_PARAMETERS(parameters, Factory)
    {
        // std::shared_ptr<LinOpFactory> GKO_FACTORY_PARAMETER(lu_factory,
        // nullptr);
        // TODO figure out if it makes sense and implement properly
        bool GKO_FACTORY_PARAMETER(only_l, false);
        bool GKO_FACTORY_PARAMETER(only_u, false);
    };

    GKO_ENABLE_LIN_OP_FACTORY(Isai, parameters, Factory);
    GKO_ENABLE_BUILD_METHOD(Factory);

protected:
    explicit Isai(std::shared_ptr<const Executor> exec)
        : EnableLinOp<Isai>(std::move(exec))
    {}

    /**
     * Creates an Isai preconditioner from a matrix using an Isai::Factory.
     *
     * @param factory  the factory to use to create the preconditoner
     * @param system_matrix  the matrix this preconditioner should be created
     *                       from
     */
    explicit Isai(const Factory *factory,
                  std::shared_ptr<const LinOp> system_matrices)
        : EnableLinOp<Isai>(factory->get_executor()),
          parameters_{factory->get_parameters()}
    {
        // GKO_ASSERT_IS_SQUARE_MATRIX(system_matrix);
        auto factors =
            dynamic_cast<const Composition<ValueType> *>(system_matrices.get());
        if (!factors) {
            GKO_NOT_SUPPORTED(system_matrices);
        }
        auto l_factor = factors->get_operators()[0];
        auto u_factor = factors->get_operators()[1];
        GKO_ASSERT_IS_SQUARE_MATRIX(l_factor);
        GKO_ASSERT_IS_SQUARE_MATRIX(u_factor);
        auto inv_l = this->generate_l(l_factor.get());
        auto inv_u = this->generate_u(u_factor.get());
        factors_ =
            Composition<ValueType>::create(std::move(inv_l), std::move(inv_u));
    }

    void apply_impl(const LinOp *b, LinOp *x) const override
    {
        factors_->apply(b, x);
    }

    void apply_impl(const LinOp *alpha, const LinOp *b, const LinOp *beta,
                    LinOp *x) const override
    {
        factors_->apply(alpha, b, beta, x);
    }

    /**
     * Generates the approximate inverse of a lower triangular matrix
     *
     * @param to_invert_l  the source lower triangular matrix used to generate
     *                     the approximate inverse
     */
    std::shared_ptr<LinOp> generate_l(const LinOp *to_invert_l);

    /**
     * Generates the approximate inverse.
     *
     * @param to_invert_u  the source upper triangular matrix used to generate
     *                     the approximate inverse
     */
    std::shared_ptr<LinOp> generate_u(const LinOp *to_invert_u);

private:
    std::shared_ptr<Composition<ValueType>> factors_;
};


}  // namespace preconditioner
}  // namespace gko


#endif  // GKO_CORE_PRECONDITIONER_ISAI_HPP_
