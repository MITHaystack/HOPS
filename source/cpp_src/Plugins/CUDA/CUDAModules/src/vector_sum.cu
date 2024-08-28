#include "vector_sum.h"


//cuda vector addition A+B=C
__global__ void cuda_vector_add(float* a, float* b, float* c, int length)
{
    int id = blockIdx.x*blockDim.x+threadIdx.x;
    if (id < length)
    {
        c[id] = a[id] + b[id];
    }
}


void vector_sum(float* a, float* b, float* c, int n)
{

    // Device input vectors
    float* d_a;
    float* d_b;
    //Device output vector
    float* d_c;

    // Size, in bytes, of each vector
    size_t bytes = n*sizeof(float);

    // Allocate memory for each vector on GPU
    cudaMalloc(&d_a, bytes);
    cudaMalloc(&d_b, bytes);
    cudaMalloc(&d_c, bytes);

    // Copy host vectors to device
    cudaMemcpy( d_a, a, bytes, cudaMemcpyHostToDevice);
    cudaMemcpy( d_b, b, bytes, cudaMemcpyHostToDevice);

    int blockSize, gridSize;

    // Number of threads in each thread block
    blockSize = 1024;

    // Number of thread blocks in grid
    gridSize = (int)ceil((float)n/blockSize);

    // Execute the kernel
    cuda_vector_add<<<gridSize, blockSize>>>(d_a, d_b, d_c, n);

    // Copy array back to host
    cudaMemcpy(c, d_c, bytes, cudaMemcpyDeviceToHost );

    // Release device memory
    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);
};
