/*******************************<GINKGO LICENSE>******************************
Copyright (c) 2017-2022, the Ginkgo authors
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

#include <ginkgo/core/base/exception.hpp>


#include <string>


#include <cublas_v2.h>
#include <cuda_runtime.h>
#include <cufft.h>
#include <curand.h>
#include <cusparse.h>


#include <ginkgo/core/base/types.hpp>


namespace gko {


std::string CudaError::get_error(int64 error_code)
{
    std::string name = cudaGetErrorName(static_cast<cudaError>(error_code));
    std::string message =
        cudaGetErrorString(static_cast<cudaError>(error_code));
    return name + ": " + message;
}


std::string CublasError::get_error(int64 error_code)
{
#define GKO_REGISTER_CUBLAS_ERROR(error_name)           \
    if (error_code == static_cast<int64>(error_name)) { \
        return #error_name;                             \
    }
    GKO_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_SUCCESS);
    GKO_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_NOT_INITIALIZED);
    GKO_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_ALLOC_FAILED);
    GKO_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_INVALID_VALUE);
    GKO_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_ARCH_MISMATCH);
    GKO_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_MAPPING_ERROR);
    GKO_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_EXECUTION_FAILED);
    GKO_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_INTERNAL_ERROR);
    GKO_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_NOT_SUPPORTED);
    GKO_REGISTER_CUBLAS_ERROR(CUBLAS_STATUS_LICENSE_ERROR);
    return "Unknown error";

#undef GKO_REGISTER_CUBLAS_ERROR
}


std::string CurandError::get_error(int64 error_code)
{
#define GKO_REGISTER_CURAND_ERROR(error_name)           \
    if (error_code == static_cast<int64>(error_name)) { \
        return #error_name;                             \
    }
    GKO_REGISTER_CURAND_ERROR(CURAND_STATUS_SUCCESS);
    GKO_REGISTER_CURAND_ERROR(CURAND_STATUS_VERSION_MISMATCH);
    GKO_REGISTER_CURAND_ERROR(CURAND_STATUS_NOT_INITIALIZED);
    GKO_REGISTER_CURAND_ERROR(CURAND_STATUS_ALLOCATION_FAILED);
    GKO_REGISTER_CURAND_ERROR(CURAND_STATUS_TYPE_ERROR);
    GKO_REGISTER_CURAND_ERROR(CURAND_STATUS_OUT_OF_RANGE);
    GKO_REGISTER_CURAND_ERROR(CURAND_STATUS_LENGTH_NOT_MULTIPLE);
    GKO_REGISTER_CURAND_ERROR(CURAND_STATUS_DOUBLE_PRECISION_REQUIRED);
    GKO_REGISTER_CURAND_ERROR(CURAND_STATUS_LAUNCH_FAILURE);
    GKO_REGISTER_CURAND_ERROR(CURAND_STATUS_PREEXISTING_FAILURE);
    GKO_REGISTER_CURAND_ERROR(CURAND_STATUS_INITIALIZATION_FAILED);
    GKO_REGISTER_CURAND_ERROR(CURAND_STATUS_ARCH_MISMATCH);
    GKO_REGISTER_CURAND_ERROR(CURAND_STATUS_INTERNAL_ERROR);
    return "Unknown error";

#undef GKO_REGISTER_CURAND_ERROR
}


std::string CusparseError::get_error(int64 error_code)
{
#define GKO_REGISTER_CUSPARSE_ERROR(error_name) \
    if (error_code == int64(error_name)) {      \
        return #error_name;                     \
    }
    GKO_REGISTER_CUSPARSE_ERROR(CUSPARSE_STATUS_SUCCESS);
    GKO_REGISTER_CUSPARSE_ERROR(CUSPARSE_STATUS_NOT_INITIALIZED);
    GKO_REGISTER_CUSPARSE_ERROR(CUSPARSE_STATUS_ALLOC_FAILED);
    GKO_REGISTER_CUSPARSE_ERROR(CUSPARSE_STATUS_INVALID_VALUE);
    GKO_REGISTER_CUSPARSE_ERROR(CUSPARSE_STATUS_ARCH_MISMATCH);
    GKO_REGISTER_CUSPARSE_ERROR(CUSPARSE_STATUS_MAPPING_ERROR);
    GKO_REGISTER_CUSPARSE_ERROR(CUSPARSE_STATUS_EXECUTION_FAILED);
    GKO_REGISTER_CUSPARSE_ERROR(CUSPARSE_STATUS_INTERNAL_ERROR);
    GKO_REGISTER_CUSPARSE_ERROR(CUSPARSE_STATUS_MATRIX_TYPE_NOT_SUPPORTED);
    return "Unknown error";

#undef GKO_REGISTER_CUSPARSE_ERROR
}


std::string CufftError::get_error(int64 error_code)
{
#define GKO_REGISTER_CUFFT_ERROR(error_name) \
    if (error_code == int64(error_name)) {   \
        return #error_name;                  \
    }
    GKO_REGISTER_CUFFT_ERROR(CUFFT_SUCCESS)
    GKO_REGISTER_CUFFT_ERROR(CUFFT_INVALID_PLAN)
    GKO_REGISTER_CUFFT_ERROR(CUFFT_ALLOC_FAILED)
    GKO_REGISTER_CUFFT_ERROR(CUFFT_INVALID_TYPE)
    GKO_REGISTER_CUFFT_ERROR(CUFFT_INVALID_VALUE)
    GKO_REGISTER_CUFFT_ERROR(CUFFT_INTERNAL_ERROR)
    GKO_REGISTER_CUFFT_ERROR(CUFFT_EXEC_FAILED)
    GKO_REGISTER_CUFFT_ERROR(CUFFT_SETUP_FAILED)
    GKO_REGISTER_CUFFT_ERROR(CUFFT_INVALID_SIZE)
    GKO_REGISTER_CUFFT_ERROR(CUFFT_UNALIGNED_DATA)
    GKO_REGISTER_CUFFT_ERROR(CUFFT_INCOMPLETE_PARAMETER_LIST)
    GKO_REGISTER_CUFFT_ERROR(CUFFT_INVALID_DEVICE)
    GKO_REGISTER_CUFFT_ERROR(CUFFT_PARSE_ERROR)
    GKO_REGISTER_CUFFT_ERROR(CUFFT_NO_WORKSPACE)
    GKO_REGISTER_CUFFT_ERROR(CUFFT_NOT_IMPLEMENTED)
    GKO_REGISTER_CUFFT_ERROR(CUFFT_LICENSE_ERROR)
    GKO_REGISTER_CUFFT_ERROR(CUFFT_NOT_SUPPORTED)
    return "Unknown error";

#undef GKO_REGISTER_CUFFT_ERROR
}


}  // namespace gko
