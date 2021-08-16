#include "MHO_Message.hh"
#include "MHO_FastFourierTransform.hh"
#include "MHO_MultidimensionalPaddedFastFourierTransform.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"
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
#define PADDED_FFT_TYPE MHO_MultidimensionalPaddedFastFourierTransform<FPTYPE,1>
#define FFT_TYPE MHO_MultidimensionalFastFourierTransform<FPTYPE,1>
// #endif


int main(int /*argc*/, char** /*argv*/)
{

    int option = 1;

    const size_t ndim = 1;
    const size_t N = 8;
    const size_t M = 2;
    const size_t NM = N*M;

    MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> array1(N);
    MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> array2(N);
    MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> expanded_array1(NM);
    MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> expanded_array2(NM);

    //fill up the array with a random signal
    srand(0);
    double r1 = 0;
    double r2 = 0;
    for (size_t i = 0; i < N; i++) 
    {
        double r2 = (rand()%1024);
        r2 /= 1024.0;
        array1(i) = std::complex<FPTYPE>( (r1+r2)/2.0, 0.0);
        r1 = r2;
    }

    //insert a peak
    array1[N/3]  = 3.0;

    //then we execute an FFT to move to frequency space 
    FFT_TYPE* fft_engine = new FFT_TYPE();
    fft_engine->SetBackward();
    fft_engine->SetInput(&array1);
    fft_engine->SetOutput(&array2);
    fft_engine->Initialize();
    fft_engine->ExecuteOperation();

    //then we copy the results into an ex#include "MHO_MultidimensionalPaddedFastFourierTransform.hh"panded array (with the middle padded by zeros)
    for (size_t i = 0; i < NM; i++) 
    {
        expanded_array1(i) = std::complex<FPTYPE>(0.0, 0.0);
        expanded_array2(i) = std::complex<FPTYPE>(0.0, 0.0);
    }

    //now copy the array into the first portion of the expanded array  (end-padded)
    for(size_t i=0; i<N; i++)
    {
        expanded_array1(i) = array2(i);
    }


    //then we execute an inverse FFT to bring us back to original do_estimation
    FFT_TYPE* fft_engine2 = new FFT_TYPE();
    fft_engine2->SetForward();
    fft_engine2->SetInput(&expanded_array1);
    fft_engine2->SetOutput(&expanded_array2);
    fft_engine2->Initialize();
    fft_engine2->ExecuteOperation();

    //now normalized the output array 
    double norm = N;
    for(size_t i=0; i<NM; i++)
    {
        expanded_array2[i] /= norm; 
    }


    for(size_t i=0; i<N; i++)
    {
        std::cout<<"original array @ "<<i<<" = "<<array1[i]<<std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    for(size_t i=0; i<NM; i++)
    {
        std::cout<<"manual zero-padded interpolated array @ "<<i<<" = "<<expanded_array2[i]<<std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    //now use the zero-padded (end) fft engine to do the same thing
    bool check;
    PADDED_FFT_TYPE* pfft_engine = new PADDED_FFT_TYPE();
    
    pfft_engine->SetPaddingFactor(M);
    pfft_engine->SetEndPadded();
    pfft_engine->SetForward();
    pfft_engine->SetInput(&array2);
    pfft_engine->SetOutput(&expanded_array1);
    pfft_engine->SelectAllAxes();
    check = pfft_engine->Initialize();
    check = pfft_engine->ExecuteOperation();

    for(size_t i=0; i<NM; i++)
    {
        expanded_array1[i] /= norm; //same normalization factor
        std::cout<<"zero-padded interpolated array @ "<<i<<" = "<<expanded_array1[i]<<std::endl;
    }


    // 
    // 
    // 
    // 
    // const size_t pad_factor = 2;
    // const size_t ndim = 1;
    // const size_t dval = 16;
    // const size_t dim_size[ndim] = {dval};//, dval, dval};
    // const size_t pad_dim_size[ndim] = {pad_factor*dval};//, pad_factor*dval, pad_factor*dval};
    // const size_t total_size = dim_size[0];// * dim_size[1] * dim_size[2];
    // MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> input(dim_size[0]);
    // MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> output(pad_dim_size[0]);
    // 
    // //fill up the array with a signal
    // int count = 0;
    // 
    // #ifdef HOPS_ENABLE_DEBUG_MSG
    // std::cout << "original data = " << std::endl;
    // #endif
    // for (size_t i = 0; i < dim_size[0]; i++) 
    // {
    //     input(i) = std::complex<FPTYPE>(i % 13);
    //     std::cout << input(i) << ", ";
    // }
    // std::cout<<std::endl;
    // std::cout << "--------------------------------------------------------------" << std::endl;
    // 
    // bool check;
    // FFT_TYPE* fft_engine = new FFT_TYPE();
    // 
    // fft_engine->SetPaddingFactor(pad_factor);
    // fft_engine->SetEndPadded();
    // fft_engine->SetForward();
    // fft_engine->SetInput(&input);
    // fft_engine->SetOutput(&output);
    // fft_engine->SelectAllAxes();
    // check = fft_engine->Initialize();
    // check = fft_engine->ExecuteOperation();
    // 
    // #ifdef HOPS_ENABLE_DEBUG_MSG
    // std::cout << "padded DFT of data = " << std::endl;
    // for (size_t i = 0; i < pad_dim_size[0]; i++) 
    // {
    //     std::cout << output(i) << ", ";
    // }
    // std::cout<<std::endl;
    // #endif
    // 
    // std::cout << "--------------------------------------------------------------" << std::endl;
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    //     //fill up the array with a random signal
    //     srand(0);
    //     double r1 = 0;
    //     double r2 = 0;
    //     for (size_t i = 0; i < N; i++) 
    //     {
    //         double r2 = (rand()%1024);
    //         r2 /= 1024.0;
    //         array1(i) = std::complex<FPTYPE>( (r1+r2)/2.0, 0.0);
    //         r1 = r2;
    //     }
    // 
    //     if(option == 0)
    //     {
    //         array1[N/3] = 3.0; //single peak
    //     }
    // 
    //     if(option == 1)
    //     {
    //         //double peak (true peak is mid-way between samples)
    //         array1[N/3] = 3.0;
    //         array1[N/3+1] = 3.0;
    //     }
    // 
    //     //array1[12] = 8.0;
    //     std::cout << "--------------------------------------------------------------" << std::endl;
    // 
    //     //then we execute an FFT to move to frequency space 
    //     FFT_TYPE* fft_engine = new FFT_TYPE();
    //     fft_engine->SetBackward();
    //     fft_engine->SetInput(&array1);
    //     fft_engine->SetOutput(&array2);
    //     fft_engine->Initialize();
    //     fft_engine->ExecuteOperation();
    // 
    //     // //normalized by length of FFT
    //     // double norm1 = std::sqrt( (double) N );
    //     // for(size_t i=0; i<N; i++)
    //     // {
    //     //     array2[i] /= norm1;
    //     // }
    // 
    //     //then we copy the results into an expanded array (with the middle padded by zeros)
    //     for (size_t i = 0; i < NM; i++) 
    //     {
    //         expanded_array1(i) = std::complex<FPTYPE>(0.0, 0.0);
    //         expanded_array2(i) = std::complex<FPTYPE>(0.0, 0.0);
    //     }
    // 
    // 
    //     if(false)
    //     {
    //         //now copy the array into the first portion of the expanded array 
    //         for(size_t i=0; i<N; i++)
    //         {
    //             expanded_array1(i) = array2(i);
    //         }
    //     }
    //     else 
    //     {
    //         //now copy half of the array into the first 1/4 of the expanded array-1 
    //         size_t mid = N/2;
    //         for(size_t i=0; i<mid; i++)
    //         {
    //             expanded_array1(i) = array2(i);
    //         }
    //         //split the middle point 
    //         size_t loc1 = N/2;
    //         size_t loc2 = NM - N/2;
    //         //expanded_array1(loc1) = array2(mid);
    //         expanded_array1(loc1) = array2(mid)/2.0;
    //         expanded_array1(loc2) = array2(mid)/2.0;
    //         //now copy the second half of the array into the last 1/4 of the expanded array
    //         for(size_t i=0; i<N/2; i++)
    //         {
    //             expanded_array1(loc2+1+i) = array2(mid+1+i);
    //         }
    // 
    //         for(size_t i=0; i<NM; i++)
    //         {
    //             std::cout<<"expanded array1 @ "<<i<<" = "<<expanded_array1[i]<<std::endl;
    //         }
    // 
    //     }
    // 
    //     //then we execute an inverse FFT to bring us back to original do_estimation
    //     FFT_TYPE* fft_engine2 = new FFT_TYPE();
    //     fft_engine2->SetForward();
    //     fft_engine2->SetInput(&expanded_array1);
    //     fft_engine2->SetOutput(&expanded_array2);
    //     fft_engine2->Initialize();
    //     fft_engine2->ExecuteOperation();
    // 
    //     // //normalized by length of FFT
    //     // double norm2 = std::sqrt( (double) NM );
    //     // for(size_t i=0; i<NM; i++)
    //     // {
    //     //     expanded_array2[i] /= norm2;
    //     // }
    // 
    //     //now normalized the output array 
    //     double norm = N;
    //     for(size_t i=0; i<NM; i++)
    //     {
    //         expanded_array2[i] /= norm; 
    //     }
    // 
    // 
    //     for(size_t i=0; i<N; i++)
    //     {
    //         std::cout<<"array @ "<<i<<" = "<<array1[i]<<std::endl;
    //     }
    // 
    // 
    //     for(size_t i=0; i<NM; i++)
    //     {
    //         std::cout<<"expanded array2 @ "<<i<<" = "<<expanded_array2[i]<<std::endl;
    //     }
    // 
    // 
    // 
    // 
    // 
    // 
    //     int nlags = 2*N;
    //     MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> xp_spec(4*nlags);
    //     MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> S(4*nlags);
    //     MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> xlag(4*nlags);
    //     MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> output(2*nlags);
    // 
    //     for (int i=0; i<4*nlags; i++){xp_spec[i] = 0.0;}
    //     for (int i=0; i<4*nlags; i++){S[i] = 0.0;}
    //     for (int i=0; i<4*nlags; i++){xlag[i] = 0.0;}
    // 
    //     for (int i=0; i<nlags/2; i++)
    //     {
    //         xp_spec[i] += array2[i];
    //     }
    // 
    //     //lower-sideband data
    //     for(int i = 0; i < nlags; i++)
    //     {
    //         // //factor = 1.0;// datum->lsbfrac;
    //         // // DC+highest goes into middle element of the S array
    //         // int sindex;
    //         // if(i){sindex = 4*nlags-i;}
    //         // else{sindex = 2*nlags;}
    //         // 
    //         // //sstd::complex<double> tmp2 = std::exp (I_complex * (status->lsb_phoff[0] - status->lsb_phoff[1]));
    //         // S[sindex] += std::conj (xp_spec[i] );// * tmp2 );
    // 
    // 
    //         //sstd::complex<double> tmp2 = std::exp (I_complex * (status->lsb_phoff[0] - status->lsb_phoff[1]));
    //         S[i] += xp_spec[i];
    // 
    // 
    //     }
    //     // 
    //     // for (int i=0; i<4*nlags; i++){S[i] = S[i] * factor;}
    //     // 
    //     // fFFTEngine.ExecuteOperation();
    // 
    //     FFT_TYPE* fft_engine3 = new FFT_TYPE();
    //     fft_engine3->SetForward();
    //     fft_engine3->SetInput(&S);
    //     fft_engine3->SetOutput(&xlag);
    //     fft_engine3->Initialize();
    //     fft_engine3->ExecuteOperation();
    // 
    //     for (int i = 0; i < 2*nlags; i++)
    //     {
    //         /* Translate so i=nlags is central lag */
    //         // skip every other (interpolated) lag
    //         int j = 2 * (i - nlags);
    //         if (j < 0){j += 4 * nlags;}
    //         /* re-normalize back to single lag */
    //         output[i] = xlag[j] / (double) (nlags / 2);
    //     }
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // 
    // // 
    // // fft_engine->SetPaddingFactor(pad_factor);
    // // fft_engine->SetEndPadded();
    // // fft_engine->SetBackward();
    // // fft_engine->SetInput(&output);
    // // fft_engine->SetOutput(&input);
    // // fft_engine->SelectAllAxes();
    // // fft_engine->Initialize();
    // // fft_engine->ExecuteOperation();
    // // 
    // // #ifdef HOPS_ENABLE_DEBUG_MSG
    // // std::cout << "IDFT of DFT of data = " << std::endl;
    // // #endif
    // // 
    // // FPTYPE norm = total_size;
    // // count = 0;
    // // FPTYPE l2_norm = 0;
    // // for (size_t i = 0; i < dim_size[0]; i++) {
    // //     for (size_t j = 0; j < dim_size[1]; j++) {
    // //         for (size_t k = 0; k < dim_size[2]; k++) {
    // //             //normalize out the factor of N caused by FFT -> IFFT -> result
    // //             std::complex<FPTYPE> del = input(i,j,k) / norm;
    // //             del -= std::complex<FPTYPE>(count % 13, count % 17);
    // //             l2_norm += std::real(del) * std::real(del) + std::imag(del) * std::imag(del);
    // //             count++;
    // //         }
    // //     }
    // // }
    // // 
    // // double err = std::sqrt(l2_norm);
    // // std::cout << "L2_diff = " << err << std::endl;
    // // std::cout << "L2_diff/N = "<< err/norm <<std::endl; //average error per point
    // // double tol = 1.5e-15; //tests for ndim=3, dval=19
    // // 
    // // HOPS_ASSERT_FLOAT_LESS_THAN(err/norm,tol);
    // 
    // delete fft_engine;

    return 0;
}
