// SPDX-FileCopyrightText: 2017 - 2024 The Ginkgo authors
//
// SPDX-License-Identifier: BSD-3-Clause

#ifndef GKO_DPCPP_MATRIX_BATCH_CSR_KERNELS_HPP_
#define GKO_DPCPP_MATRIX_BATCH_CSR_KERNELS_HPP_


#include <memory>

#include <CL/sycl.hpp>

#include "core/base/batch_struct.hpp"
#include "core/matrix/batch_struct.hpp"
#include "dpcpp/base/batch_struct.hpp"
#include "dpcpp/base/config.hpp"
#include "dpcpp/base/dim3.dp.hpp"
#include "dpcpp/base/dpct.hpp"
#include "dpcpp/base/helper.hpp"
#include "dpcpp/components/cooperative_groups.dp.hpp"
#include "dpcpp/components/thread_ids.dp.hpp"
#include "dpcpp/matrix/batch_struct.hpp"


namespace gko {
namespace kernels {
namespace GKO_DEVICE_NAMESPACE {
namespace batch_single_kernels {


template <typename ValueType, typename IndexType>
__dpct_inline__ void simple_apply(
    const gko::batch::matrix::csr::batch_item<const ValueType, IndexType>& mat,
    const ValueType* b, ValueType* x, sycl::nd_item<3>& item_ct1)
{
    const auto num_rows = mat.num_rows;
    const auto val = mat.values;
    const auto col = mat.col_idxs;
    for (int row = item_ct1.get_local_linear_id(); row < num_rows;
         row += item_ct1.get_local_range().size()) {
        auto temp = zero<ValueType>();
        for (auto nnz = mat.row_ptrs[row]; nnz < mat.row_ptrs[row + 1]; nnz++) {
            const auto col_idx = col[nnz];
            temp += val[nnz] * b[col_idx];
        }
        x[row] = temp;
    }
}


template <typename ValueType, typename IndexType>
__dpct_inline__ void advanced_apply(
    const ValueType alpha,
    const gko::batch::matrix::csr::batch_item<const ValueType, IndexType>& mat,
    const ValueType* b, const ValueType beta, ValueType* x,
    sycl::nd_item<3>& item_ct1)
{
    const auto num_rows = mat.num_rows;
    const auto val = mat.values;
    const auto col = mat.col_idxs;
    for (int row = item_ct1.get_local_linear_id(); row < num_rows;
         row += item_ct1.get_local_range().size()) {
        auto temp = zero<ValueType>();
        for (auto nnz = mat.row_ptrs[row]; nnz < mat.row_ptrs[row + 1]; nnz++) {
            const auto col_idx = col[nnz];
            temp += val[nnz] * b[col_idx];
        }
        x[row] = alpha * temp + beta * x[row];
    }
}


template <typename ValueType, typename IndexType>
__dpct_inline__ void scale(const int num_rows, const ValueType* const col_scale,
                           const ValueType* const row_scale,
                           const IndexType* const col_idxs,
                           const IndexType* const row_ptrs,
                           ValueType* const values, sycl::nd_item<3>& item_ct1)
{
    for (int row = item_ct1.get_local_linear_id(); row < num_rows;
         row += item_ct1.get_local_range().size()) {
        const ValueType row_scalar = row_scale[row];
        for (auto nnz = row_ptrs[row]; nnz < row_ptrs[row + 1]; nnz++) {
            values[nnz] *= row_scalar * col_scale[col_idxs[nnz]];
        }
    }
}


template <typename ValueType, typename IndexType>
__dpct_inline__ void add_scaled_identity(
    const ValueType alpha, const ValueType beta,
    const gko::batch::matrix::csr::batch_item<ValueType, IndexType>& mat,
    sycl::nd_item<3>& item_ct1)
{
    for (int row = item_ct1.get_local_linear_id(); row < mat.num_rows;
         row += item_ct1.get_local_range().size()) {
        for (auto nnz = mat.row_ptrs[row]; nnz < mat.row_ptrs[row + 1]; nnz++) {
            auto col = mat.col_idxs[nnz];
            mat.values[nnz] *= beta;
            if (row == col) {
                mat.values[nnz] += alpha;
            }
        }
    }
}


}  // namespace batch_single_kernels
}  // namespace GKO_DEVICE_NAMESPACE
}  // namespace kernels
}  // namespace gko


#endif
