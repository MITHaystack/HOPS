#include "MHO_Message.hh"
#include "MHO_FastFourierTransform.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"
#ifdef HOPS_USE_FFTW3
#include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#endif

#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <iostream>

#ifdef USE_ROOT
    #include "TCanvas.h"
    #include "TApplication.h"
    #include "TStyle.h"
    #include "TColor.h"
    #include "TGraph.h"
    #include "TGraph2D.h"
    #include "TH2D.h"
    #include "TMath.h"
    #include "TMultiGraph.h"
#endif

using namespace hops;

#ifdef HOPS_USE_FFTW3
typedef double FPTYPE;
#define FFT_TYPE MHO_MultidimensionalFastFourierTransformFFTW<FPTYPE,1>
#else
typedef double FPTYPE;
#define FFT_TYPE MHO_MultidimensionalFastFourierTransform<FPTYPE,1>
#endif


int main(int /*argc*/, char** /*argv*/)
{
    const size_t ndim = 1;
    const size_t N = 16;
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

    std::cout << "--------------------------------------------------------------" << std::endl;

    //then we execute an FFT to move to frequency space 
    FFT_TYPE* fft_engine = new FFT_TYPE();
    fft_engine->SetForward();
    fft_engine->SetInput(&array1);
    fft_engine->SetOutput(&array2);
    fft_engine->Initialize();
    fft_engine->ExecuteOperation();

    // //normalized by length of FFT
    // double norm1 = std::sqrt( (double) N );
    // for(size_t i=0; i<N; i++)
    // {
    //     array2[i] /= norm1;
    // }

    //then we copy the results into an expanded array (with the middle padded by zeros)
    for (size_t i = 0; i < NM; i++) 
    {
        expanded_array1(i) = std::complex<FPTYPE>(0.0, 0.0);
        expanded_array2(i) = std::complex<FPTYPE>(0.0, 0.0);
    }

    //now copy half of the array into the first 1/4 of the expanded array-1 
    size_t mid = N/2;
    for(size_t i=0; i<mid; i++)
    {
        expanded_array1(i) = array2(i);
    }
    //split the middle point 
    size_t loc1 = N/2;
    size_t loc2 = NM - N/2;
    expanded_array1(loc1) = array2(mid)/2.0;
    expanded_array1(loc2) = array2(mid)/2.0;
    //now copy the second half of the array into the last 1/4 of the expanded array
    for(size_t i=0; i<N/2; i++)
    {
        expanded_array1(loc2+1+i) = array2(mid+1+i);
    }

    for(size_t i=0; i<NM; i++)
    {
        std::cout<<"expanded array1 @ "<<i<<" = "<<expanded_array1[i]<<std::endl;
    }

    //then we execute an inverse FFT to bring us back to original do_estimation
    FFT_TYPE* fft_engine2 = new FFT_TYPE();
    fft_engine2->SetBackward();
    fft_engine2->SetInput(&expanded_array1);
    fft_engine2->SetOutput(&expanded_array2);
    fft_engine2->Initialize();
    fft_engine2->ExecuteOperation();

    // //normalized by length of FFT
    // double norm2 = std::sqrt( (double) NM );
    // for(size_t i=0; i<NM; i++)
    // {
    //     expanded_array2[i] /= norm2;
    // }

    //now normalized the output array 
    double norm = N;
    for(size_t i=0; i<NM; i++)
    {
        expanded_array2[i] /= norm; 
    }


    for(size_t i=0; i<N; i++)
    {
        std::cout<<"array @ "<<i<<" = "<<array1[i]<<std::endl;
    }


    for(size_t i=0; i<NM; i++)
    {
        std::cout<<"expanded array2 @ "<<i<<" = "<<expanded_array2[i]<<std::endl;
    }



    // 
    // for (int i=0; i<4*nlags; i++){xp_spec[i] = 0.0;}
    // for (int i=0; i<4*nlags; i++){S[i] = 0.0;}
    // 
    // for (int i=0; i<nlags/2; i++)
    // {
    //     z = this->fInput1->at(pp,fr,ap,i);
    //     xp_spec[i] += z;
    // }
    // 
    // //lower-sideband data
    // for(int i = 0; i < nlags; i++)
    // {
    //     factor = 1.0;// datum->lsbfrac;
    //     // DC+highest goes into middle element of the S array
    //     int sindex;
    //     if(i){sindex = 4*nlags-i;}
    //     else{sindex = 2*nlags;}
    // 
    //     //sstd::complex<double> tmp2 = std::exp (I_complex * (status->lsb_phoff[0] - status->lsb_phoff[1]));
    //     S[sindex] += factor * std::conj (xp_spec[i] );// * tmp2 );
    // }
    // 
    // for (int i=0; i<4*nlags; i++){S[i] = S[i] * factor;}
    // 
    // fFFTEngine.ExecuteOperation();
    // 
    // for (int i = 0; i < 2*nlags; i++)
    // {
    //     /* Translate so i=nlags is central lag */
    //     // skip every other (interpolated) lag
    //     int j = 2 * (i - nlags);
    //     if (j < 0){j += 4 * nlags;}
    //     /* re-normalize back to single lag */
    //     this->fOutput->at(0,fr,ap,i) = xlag[j] / (double) (nlags / 2);
    // }




    return 0;
}
