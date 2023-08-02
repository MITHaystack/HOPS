#include "MHO_Message.hh"
#include "MHO_FastFourierTransform.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"
// #ifdef HOPS_USE_FFTW3
// #include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
// #endif

#include <cmath>
#include <iomanip>
#include <iostream>

using namespace hops;

// #ifdef HOPS_USE_FFTW3
// typedef double FPTYPE;
// #define ARRAY_TYPE MHO_NDArrayWrapper< std::complex<FPTYPE>, 3 > 
// #define FFT_TYPE MHO_MultidimensionalFastFourierTransformFFTW<ARRAY_TYPE>
// #else
typedef double FPTYPE;
#define ARRAY_TYPE MHO_NDArrayWrapper< std::complex<FPTYPE>, 3 > 
#define FFT_TYPE MHO_MultidimensionalFastFourierTransform<ARRAY_TYPE>
// #endif

#define PRINT_DETAIL

int main(int /*argc*/, char** /*argv*/)
{
    const size_t ndim = 3;
    const size_t dval = 5;
    const size_t dim_size[ndim] = {dval, dval, dval};
    const size_t total_size = dim_size[0] * dim_size[1] * dim_size[2];
    ARRAY_TYPE input(dim_size);

    //fill up the array with a signal
    int count = 0;

    // #ifdef HOPS_USE_FFTW3
    // std::cout<<"using fftw3 for FFTs"<<std::endl;
    // #endif

    #ifdef PRINT_DETAIL
    std::cout << "original data = " << std::endl;
    #endif
    for (size_t i = 0; i < dim_size[0]; i++) {
        for (size_t j = 0; j < dim_size[1]; j++) {
            for (size_t k = 0; k < dim_size[2]; k++) {
                input(i,j,k) = std::complex<FPTYPE>(count % 13, count % 17);

                #ifdef PRINT_DETAIL
                std::cout << input(i,j,k) << ", ";
                #endif
                count++;
            }
            #ifdef PRINT_DETAIL
            std::cout << std::endl;
            #endif
        }
        #ifdef PRINT_DETAIL
        std::cout << std::endl;
        #endif
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    FFT_TYPE* fft_engine = new FFT_TYPE();

    fft_engine->SetForward();
    fft_engine->SetArgs(&input);
    
    fft_engine->DeselectAllAxes();
    fft_engine->SelectAxis(0);
    fft_engine->SelectAxis(2);
    
    fft_engine->Initialize();
    fft_engine->Execute();
    
    #ifdef PRINT_DETAIL
    std::cout << "DFT of data = " << std::endl;
    for (size_t i = 0; i < dim_size[0]; i++) {
        for (size_t j = 0; j < dim_size[1]; j++) {
            for (size_t k = 0; k < dim_size[2]; k++) {
                std::cout << input(i,j,k) << ", ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    #endif


    //just do the middle data axis
    fft_engine->SetForward();
    fft_engine->SetArgs(&input);
    fft_engine->DeselectAllAxes();
    fft_engine->SelectAxis(1);
    fft_engine->Initialize();
    fft_engine->Execute();
    
    #ifdef PRINT_DETAIL
    std::cout << "DFT of data = " << std::endl;
    for (size_t i = 0; i < dim_size[0]; i++) {
        for (size_t j = 0; j < dim_size[1]; j++) {
            for (size_t k = 0; k < dim_size[2]; k++) {
                std::cout << input(i,j,k) << ", ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    #endif

    std::cout << "--------------------------------------------------------------" << std::endl;

    fft_engine->SetBackward();
    fft_engine->SetArgs(&input);
    fft_engine->SelectAllAxes();
    fft_engine->Initialize();
    fft_engine->Execute();

    #ifdef PRINT_DETAIL
    std::cout << "IDFT of DFT of data = " << std::endl;
    #endif

    FPTYPE norm = total_size;
    count = 0;
    FPTYPE l2_norm = 0;
    for (size_t i = 0; i < dim_size[0]; i++) {
        for (size_t j = 0; j < dim_size[1]; j++) {
            for (size_t k = 0; k < dim_size[2]; k++) {
                //normalize out the factor of N caused by FFT -> IFFT -> result
                std::complex<FPTYPE> del = input(i,j,k) / norm;
                std::cout << del << ", ";
                del -= std::complex<FPTYPE>(count % 13, count % 17);
                l2_norm += std::real(del) * std::real(del) + std::imag(del) * std::imag(del);
                count++;
            }
        }
    }

    double err = std::sqrt(l2_norm);
    std::cout << "L2_diff = " << err << std::endl;
    std::cout << "L2_diff/N = "<< err/norm <<std::endl; //average error per point
    double tol = 1.5e-15; //tests for ndim=3, dval=19

    HOPS_ASSERT_FLOAT_LESS_THAN(err/norm,tol);

    delete fft_engine;

    return 0;
}
