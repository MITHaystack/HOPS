#include <iostream>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <iomanip>
#include <random>
#include <cmath>

// CUDA includes
#include <cuComplex.h>
#include <cufft.h>
#include <stdint.h>
#include <cuda_runtime_api.h>
#include <cuda.h>


int main() {
    // Define the dimensions of the 4D array
    const int dim1 = 1;
    const int dim2 = 32;
    const int dim3 = 4096;
    const int dim4 = 464;

    // Create a 4D array of complex numbers (real + imaginary parts)
    cufftComplex* h_data = new cufftComplex[dim1 * dim2 * dim3 * dim4];

    // Initialize your complex data here...

    // Allocate device memory and copy data to it
    cufftComplex* d_data;
    cudaMalloc((void**)&d_data, sizeof(cufftComplex) * dim1 * dim2 * dim3 * dim4);
    cudaMemcpy(d_data, h_data, sizeof(cufftComplex) * dim1 * dim2 * dim3 * dim4, cudaMemcpyHostToDevice);

    // Create a 4D CUFFT plan
    cufftHandle plan;
    int n[4] = {dim1, dim2, dim3, dim4};
    cufftPlanMany(&plan, 4, n, NULL, 1, 0, NULL, 1, 0, CUFFT_C2C, dim4);

    // Perform FFT
    cufftExecC2C(plan, d_data, d_data, CUFFT_FORWARD);

    // Free resources
    cufftDestroy(plan);
    cudaFree(d_data);
    delete[] h_data;

    return 0;
}
