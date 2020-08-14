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

#ifndef GKO_CORE_SOLVER_GMRES_MIXED_KERNELS_HPP_
#define GKO_CORE_SOLVER_GMRES_MIXED_KERNELS_HPP_


#include <ginkgo/core/base/array.hpp>
#include <ginkgo/core/base/math.hpp>
#include <ginkgo/core/base/types.hpp>
#include <ginkgo/core/matrix/dense.hpp>
#include <ginkgo/core/stop/stopping_status.hpp>


#include "core/base/extended_float.hpp"
#include "core/solver/gmres_mixed_accessor.hpp"


#define FINISH_ARNOLDI 3


/**
 * Instantiates a template for each value type with each lower precision type
 * supported by Ginkgo for GmresMixed.
 *
 * @param _macro  A macro which expands the template instantiation
 *                (not including the leading `template` specifier).
 *                Should take two arguments:
 *                1. the first will be used as the regular ValueType
 *                     (precisions supported by Ginkgo), and
 *                2. the second the value type of the reduced precision.
 */
// #if defined(__CUDACC__) || defined(__HIPCC__)
// #define GKO_INSTANTIATE_FOR_EACH_GMRES_MIXED_TYPE(_macro) \
//     template _macro(double, half);                  \
//     template _macro(double, float);                 \
//     template _macro(double, double)
// #else
#define GKO_UNPACK(...) __VA_ARGS__
#define GKO_INSTANTIATE_FOR_EACH_GMRES_MIXED_TYPE2(_macro)       \
    template _macro(double, double);                             \
    template _macro(double, float);                              \
    template _macro(double, int64);                              \
    template _macro(double, int32);                              \
    template _macro(double, int16);                              \
    template _macro(double, half);                               \
    template _macro(float, float, float >);                      \
    template _macro(float, half, float >);                       \
    template _macro(std::complex<double>, std::complex<double>); \
    template _macro(std::complex<double>, std::complex<float>);  \
    template _macro(std::complex<float>, std::complex<float>)

#define GKO_INSTANTIATE_FOR_EACH_GMRES_MIXED_TYPE(_macro)                    \
    template _macro(double, GKO_UNPACK(Accessor3d<double, double>));         \
    template _macro(double, GKO_UNPACK(Accessor3d<float, double>));          \
    template _macro(double, GKO_UNPACK(Accessor3d<int64, double>));          \
    template _macro(double, GKO_UNPACK(Accessor3d<int32, double>));          \
    template _macro(double, GKO_UNPACK(Accessor3d<int16, double>));          \
    template _macro(double, GKO_UNPACK(Accessor3d<half, double>));           \
    template _macro(float, GKO_UNPACK(Accessor3d<float, float>));            \
    template _macro(float, GKO_UNPACK(Accessor3d<half, float>));             \
    template _macro(                                                         \
        std::complex<double>,                                                \
        GKO_UNPACK(Accessor3d<std::complex<double>, std::complex<double>>)); \
    template _macro(                                                         \
        std::complex<double>,                                                \
        GKO_UNPACK(Accessor3d<std::complex<float>, std::complex<double>>));  \
    template _macro(                                                         \
        std::complex<float>,                                                 \
        GKO_UNPACK(Accessor3d<std::complex<float>, std::complex<float>>))
// #endif
// #undef GKO_UNPACK


