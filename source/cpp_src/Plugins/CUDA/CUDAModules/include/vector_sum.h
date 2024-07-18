#ifndef H_VECTOR_SUM_H__
#define H_VECTOR_SUM_H__

// CUDA includes
#include <cuComplex.h>
#include <cufft.h>
#include <stdint.h>
#include <cuda_runtime_api.h>
#include <cuda.h>

//sum two vectors
__global__ void cuda_vector_add(float* a, float* b, float* c, int length);
extern "C" void vector_sum(float* a, float* b, float* c, int n);

//sum a single vector to reduce it to a single value
__global__ void cuda_vector_reduce(float* a, float* b, int length);
extern "C" void vector_reduce(float* a, float* b, int n);

#endif
