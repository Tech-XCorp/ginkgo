// SPDX-FileCopyrightText: 2017 - 2024 The Ginkgo authors
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef GKO_PUBLIC_CORE_MATRIX_ROW_GATHERER_HPP_
#define GKO_PUBLIC_CORE_MATRIX_ROW_GATHERER_HPP_


#include <algorithm>
#include <memory>
#include <numeric>
#include <vector>

#include <ginkgo/core/base/array.hpp>
#include <ginkgo/core/base/exception.hpp>
#include <ginkgo/core/base/exception_helpers.hpp>
#include <ginkgo/core/base/executor.hpp>
#include <ginkgo/core/base/lin_op.hpp>
#include <ginkgo/core/base/types.hpp>
#include <ginkgo/core/base/utils.hpp>


namespace gko {
namespace matrix {


/**
 * RowGatherer is a matrix "format" which stores the gather indices arrays which
 * can be used to gather rows to another matrix.
 *
 * @tparam IndexType  precision of rowgatherer array indices.
 *
 * @note This format is used mainly to allow for an abstraction of the
 * rowgatherer and provides the user with an apply method which
 * calls the respective Dense rowgatherer operation. As such it only stores an
 * array of the rowgatherer indices.
 *
 * @ingroup rowgatherer
 * @ingroup matrix
 * @ingroup LinOp
 */
template <typename IndexType = int32>
class RowGatherer : public EnableLinOp<RowGatherer<IndexType>> {
    friend class EnablePolymorphicObject<RowGatherer, LinOp>;

public:
    using index_type = IndexType;

    /**
     * Returns a pointer to the row index array for gathering.
     *
     * @return the pointer to the row index array for gathering.
     */
    index_type* get_row_idxs() noexcept { return row_idxs_.get_data(); }

    /**
     * @copydoc get_row_idxs()
     *
     * @note This is the constant version of the function, which can be
     *       significantly more memory efficient than the non-constant version,
     *       so always prefer this version.
     */
    const index_type* get_const_row_idxs() const noexcept
    {
        return row_idxs_.get_const_data();
    }

    /**
     * Creates uninitialized RowGatherer arrays of the specified size.
     *
     * @param exec  Executor associated to the matrix
     * @param size  size of the RowGatherable matrix
     *
     * @return A smart pointer to the newly created matrix.
     */
    static std::unique_ptr<RowGatherer> create(
        std::shared_ptr<const Executor> exec, const dim<2>& size = {});

    /**
     * Creates a RowGatherer matrix from an already allocated (and initialized)
     * row gathering array
     *
     * @param exec  Executor associated to the matrix
     * @param size  size of the rowgatherer array.
     * @param row_idxs array of rowgatherer array
     *
     * @note If `row_idxs` is not an rvalue, not an array of
     * IndexType, or is on the wrong executor, an internal copy will be created,
     * and the original array data will not be used in the matrix.
     *
     * @return A smart pointer to the newly created matrix.
     */
    static std::unique_ptr<RowGatherer> create(
        std::shared_ptr<const Executor> exec, const dim<2>& size,
        array<index_type> row_idxs);

    /**
     * Creates a constant (immutable) RowGatherer matrix from a constant array.
     *
     * @param exec  the executor to create the matrix on
     * @param size  the dimensions of the matrix
     * @param row_idxs  the gathered row indices  of the matrix
     * @returns A smart pointer to the constant matrix wrapping the input arrays
     *          (if they reside on the same executor as the matrix) or a copy of
     *          the arrays on the correct executor.
     */
    static std::unique_ptr<const RowGatherer> create_const(
        std::shared_ptr<const Executor> exec, const dim<2>& size,
        gko::detail::const_array_view<IndexType>&& row_idxs);

protected:
    RowGatherer(std::shared_ptr<const Executor> exec, const dim<2>& size = {});

    RowGatherer(std::shared_ptr<const Executor> exec, const dim<2>& size,
                array<index_type> row_idxs);

    void apply_impl(const LinOp* in, LinOp* out) const override;

    void apply_impl(const LinOp* alpha, const LinOp* in, const LinOp* beta,
                    LinOp* out) const override;

private:
    gko::array<index_type> row_idxs_;
};


}  // namespace matrix
}  // namespace gko


#endif  // GKO_PUBLIC_CORE_MATRIX_ROW_GATHERER_HPP_
