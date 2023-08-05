#include "MHO_Message.hh"
#include "MHO_FastFourierTransform.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"

#include <cmath>
#include <iomanip>
#include <iostream>

using namespace hops;

typedef double FPTYPE;
#define ARRAY_TYPE MHO_NDArrayWrapper< std::complex<FPTYPE>, 2 > 
#define FFT_TYPE MHO_MultidimensionalFastFourierTransform<ARRAY_TYPE>


#define PRINT_DETAIL

int main(int /*argc*/, char** /*argv*/)
{
    const size_t ndim = 2;
    const size_t dim_size[ndim] = {4, 8};

    ARRAY_TYPE input(dim_size);

    input(0,0) = std::complex<double>(1.0, 0.);
    input(1,0) = std::complex<double>(1.0, 0.);
    input(2,0) = std::complex<double>(1.0, 0.);
    input(3,0) = std::complex<double>(1.0, 0.);

    //fill up the array with a signal
    int count = 0;
    
    #ifdef PRINT_DETAIL
    std::cout << "original data = " << std::endl;
    #endif
    for (size_t i = 0; i < dim_size[0]; i++) 
    {
        for (size_t j = 0; j < dim_size[1]; j++) 
        {
                std::cout << input(i,j) << ", ";
        }
        std::cout << std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    FFT_TYPE* fft_engine = new FFT_TYPE();

    fft_engine->SetForward();
    fft_engine->SetArgs(&input);
    fft_engine->SelectAllAxes();
    fft_engine->Initialize();
    fft_engine->Execute();
    
    #ifdef PRINT_DETAIL
    std::cout << "DFT of data = " << std::endl;
    for (size_t i = 0; i < dim_size[0]; i++) 
    {
        for (size_t j = 0; j < dim_size[1]; j++) 
        {
                std::cout << input(i,j) << ", ";
        }
        std::cout << std::endl;
    }
    #endif


    std::cout << "--------------------------------------------------------------" << std::endl;
    double delta = 0.0;
    std::complex<double> expected;

    for (size_t i = 0; i < dim_size[0]; i++) 
    {
        expected = std::complex<double>(0.0, 0.0);
        if(i == 0){expected = std::complex<double>(4.0, 0.0);}
        for (size_t j = 0; j < dim_size[1]; j++) 
        {
            std::complex<double> val = input(i,j);
            delta += std::abs(val - expected);
        }
    }

    std::cout<<"delta = "<<delta<<std::endl;

    double tol = 1e-15;
    HOPS_ASSERT_FLOAT_LESS_THAN(delta,tol);

    delete fft_engine;

    return 0;
}
