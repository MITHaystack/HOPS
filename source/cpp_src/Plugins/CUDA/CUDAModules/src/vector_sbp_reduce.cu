#include "vector_sbp_reduce.h"


static const int blockSize = 1024;

__global__ void cuda_vector_sbp_reduce(const float* a, float* out, int n)
{
    int idx = threadIdx.x;
    float sum = 0.0;

    for (int i = idx; i < n; i += blockSize)
    {
        sum += a[i];
    }

    __shared__ float r[blockSize];

    r[idx] = sum;
    __syncthreads();

    for (int size = blockSize/2; size>0; size/=2)
    {
        if (idx<size)
        {
            r[idx] += r[idx+size];
        }
        __syncthreads();
    }
    if (idx == 0){ *out = r[0];};
}

void vector_sbp_reduce(float* a, float* b, int n)
{

    // Device input vectors
    float* d_a;
    float* d_out;

    // Size, in bytes, of each vector
    size_t bytes = n*sizeof(float);

    // Allocate memory for each vector on GPU
    cudaMalloc(&d_a, bytes);
    cudaMalloc(&d_out, sizeof(float));

    // Copy host input vector to device
    cudaMemcpy(d_a, a, bytes, cudaMemcpyHostToDevice);


    // Number of threads in each thread block is blockSize = 1024;
    // Number of thread blocks in grid
    int gridSize;
    gridSize = (int)ceil((float)n/blockSize);

    // Execute the kernel
    cuda_vector_sbp_reduce<<<gridSize, blockSize>>>(d_a, d_out, n);

    // Copy array back to host
    cudaMemcpy(b, d_out, sizeof(float), cudaMemcpyDeviceToHost );

    // Release device memory
    cudaFree(d_a);
    cudaFree(d_out);
};
