/*******************************<GINKGO LICENSE>******************************
Copyright (c) 2017-2021, the Ginkgo authors
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

#ifndef GKO_CORE_BASE_SIMPLE_KERNELS_HPP_
#define GKO_CORE_BASE_SIMPLE_KERNELS_HPP_


#if defined(GKO_COMPILING_CUDA)

#include "cuda/base/kernel_launch.cuh"
#define GKO_DEVICE_NAMESPACE cuda

#elif defined(GKO_COMPILING_HIP)

#include "hip/base/kernel_launch.hip.hpp"
#define GKO_DEVICE_NAMESPACE hip

#elif defined(GKO_COMPILING_DPCPP)

#include "dpcpp/base/kernel_launch.dp.hpp"
#define GKO_DEVICE_NAMESPACE dpcpp

#elif defined(GKO_COMPILING_OMP)

#include "omp/base/kernel_launch.hpp"
#define GKO_DEVICE_NAMESPACE omp

#else

#error "This file should only be used inside Ginkgo device compilation"

#endif


#endif  // GKO_CORE_BASE_SIMPLE_KERNELS_HPP_
