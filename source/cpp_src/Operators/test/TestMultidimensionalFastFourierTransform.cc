#include "MHO_Message.hh"
#include "MHO_FastFourierTransform.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"
#ifdef HOPS_USE_FFTW3
#include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#endif

#include <cmath>
#include <iomanip>
#include <iostream>

//
// #ifdef HOPS_USE_FFTW3
// #define FFT_TYPE MHO_MultidimensionalFastFourierTransformFFTW<1>
// #else
// #define FFT_TYPE MHO_MultidimensionalFastFourierTransform<1>
// #endif
//
// using namespace hops;
//
// int main(int /*argc*/, char** /*argv*/)
// {
//     const size_t ndim = 1;
//     const size_t dval = 5;
//     const size_t dim_size[ndim] = {dval};
//     const size_t total_size = dim_size[0];
//     MHO_NDArrayWrapper<std::complex<double>, ndim> input(dim_size[0]);
//
//     //fill up the array with a signal
//     int count = 0;
//     std::cout << "original data = " << std::endl;
//     for (size_t i = 0; i < dim_size[0]; i++) {
//         input(i) = std::complex<double>(count, 0.0);
//         std::cout << input(i) << ", ";
//         count++;
//     }
//
//     std::cout << "--------------------------------------------------------------" << std::endl;
//
//     FFT_TYPE* fft_engine = new FFT_TYPE();
//
//     fft_engine->SetForward();
//     fft_engine->SetInput(&input);
//     fft_engine->SetOutput(&input);
//     fft_engine->Initialize();
//     fft_engine->ExecuteOperation();
//
//     std::cout << "DFT of data = " << std::endl;
//     for (size_t i = 0; i < dim_size[0]; i++) {
//         std::cout << input(i) << ", ";
//     }
//
//     std::cout << "--------------------------------------------------------------" << std::endl;
//
//     fft_engine->SetBackward();
//     fft_engine->SetInput(&input);
//     fft_engine->SetOutput(&input);
//     fft_engine->Initialize();
//     fft_engine->ExecuteOperation();
//
//     std::cout << "IDFT of DFT of data = " << std::endl;
//     double norm = total_size;
//     count = 0;
//     double l2_norm = 0;
//     for (size_t i = 0; i < dim_size[0]; i++) {
//         std::complex<double> del = input(i) / norm;
//         del -= std::complex<double>(count, 0.0);
//         l2_norm += std::real(del) * std::real(del) + std::imag(del) * std::imag(del);
//         count++;
//     }
//
//
//     std::cout << "L2 norm difference = " << std::sqrt(l2_norm) << std::endl;
//
//     delete fft_engine;
//
//     return 0;
// }
//
//
//



