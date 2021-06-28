#include "MHO_Message.hh"
#include "MHO_FastFourierTransform.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"
#ifdef HOPS_USE_FFTW3
#include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#endif

#include <cmath>
#include <iomanip>
#include <iostream>

using namespace hops;

#ifdef HOPS_USE_FFTW3
typedef double FPTYPE;
#define FFT_TYPE MHO_MultidimensionalFastFourierTransformFFTW<FPTYPE,3>
#else
typedef double FPTYPE;
#define FFT_TYPE MHO_MultidimensionalFastFourierTransform<FPTYPE,3>
#endif


int main(int /*argc*/, char** /*argv*/)
{
    const size_t ndim = 3;
    const size_t dval = 19;
    const size_t dim_size[ndim] = {dval, dval, dval};
    const size_t total_size = dim_size[0] * dim_size[1] * dim_size[2];
    MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> input(dim_size);

    //fill up the array with a signal
    int count = 0;
    std::cout << "original data = " << std::endl;
    for (size_t i = 0; i < dim_size[0]; i++) {
        for (size_t j = 0; j < dim_size[1]; j++) {
            for (size_t k = 0; k < dim_size[2]; k++) {
                input(i,j,k) = std::complex<FPTYPE>(count % 13, count % 17);
                std::cout << input(i,j,k) << ", ";
                count++;
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    FFT_TYPE* fft_engine = new FFT_TYPE();

    fft_engine->SetForward();
    fft_engine->SetInput(&input);
    fft_engine->SetOutput(&input);

#ifndef HOPS_USE_FFTW3 //test the axis selection feature (not implemented for FFTW)
    fft_engine->DeselectAllAxes();
    fft_engine->SelectAxis(0);
    fft_engine->SelectAxis(2);
#endif

    fft_engine->Initialize();
    fft_engine->ExecuteOperation();

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

#ifndef HOPS_USE_FFTW3
    //just do the middle data axis
    fft_engine->SetForward();
    fft_engine->SetInput(&input);
    fft_engine->SetOutput(&input);
    fft_engine->DeselectAllAxes();
    fft_engine->SelectAxis(1);
    fft_engine->Initialize();
    fft_engine->ExecuteOperation();

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
    fft_engine->SetInput(&input);
    fft_engine->SetOutput(&input);
#ifndef HOPS_USE_FFTW3
    fft_engine->SelectAllAxes();
#endif
    fft_engine->Initialize();
    fft_engine->ExecuteOperation();

    std::cout << "IDFT of DFT of data = " << std::endl;
    FPTYPE norm = total_size;
    count = 0;
    FPTYPE l2_norm = 0;
    for (size_t i = 0; i < dim_size[0]; i++) {
        for (size_t j = 0; j < dim_size[1]; j++) {
            for (size_t k = 0; k < dim_size[2]; k++) {
                std::complex<FPTYPE> del = input(i,j,k) / norm;
                del -= std::complex<FPTYPE>(count % 13, count % 17);
                l2_norm += std::real(del) * std::real(del) + std::imag(del) * std::imag(del);
                count++;
            }
        }
    }


    std::cout << "L2 norm difference = " << std::sqrt(l2_norm) << std::endl;

    delete fft_engine;

    return 0;
}
