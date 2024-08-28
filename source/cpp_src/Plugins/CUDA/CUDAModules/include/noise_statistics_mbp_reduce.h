#ifndef H_NOISE_STAT_MBP_REDUCE_H__
#define H_NOISE_STAT_MBP_REDUCE_H__

// CUDA includes
#include <cuComplex.h>
#include <cufft.h>
#include <stdint.h>
#include <cuda_runtime_api.h>
#include <cuda.h>

//sum a single vector to reduce it to a single value, single-block parallel block reduction
__global__ void cuda_noise_statistics_mbp_reduce1(const float* in, float* out_sum, float* out_sum2, int n);
__global__ void cuda_noise_statistics_mbp_reduce2(const float* in, const float* in2, float* out_sum, float* out_sum2, int n);
extern "C" void noise_statistics_mbp_reduce(float* in, float* sum, float* sum2, int n);

#endif
