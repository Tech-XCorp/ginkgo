// SPDX-FileCopyrightText: 2017 - 2024 The Ginkgo authors
//
// SPDX-License-Identifier: BSD-3-Clause

#include "core/factorization/cholesky_kernels.hpp"

#include <algorithm>
#include <memory>
#include <numeric>

#include <ginkgo/core/matrix/csr.hpp>

#include "core/base/allocator.hpp"
#include "core/base/iterator_factory.hpp"
#include "core/components/fill_array_kernels.hpp"
#include "core/components/format_conversion_kernels.hpp"
#include "core/components/prefix_sum_kernels.hpp"
#include "core/factorization/elimination_forest.hpp"
#include "core/factorization/lu_kernels.hpp"
#include "core/matrix/csr_lookup.hpp"


namespace gko {
namespace kernels {
namespace reference {
/**
 * @brief The Cholesky namespace.
 *
 * @ingroup factor
 */
namespace cholesky {


template <typename ValueType, typename IndexType>
void symbolic_count(std::shared_ptr<const DefaultExecutor> exec,
                    const matrix::Csr<ValueType, IndexType>* mtx,
                    const factorization::elimination_forest<IndexType>& forest,
                    IndexType* row_nnz, array<IndexType>&)
{
    const auto num_rows = mtx->get_size()[0];
    const auto row_ptrs = mtx->get_const_row_ptrs();
    const auto cols = mtx->get_const_col_idxs();
    const auto parent = forest.parents.get_const_data();
    vector<bool> visited(num_rows, {exec});
    for (IndexType row = 0; row < num_rows; row++) {
        IndexType count{};
        visited.assign(num_rows, false);
        visited[row] = true;
        const auto row_begin = row_ptrs[row];
        const auto row_end = row_ptrs[row + 1];
        for (auto nz = row_begin; nz < row_end; nz++) {
            const auto col = cols[nz];
            if (col < row) {
                auto node = col;
                while (!visited[node]) {
                    visited[node] = true;
                    count++;
                    node = parent[node];
                }
            }
        }
        row_nnz[row] = count + 1;  // add diagonal entry
    }
}

GKO_INSTANTIATE_FOR_EACH_VALUE_AND_INDEX_TYPE(
    GKO_DECLARE_CHOLESKY_SYMBOLIC_COUNT);


template <typename ValueType, typename IndexType>
void symbolic_factorize(
    std::shared_ptr<const DefaultExecutor> exec,
    const matrix::Csr<ValueType, IndexType>* mtx,
    const factorization::elimination_forest<IndexType>& forest,
    matrix::Csr<ValueType, IndexType>* l_factor, const array<IndexType>&)
{
    const auto num_rows = mtx->get_size()[0];
    const auto row_ptrs = mtx->get_const_row_ptrs();
    const auto cols = mtx->get_const_col_idxs();
    const auto out_row_ptrs = l_factor->get_const_row_ptrs();
    const auto out_cols = l_factor->get_col_idxs();
    const auto parent = forest.parents.get_const_data();
    vector<bool> visited(num_rows, {exec});
    for (IndexType row = 0; row < num_rows; row++) {
        auto out_nz = out_row_ptrs[row];
        visited.assign(num_rows, false);
        visited[row] = true;
        const auto row_begin = row_ptrs[row];
        const auto row_end = row_ptrs[row + 1];
        for (auto nz = row_begin; nz < row_end; nz++) {
            const auto col = cols[nz];
            if (col < row) {
                auto node = col;
                while (!visited[node]) {
                    visited[node] = true;
                    out_cols[out_nz++] = node;
                    node = parent[node];
                }
            }
        }
        out_cols[out_nz] = row;
    }
}

GKO_INSTANTIATE_FOR_EACH_VALUE_AND_INDEX_TYPE(
    GKO_DECLARE_CHOLESKY_SYMBOLIC_FACTORIZE);


template <typename ValueType, typename IndexType>
void forest_from_factor(
    std::shared_ptr<const DefaultExecutor> exec,
    const matrix::Csr<ValueType, IndexType>* factors,
    gko::factorization::elimination_forest<IndexType>& forest)
{
    const auto row_ptrs = factors->get_const_row_ptrs();
    const auto col_idxs = factors->get_const_col_idxs();
    const auto num_rows = static_cast<IndexType>(factors->get_size()[0]);
    const auto parents = forest.parents.get_data();
    const auto children = forest.children.get_data();
    const auto child_ptrs = forest.child_ptrs.get_data();
    // filled with sentinel for unattached nodes
    std::fill_n(parents, num_rows, num_rows);
    for (IndexType row = 0; row < num_rows; row++) {
        const auto row_begin = row_ptrs[row];
        const auto row_end = row_ptrs[row + 1];
        for (auto nz = row_begin; nz < row_end; nz++) {
            const auto col = col_idxs[nz];
            // use the lower triangle, min row from column
            if (col < row) {
                parents[col] = std::min(parents[col], row);
            }
        }
    }
    // group by parent
    vector<IndexType> parents_copy(parents, parents + num_rows, {exec});
    std::iota(children, children + num_rows, 0);
    const auto it = detail::make_zip_iterator(parents_copy.begin(), children);
    std::stable_sort(it, it + num_rows);
    components::convert_idxs_to_ptrs(exec, parents_copy.data(), num_rows,
                                     num_rows + 1, child_ptrs);
}

GKO_INSTANTIATE_FOR_EACH_VALUE_AND_INDEX_TYPE(
    GKO_DECLARE_CHOLESKY_FOREST_FROM_FACTOR);


template <typename ValueType, typename IndexType>
void initialize(std::shared_ptr<const DefaultExecutor> exec,
                const matrix::Csr<ValueType, IndexType>* mtx,
                const IndexType* factor_lookup_offsets,
                const int64* factor_lookup_descs,
                const int32* factor_lookup_storage, IndexType* diag_idxs,
                IndexType* transpose_idxs,
                matrix::Csr<ValueType, IndexType>* factors)
{
    lu_factorization::initialize(exec, mtx, factor_lookup_offsets,
                                 factor_lookup_descs, factor_lookup_storage,
                                 diag_idxs, factors);
    // convert to COO
    const auto nnz = factors->get_num_stored_elements();
    array<IndexType> row_idx_array{exec, nnz};
    const auto row_idxs = row_idx_array.get_data();
    const auto col_idxs = factors->get_const_col_idxs();
    components::convert_ptrs_to_idxs(exec, factors->get_const_row_ptrs(),
                                     factors->get_size()[0], row_idxs);
    components::fill_seq_array(exec, transpose_idxs, nnz);
    // compute nonzero permutation for sparse transpose
    std::sort(transpose_idxs, transpose_idxs + nnz,
              [&](IndexType i, IndexType j) {
                  return std::tie(col_idxs[i], row_idxs[i]) <
                         std::tie(col_idxs[j], row_idxs[j]);
              });
}

GKO_INSTANTIATE_FOR_EACH_VALUE_AND_INDEX_TYPE(GKO_DECLARE_CHOLESKY_INITIALIZE);


template <typename ValueType, typename IndexType>
void factorize(std::shared_ptr<const DefaultExecutor> exec,
               const IndexType* lookup_offsets, const int64* lookup_descs,
               const int32* lookup_storage, const IndexType* diag_idxs,
               const IndexType* transpose_idxs,
               const factorization::elimination_forest<IndexType>& forest,
               matrix::Csr<ValueType, IndexType>* factors,
               array<int>& tmp_storage)
{
    const auto num_rows = factors->get_size()[0];
    const auto row_ptrs = factors->get_const_row_ptrs();
    const auto cols = factors->get_const_col_idxs();
    const auto vals = factors->get_values();
    for (size_type row = 0; row < num_rows; row++) {
        const auto row_begin = row_ptrs[row];
        const auto row_diag = diag_idxs[row];
        matrix::csr::device_sparsity_lookup<IndexType> lookup{
            row_ptrs, cols, lookup_offsets, lookup_storage, lookup_descs, row};
        for (auto lower_nz = row_begin; lower_nz < row_diag; lower_nz++) {
            const auto dep = cols[lower_nz];
            const auto dep_diag_idx = diag_idxs[dep];
            const auto dep_diag = vals[dep_diag_idx];
            const auto dep_end = row_ptrs[dep + 1];
            const auto scale = vals[lower_nz] / dep_diag;
            vals[lower_nz] = scale;
            for (auto dep_nz = dep_diag_idx + 1; dep_nz < dep_end; dep_nz++) {
                const auto col = cols[dep_nz];
                if (col < row) {
                    const auto val = vals[dep_nz];
                    const auto nz = row_begin + lookup.lookup_unsafe(col);
                    vals[nz] -= scale * val;
                }
            }
        }
        ValueType diag = vals[row_diag];
        for (auto lower_nz = row_begin; lower_nz < row_diag; lower_nz++) {
            const auto col = cols[lower_nz];
            diag -= squared_norm(vals[lower_nz]);
            // copy the lower triangular entries to the transpose
            vals[transpose_idxs[lower_nz]] = conj(vals[lower_nz]);
        }
        vals[row_diag] = sqrt(diag);
    }
}

GKO_INSTANTIATE_FOR_EACH_VALUE_AND_INDEX_TYPE(GKO_DECLARE_CHOLESKY_FACTORIZE);


}  // namespace cholesky
}  // namespace reference
}  // namespace kernels
}  // namespace gko
