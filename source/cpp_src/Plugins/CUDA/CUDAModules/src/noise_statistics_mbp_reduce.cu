#include "noise_statistics_mbp_reduce.h"

#include <cstdio>

static const int blockSize = 1024;
static const int gridSize = 24; //this number is hardware-dependent; usually #SM*2 is a good number.

//first pass reduction (compute partial sum and sum2)
__global__ void cuda_noise_statistics_mbp_reduce1(const float* in, float* out_sum, float* out_sum2, int n)
{
    int tid = threadIdx.x;
    int gid = tid + blockIdx.x*blockSize;
    const int gridSize = blockSize*gridDim.x;

    float sum = 0.0;
    float sum2 = 0.0;

    for (int i = gid; i < n; i += gridSize)
    {
        float tmp = in[i];
        sum += tmp;
        sum2 += tmp*tmp;
    }

    __shared__ float shared_workspace[blockSize];
    __shared__ float shared_workspace2[blockSize];

    shared_workspace[tid] = sum;
    shared_workspace2[tid] = sum2;

    //wait for all threads to sync up
    __syncthreads();

    //proceed in lock-step with parallel reduction
    for (int size = blockSize/2; size>0; size/=2)
    {
        if(tid<size)
        {
            shared_workspace[tid] += shared_workspace[tid+size];
            shared_workspace2[tid] += shared_workspace2[tid+size];
        }
        __syncthreads();
    }

    if(tid == 0)
    {
        out_sum[blockIdx.x] = shared_workspace[0];
        out_sum2[blockIdx.x] = shared_workspace2[0];
    }
};

__global__ void cuda_noise_statistics_mbp_reduce2(const float* in, const float* in2, float* out_sum, float* out_sum2, int n)
{
    int tid = threadIdx.x;

    __shared__ float shared_workspace[blockSize];
    __shared__ float shared_workspace2[blockSize];

    shared_workspace[tid] = in[tid];
    shared_workspace2[tid] = in2[tid];

    //wait for all threads to sync up
    __syncthreads();

    //proceed in lock-step with parallel reduction
    for (int size = blockSize/2; size>0; size/=2)
    {
        if(tid<size)
        {
            shared_workspace[tid] += shared_workspace[tid+size];
            shared_workspace2[tid] += shared_workspace2[tid+size];
        }
        __syncthreads();
    }

    if(tid == 0)
    {
        out_sum[0] = shared_workspace[0];
        out_sum2[0] = shared_workspace2[0];
    }
};



void noise_statistics_mbp_reduce(float* input, float* sum, float* sum2, int n)
{
    // Device input vectors
    float* d_in;
    float* d_out;
    float* d_out2;

    float* f_out;
    float* f_out2;

    // Size, in bytes, of each vector
    size_t bytes = n*sizeof(float);

    // Allocate memory for each vector on GPU
    cudaMalloc(&d_in, bytes);
    cudaMalloc(&d_out, blockSize*sizeof(float));
    cudaMalloc(&d_out2, blockSize*sizeof(float));
    cudaMalloc(&f_out, sizeof(float));
    cudaMalloc(&f_out2, sizeof(float));

    // Copy host input vector to device
    cudaMemcpy(d_in, input, bytes, cudaMemcpyHostToDevice);

    //run the first level of reduction
    cuda_noise_statistics_mbp_reduce1<<<gridSize, blockSize>>>(d_in, d_out, d_out2, n);

    //now reduce the partial results to a single number
    cuda_noise_statistics_mbp_reduce2<<<1, blockSize>>>(d_out, d_out2, f_out, f_out2, blockSize); ///THIS IS WRONG FOR SUM2!!!
    //dev_out[0] now holds the final result
    cudaDeviceSynchronize();

    // Copy array back to host --- just the first element
    cudaMemcpy(sum, f_out, sizeof(float), cudaMemcpyDeviceToHost );
    cudaMemcpy(sum2, f_out2, sizeof(float), cudaMemcpyDeviceToHost );
    //
    // float tmp = 0.0;
    // float tmp2 = 0.0;
    //
    // for(int i=0; i<blockSize; i++)
    // {
    //     tmp += sum[i];
    //     tmp2 += sum2[i];
    // }

    printf("sum %f \n",sum);
    printf("sum2 %f \n",sum2);

    // Release device memory
    cudaFree(d_in);
    cudaFree(d_out);
    cudaFree(d_out2);
    cudaFree(f_out);
    cudaFree(f_out2);
};
