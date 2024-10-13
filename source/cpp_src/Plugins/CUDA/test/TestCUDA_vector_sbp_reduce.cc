#include <fstream>
#include <iostream>
#include <memory>
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

#include "vector_sbp_reduce.h"

int main(int /*argc*/, char** /*argv*/)
{
    int n = 1000000;

    // Host input vectors
    float* h_a;
    float* h_c;

    // Allocate memory for each vector on host
    h_a = new float[n];
    h_c = new float[1];
    h_c[0] = 0.0;

    int i;
    // Initialize vectors on host
    for(i = 0; i < n; i++)
    {
        h_a[i] = 1.1;
    }

    vector_sbp_reduce(h_a, h_c, n);

    std::cout << "final result: " << h_c[0] << std::endl;

    delete[] h_a;
    delete[] h_c;

    return 0;
}
