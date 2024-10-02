#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <vector>

// CUDA includes
#include <cuComplex.h>
#include <cuda.h>
#include <cuda_runtime_api.h>
#include <cufft.h>
#include <stdint.h>

#include <complex>
#include <cufft.h>
#include <iostream>

int main()
{

    // Define the size of the array
    const int N = 1024; // Number of rows
    const int M = 512;  // Number of columns

    // Create a 2D array of complex numbers
    std::complex< double >* hostData = new std::complex< double >[N * M];

    // Allocate device memory for the input data
    cufftDoubleComplex* deviceData;
    cudaMalloc((void**)&deviceData, sizeof(cufftDoubleComplex) * N * M);
    cudaMemcpy(deviceData, hostData, sizeof(std::complex< double >) * N * M, cudaMemcpyHostToDevice);

    // Create a cuFFT plan
    cufftHandle plan;
    cufftPlan1d(&plan, M, CUFFT_Z2Z, N); // 1D complex-to-complex FFT along the second dimension

    // Execute FFT
    cufftExecZ2Z(plan, deviceData, deviceData, CUFFT_FORWARD);

    // Copy the result back to host
    cudaMemcpy(hostData, deviceData, sizeof(std::complex< double >) * N * M, cudaMemcpyDeviceToHost);

    // Clean up
    cufftDestroy(plan);
    cudaFree(deviceData);
    delete[] hostData;

    return 0;
}

// int main() {
//     // Define the dimensions of the 4D array
//     const int dim1 = 1;
//     const int dim2 = 32;
//     const int dim3 = 4096;
//     const int dim4 = 464;
//
//     // Create a 4D array of complex numbers (real + imaginary parts)
//     cufftComplex* h_data = new cufftComplex[dim1 * dim2 * dim3 * dim4];
//
//     // Initialize your complex data here...
//
//     // Allocate device memory and copy data to it
//     cufftComplex* d_data;
//     cudaMalloc((void**)&d_data, sizeof(cufftComplex) * dim1 * dim2 * dim3 * dim4);
//     cudaMemcpy(d_data, h_data, sizeof(cufftComplex) * dim1 * dim2 * dim3 * dim4, cudaMemcpyHostToDevice);
//
//     // Create a 4D CUFFT plan
//     cufftHandle plan;
//     int n[4] = {dim1, dim2, dim3, dim4};
//     cufftPlanMany(&plan, 4, n, NULL, 1, 0, NULL, 1, 0, CUFFT_C2C, dim4);
//
//     // Perform FFT
//     cufftExecC2C(plan, d_data, d_data, CUFFT_FORWARD);
//
//     // Free resources
//     cufftDestroy(plan);
//     cudaFree(d_data);
//     delete[] h_data;
//
//     return 0;
// }