namespace gko {
namespace kernels {
namespace gmres_mixed {


#define GKO_DECLARE_GMRES_MIXED_INITIALIZE_1_KERNEL(_type)                     \
    void initialize_1(                                                         \
        std::shared_ptr<const DefaultExecutor> exec,                           \
        const matrix::Dense<_type> *b,                                         \
        matrix::Dense<remove_complex<_type>> *b_norm,                          \
        matrix::Dense<_type> *residual, matrix::Dense<_type> *givens_sin,      \
        matrix::Dense<_type> *givens_cos, Array<stopping_status> *stop_status, \
        size_type krylov_dim)


#define GKO_DECLARE_GMRES_MIXED_INITIALIZE_2_KERNEL(_type1, _accessor)      \
    void initialize_2(std::shared_ptr<const DefaultExecutor> exec,          \
                      const matrix::Dense<_type1> *residual,                \
                      matrix::Dense<remove_complex<_type1>> *residual_norm, \
                      matrix::Dense<_type1> *residual_norm_collection,      \
                      matrix::Dense<remove_complex<_type1>> *arnoldi_norm,  \
                      _accessor krylov_bases,                               \
                      matrix::Dense<_type1> *next_krylov_basis,             \
                      Array<size_type> *final_iter_nums, size_type krylov_dim)


#define GKO_DECLARE_GMRES_MIXED_STEP_1_KERNEL(_type1, _accessor)              \
    void step_1(                                                              \
        std::shared_ptr<const DefaultExecutor> exec,                          \
        matrix::Dense<_type1> *next_krylov_basis,                             \
        matrix::Dense<_type1> *givens_sin, matrix::Dense<_type1> *givens_cos, \
        matrix::Dense<remove_complex<_type1>> *residual_norm,                 \
        matrix::Dense<_type1> *residual_norm_collection,                      \
        _accessor krylov_bases, matrix::Dense<_type1> *hessenberg_iter,       \
        matrix::Dense<_type1> *buffer_iter,                                   \
        const matrix::Dense<remove_complex<_type1>> *b_norm,                  \
        matrix::Dense<remove_complex<_type1>> *arnoldi_norm, size_type iter,  \
        Array<size_type> *final_iter_nums,                                    \
        const Array<stopping_status> *stop_status,                            \
        Array<stopping_status> *reorth_status, Array<size_type> *num_reorth,  \
        int *num_reorth_steps, int *num_reorth_vectors)


#define GKO_DECLARE_GMRES_MIXED_STEP_2_KERNEL(_type1, _accessor)       \
    void step_2(std::shared_ptr<const DefaultExecutor> exec,           \
                const matrix::Dense<_type1> *residual_norm_collection, \
                _accessor /*::const_type*/ krylov_bases,               \
                const matrix::Dense<_type1> *hessenberg,               \
                matrix::Dense<_type1> *y,                              \
                matrix::Dense<_type1> *before_preconditioner,          \
                const Array<size_type> *final_iter_nums)


#define GKO_DECLARE_ALL_AS_TEMPLATES                                    \
    template <typename ValueType>                                       \
    GKO_DECLARE_GMRES_MIXED_INITIALIZE_1_KERNEL(ValueType);             \
    template <typename ValueType, typename Accessor3d>                  \
    GKO_DECLARE_GMRES_MIXED_INITIALIZE_2_KERNEL(ValueType, Accessor3d); \
    template <typename ValueType, typename Accessor3d>                  \
    GKO_DECLARE_GMRES_MIXED_STEP_1_KERNEL(ValueType, Accessor3d);       \
    template <typename ValueType, typename Accessor3d>                  \
    GKO_DECLARE_GMRES_MIXED_STEP_2_KERNEL(ValueType, Accessor3d)


}  // namespace gmres_mixed


namespace omp {
namespace gmres_mixed {

GKO_DECLARE_ALL_AS_TEMPLATES;

}  // namespace gmres_mixed
}  // namespace omp


namespace cuda {
namespace gmres_mixed {

GKO_DECLARE_ALL_AS_TEMPLATES;

}  // namespace gmres_mixed
}  // namespace cuda


namespace reference {
namespace gmres_mixed {

GKO_DECLARE_ALL_AS_TEMPLATES;

}  // namespace gmres_mixed
}  // namespace reference


namespace hip {
namespace gmres_mixed {

GKO_DECLARE_ALL_AS_TEMPLATES;

}  // namespace gmres_mixed
}  // namespace hip


namespace dpcpp {
namespace gmres_mixed {

GKO_DECLARE_ALL_AS_TEMPLATES;

}  // namespace gmres_mixed
}  // namespace dpcpp


#undef GKO_DECLARE_ALL_AS_TEMPLATES


}  // namespace kernels
}  // namespace gko


#endif  // GKO_CORE_SOLVER_GMRES_MIXED_KERNELS_HPP_
