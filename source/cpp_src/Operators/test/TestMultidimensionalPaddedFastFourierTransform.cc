#include "MHO_Message.hh"
#include "MHO_FastFourierTransform.hh"
#include "MHO_MultidimensionalPaddedFastFourierTransform.hh"
// #ifdef HOPS_USE_FFTW3
// #include "MHO_MultidimensionalPaddedFastFourierTransformFFTW.hh"
// #endif

#include <cmath>
#include <iomanip>
#include <iostream>

using namespace hops;

// #ifdef HOPS_USE_FFTW3
// typedef double FPTYPE;
// #define FFT_TYPE MHO_MultidimensionalPaddedFastFourierTransformFFTW<FPTYPE,3>
// #else
typedef double FPTYPE;
#define FFT_TYPE MHO_MultidimensionalPaddedFastFourierTransform<FPTYPE,3>
// #endif


int main(int /*argc*/, char** /*argv*/)
{
    const size_t pad_factor = 1;
    const size_t ndim = 3;
    const size_t dval = 4;
    const size_t dim_size[ndim] = {dval, dval, dval};
    const size_t pad_dim_size[ndim] = {pad_factor*dval, pad_factor*dval, pad_factor*dval};
    const size_t total_size = dim_size[0] * dim_size[1] * dim_size[2];
    MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> input(dim_size);
    MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> output(pad_dim_size);

    //fill up the array with a signal
    int count = 0;

    #ifdef HOPS_ENABLE_DEBUG_MSG
    std::cout << "original data = " << std::endl;
    #endif
    for (size_t i = 0; i < dim_size[0]; i++) {
        for (size_t j = 0; j < dim_size[1]; j++) {
            for (size_t k = 0; k < dim_size[2]; k++) {
                input(i,j,k) = std::complex<FPTYPE>(count % 13, count % 17);

                #ifdef HOPS_ENABLE_DEBUG_MSG
                std::cout << input(i,j,k) << ", ";
                #endif
                count++;
            }
            #ifdef HOPS_ENABLE_DEBUG_MSG
            std::cout << std::endl;
            #endif
        }
        #ifdef HOPS_ENABLE_DEBUG_MSG
        std::cout << std::endl;
        #endif
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    bool check;
    FFT_TYPE* fft_engine = new FFT_TYPE();

    fft_engine->SetPaddingFactor(pad_factor);
    fft_engine->SetEndPadded();
    fft_engine->SetForward();
    fft_engine->SetInput(&input);
    fft_engine->SetOutput(&output);
    fft_engine->SelectAllAxes();
    check = fft_engine->Initialize();
    check = fft_engine->ExecuteOperation();

    #ifdef HOPS_ENABLE_DEBUG_MSG
    std::cout << "DFT of data = " << std::endl;
    for (size_t i = 0; i < dim_size[0]; i++) {
        for (size_t j = 0; j < dim_size[1]; j++) {
            for (size_t k = 0; k < dim_size[2]; k++) {
                std::cout << output(i,j,k) << ", ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
    #endif

    std::cout << "--------------------------------------------------------------" << std::endl;

    fft_engine->SetPaddingFactor(pad_factor);
    fft_engine->SetEndPadded();
    fft_engine->SetBackward();
    fft_engine->SetInput(&output);
    fft_engine->SetOutput(&input);
    fft_engine->SelectAllAxes();
    fft_engine->Initialize();
    fft_engine->ExecuteOperation();

    #ifdef HOPS_ENABLE_DEBUG_MSG
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