// 
// 
// #ifdef HOPS_USE_FFTW3
// #define FFT_TYPE MHO_MultidimensionalFastFourierTransformFFTW<2>
// #else
// #define FFT_TYPE MHO_MultidimensionalFastFourierTransform<2>
// #endif
// 
// using namespace hops;
// 
// int main(int /*argc*/, char** /*argv*/)
// {
//     const size_t ndim = 2;
//     const size_t dval = 4;
//     const size_t dim_size[ndim] = {dval, dval};
//     const size_t total_size = dim_size[0] * dim_size[1];
//     MHO_NDArrayWrapper<std::complex<double>, ndim> input(dim_size);
//     MHO_NDArrayWrapper<std::complex<double>, ndim> output(dim_size);
// 
//     //fill up the array with a signal
//     int count = 0;
//     std::cout << "original data = " << std::endl;
//     for (size_t i = 0; i < dim_size[0]; i++) {
//         for (size_t j = 0; j < dim_size[1]; j++) {
//                 input(i,j) = std::complex<double>(count % 13, count % 17);
//                 std::cout << input(i,j) << ", ";
//                 count++;
//         }
//         std::cout << std::endl;
//     }
// 
//     std::cout << "--------------------------------------------------------------" << std::endl;
// 
//     FFT_TYPE* fft_engine = new FFT_TYPE();
// 
//     //fft_engine->SetBackward();//Forward();
//     fft_engine->SetForward();
//     fft_engine->SetInput(&input);
//     fft_engine->SetOutput(&output);
//     fft_engine->Initialize();
//     fft_engine->ExecuteOperation();
// 
//     std::cout << "DFT of data = " << std::endl;
//     for (size_t i = 0; i < dim_size[0]; i++) {
//         for (size_t j = 0; j < dim_size[1]; j++) {
//                 std::cout << output(i,j) << ", ";
//         }
//         std::cout << std::endl;
//     }
// 
//     std::cout << "--------------------------------------------------------------" << std::endl;
// 
//     fft_engine->SetBackward();
//     //fft_engine->SetForward();//SetBackward();
//     fft_engine->SetInput(&output);
//     fft_engine->SetOutput(&input);
//     fft_engine->Initialize();
//     fft_engine->ExecuteOperation();
// 
//     std::cout << "IDFT of DFT of data = " << std::endl;
//     double norm = total_size;
//     count = 0;
//     double l2_norm = 0;
//     for (size_t i = 0; i < dim_size[0]; i++) {
//         for (size_t j = 0; j < dim_size[1]; j++) {
//             std::complex<double> del = input(i,j) / norm;
//             del -= std::complex<double>(count % 13, count % 17);
//             l2_norm += std::real(del) * std::real(del) + std::imag(del) * std::imag(del);
//             count++;
//         }
//     }
// 
// 
//     std::cout << "L2 norm difference = " << std::sqrt(l2_norm) << std::endl;
// 
//     delete fft_engine;
// 
//     return 0;
// }
// 
// 
// 
// 
// 
// 








































#ifdef HOPS_USE_FFTW3
typedef double FP_Type; //FFTW3 must use double
#define FFT_TYPE MHO_MultidimensionalFastFourierTransformFFTW<3>
#else
typedef float FP_type;
#define FFT_TYPE MHO_MultidimensionalFastFourierTransform<FP_Type,3> 
#endif






using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
    const size_t ndim = 3;
    const size_t dval = 5;
    const size_t dim_size[ndim] = {dval, dval, dval};
    const size_t total_size = dim_size[0] * dim_size[1] * dim_size[2];
    MHO_NDArrayWrapper<std::complex<FP_Type>, ndim> input(dim_size);

    //fill up the array with a signal
    int count = 0;
    std::cout << "original data = " << std::endl;
    for (size_t i = 0; i < dim_size[0]; i++) {
        for (size_t j = 0; j < dim_size[1]; j++) {
            for (size_t k = 0; k < dim_size[2]; k++) {
                input(i,j,k) = std::complex<FP_Type>(count % 13, count % 17);
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

    std::cout << "--------------------------------------------------------------" << std::endl;

    fft_engine->SetBackward();
    fft_engine->SetInput(&input);
    fft_engine->SetOutput(&input);
    fft_engine->Initialize();
    fft_engine->ExecuteOperation();

    std::cout << "IDFT of DFT of data = " << std::endl;
    FP_Type norm = total_size;
    count = 0;
    FP_Type l2_norm = 0;
    for (size_t i = 0; i < dim_size[0]; i++) {
        for (size_t j = 0; j < dim_size[1]; j++) {
            for (size_t k = 0; k < dim_size[2]; k++) {
                std::complex<FP_Type> del = input(i,j,k) / norm;
                del -= std::complex<FP_Type>(count % 13, count % 17);
                l2_norm += std::real(del) * std::real(del) + std::imag(del) * std::imag(del);
                count++;
            }
        }
    }


    std::cout << "L2 norm difference = " << std::sqrt(l2_norm) << std::endl;

    delete fft_engine;

    return 0;
}
