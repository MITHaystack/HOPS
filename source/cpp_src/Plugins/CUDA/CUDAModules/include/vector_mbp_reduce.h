#ifndef H_VECTOR_MBP_REDUCE_H__
#define H_VECTOR_MBP_REDUCE_H__

// CUDA includes
#include <cuComplex.h>
#include <cufft.h>
#include <stdint.h>
#include <cuda_runtime_api.h>
#include <cuda.h>

static const int wholeArraySize = 100000000;
static const int blockSize = 1024;
static const int gridSize = 24; //this number is hardware-dependent; usually #SM*2 is a good number.


//sum a single vector to reduce it to a single value, single-block parallel block reduction
__global__ void cuda_vector_mbp_reduce(const float* a, float* out);
extern "C" void vector_mbp_reduce(float* a, float* b, int n);

#endif
