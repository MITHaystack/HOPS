#include "vector_mbp_reduce.h"


__global__ void cuda_vector_mbp_reduce(const float* a, float* out, int n)
{
    int thIdx = threadIdx.x;
    int gthIdx = thIdx + blockIdx.x*blockSize;
    const int gridSize = blockSize*gridDim.x;

    float sum = 0.0;

    for (int i = gthIdx; i < n; i += gridSize)
    {
        sum += a[i];
    }

    __shared__ float shArr[blockSize];
    shArr[thIdx] = sum;

    __syncthreads();

    for (int size = blockSize/2; size>0; size/=2)
    {
        if(thIdx<size)
        {
            shArr[thIdx] += shArr[thIdx+size];
        }
        __syncthreads();
    }

    if (thIdx == 0)
    {
        out[blockIdx.x] = shArr[0];
    }
}

void vector_mbp_reduce(float* a, float* b, int n)
{
    // Device input vectors
    float* d_a;
    float* d_out;

    // Size, in bytes, of each vector
    size_t bytes = n*sizeof(float);

    // Allocate memory for each vector on GPU
    cudaMalloc(&d_a, bytes);
    cudaMalloc(&d_out, blockSize*sizeof(float));

    // Copy host input vector to device
    cudaMemcpy(d_a, a, bytes, cudaMemcpyHostToDevice);

    // Execute the kernel
    cuda_vector_mbp_reduce<<<gridSize, blockSize>>>(d_a, d_out, n);
    //dev_out now holds the partial result
    cuda_vector_mbp_reduce<<<1, blockSize>>>(d_out, d_out, blockSize);
    //dev_out[0] now holds the final result
    cudaDeviceSynchronize();

    // Copy array back to host --- just the first element
    cudaMemcpy(b, d_out, sizeof(float), cudaMemcpyDeviceToHost );

    // Release device memory
    cudaFree(d_a);
    cudaFree(d_out);
};
