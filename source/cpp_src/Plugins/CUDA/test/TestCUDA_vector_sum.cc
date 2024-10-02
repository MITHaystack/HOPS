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

#include "vector_sum.h"

int main(int /*argc*/, char** /*argv*/)
{
    int n = 1000000;

    // Host input vectors
    float* h_a;
    float* h_b;
    //Host output vector
    float* h_c;

    // Allocate memory for each vector on host
    h_a = new float[n];
    h_b = new float[n];
    h_c = new float[n];

    int i;
    // Initialize vectors on host
    for(i = 0; i < n; i++)
    {
        float x = 2 * M_PI * ((float)i / (float)n);
        h_a[i] = sin(x) * sin(x);
        h_b[i] = cos(x) * cos(x);
    }

    vector_sum(h_a, h_b, h_c, n);

    // Sum up vector c and print result divided by n, this should equal 1 within error
    double sum = 0;
    for(i = 0; i < n; i++)
    {
        sum += h_c[i];
    }
    std::cout << "final result (should be 1.0): " << sum / n << std::endl;

    delete[] h_a;
    delete[] h_b;
    delete[] h_c;

    return 0;
}
