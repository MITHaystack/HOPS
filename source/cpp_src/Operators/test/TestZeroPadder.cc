#include "MHO_Message.hh"
#include "MHO_FastFourierTransform.hh"
#include "MHO_MultidimensionalPaddedFastFourierTransform.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"
#include "MHO_ZeroPadder.hh"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <getopt.h>



using namespace hops;

typedef double FPTYPE;
#define PADDED_FFT_TYPE MHO_MultidimensionalPaddedFastFourierTransform<FPTYPE,1>
#define FFT_TYPE MHO_MultidimensionalFastFourierTransform<FPTYPE,1>

int main(int argc, char** argv)
{


    std::string usage = "TestMultidimensionalPaddedFastFourierTransform";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string input_dir;
    std::string baseline;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"toggle-padding-type", no_argument, 0, 't'},
                                          {"toggle-flip", no_argument, 0, 'f'}};

    static const char* optString = "htf";

    int pad_type = 0;
    int flip = 0;

    while(true)
    {
        char optId = getopt_long(argc, argv, optString, longOptions, NULL);
        if (optId == -1)
            break;
        switch(optId)
        {
            case ('h'):  // help
                std::cout << usage << std::endl;
                return 0;
            case ('t'):
                pad_type = 1;
                break;
            case ('f'):
                flip = 1;
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }


    //first we set up the input data
    const size_t ndim = 1;
    const size_t N = 5;
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
    //array1[N/3+1]  = 3.0;

    //then we execute an FFT to move to frequency space 
    FFT_TYPE* fft_engine = new FFT_TYPE();
    fft_engine->SetBackward();
    fft_engine->SetInput(&array1);
    fft_engine->SetOutput(&array2);
    fft_engine->Initialize();
    fft_engine->ExecuteOperation();

    MHO_ZeroPadder< std::complex<FPTYPE>, 1>* zero_padder = new MHO_ZeroPadder< std::complex<FPTYPE>,1>();;
    zero_padder->SetInput(&array2);
    zero_padder->SetOutput(&expanded_array1);
    zero_padder->SetPaddingFactor(M);

    if(pad_type == 0)
    {
        zero_padder->SetEndPadded();
        if(flip == 0){zero_padder->SetNoFlip();}
        else{zero_padder->SetFlip();}
    }
    else if(pad_type == 1)
    {
        zero_padder->SetCenterPadded();
        if(flip == 0){zero_padder->SetNoFlip();}
        else{zero_padder->SetFlip();}
    }

    zero_padder->Initialize();
    zero_padder->ExecuteOperation();

    
    for(size_t i=0; i<N; i++)
    {
        std::cout<<"original array @ "<<i<<" = "<<array2[i]<<std::endl;
    }

    std::cout << "--------------------------------------------------------------" << std::endl;

    for(size_t i=0; i<NM; i++)
    {
        std::cout<<"zero-padded array @ "<<i<<" = "<<expanded_array1[i]<<std::endl;
    }



    return 0;
}
