#ifndef H_VECTOR_SBP_REDUCE_H__
#define H_VECTOR_SBP_REDUCE_H__

// CUDA includes
#include <cuComplex.h>
#include <cufft.h>
#include <stdint.h>
#include <cuda_runtime_api.h>
#include <cuda.h>

//sum a single vector to reduce it to a single value, single-block parallel block reduction
__global__ void cuda_vector_sbp_reduce(const float* a, float* out);
extern "C" void vector_sbp_reduce(float* a, float* b, int n);

#endif
