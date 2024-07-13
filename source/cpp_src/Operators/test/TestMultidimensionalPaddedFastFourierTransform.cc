#include "MHO_Message.hh"
#include "MHO_FastFourierTransform.hh"
#include "MHO_MultidimensionalPaddedFastFourierTransform.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <getopt.h>

using namespace hops;

typedef double FPTYPE;
#define ARRAY_TYPE MHO_NDArrayWrapper< std::complex<FPTYPE>, 1 >
#define PADDED_FFT_TYPE MHO_MultidimensionalPaddedFastFourierTransform< ARRAY_TYPE >
#define FFT_TYPE MHO_MultidimensionalFastFourierTransform< ARRAY_TYPE >

inline int positive_modulo(int i, int n) {
    return (i % n + n) % n;
}

int main(int argc, char** argv)
{


    std::string usage = "TestMultidimensionalPaddedFastFourierTransform";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string input_dir;
    std::string baseline;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"toggle-padding-type", no_argument, 0, 't'}};

    static const char* optString = "ht";

    int option = 0;

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
                option = 1;
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }


    //first we set up the input data
    const size_t ndim = 1;
    const size_t N = 16; //only even N supported
    const size_t M = 4; //even or odd M is OK
    const size_t NM = N*M;

    ARRAY_TYPE array1(N);
    ARRAY_TYPE array2(N);
    ARRAY_TYPE expanded_array1(NM);
    ARRAY_TYPE expanded_array2(NM);

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
    array1[N/3+1]  = 1.0;
    array1[N/3+2]  = 2.0;

    //then we execute an FFT to move to frequency space
    FFT_TYPE* fft_engine = new FFT_TYPE();
    fft_engine->SetBackward();
    fft_engine->SetArgs(&array1, &array2);
    fft_engine->Initialize();
    fft_engine->Execute();

    if(option == 0)
    {
        //here we run a end-padded fft interpolation

        //zero out the expanded arrays
        for (size_t i = 0; i < NM; i++)
        {
            expanded_array1(i) = std::complex<FPTYPE>(0.0, 0.0);
            expanded_array2(i) = std::complex<FPTYPE>(0.0, 0.0);
        }

        //now copy the array into the first portion of the expanded array (end-padded)
        for(size_t i=0; i<N; i++)
        {
            expanded_array1(i) = array2(i);
        }

        //then we execute an 'inverse' FFT to bring us back to original space
        FFT_TYPE* fft_engine2 = new FFT_TYPE();
        fft_engine2->SetForward();
        fft_engine2->SetArgs(&expanded_array1, &expanded_array2);
        fft_engine2->Initialize();
        fft_engine2->Execute();

        //now normalize the output array
        double norm = N;
        for(size_t i=0; i<NM; i++)
        {
            expanded_array2[i] /= norm;
        }


        for(size_t i=0; i<N; i++)
        {
            //std::cout<<"original array @ "<<i<<" = "<<array1[i]<<std::endl;
        }

        //std::cout << "--------------------------------------------------------------" << std::endl;

        for(size_t i=0; i<N; i++)
        {
            //std::cout<<"DFT'd array @ "<<i<<" = "<<array2[i]<<std::endl;
        }

        //std::cout << "--------------------------------------------------------------" << std::endl;



        for(size_t i=0; i<NM; i++)
        {
            //std::cout<<"manually zero end-padded interpolated array @ "<<i<<" = "<<expanded_array2[i]<<std::endl;
        }

        //std::cout << "--------------------------------------------------------------" << std::endl;

        //now use the zero-padded (end) fft engine to do the same thing
        bool check;
        PADDED_FFT_TYPE* pfft_engine = new PADDED_FFT_TYPE();

        pfft_engine->SetPaddingFactor(M);
        pfft_engine->SetEndPadded();
        //pfft_engine->SetCenterPadded();
        pfft_engine->SetForward();
        pfft_engine->SetArgs(&array2, &expanded_array1);
        // pfft_engine->SetInput(&array2);
        // pfft_engine->SetOutput(&expanded_array1);
        pfft_engine->SelectAllAxes();
        check = pfft_engine->Initialize();
        check = pfft_engine->Execute();

        for(size_t i=0; i<NM; i++)
        {
            expanded_array1[i] /= norm; //same normalization factor
            //std::cout<<"zero-padded interpolated array @ "<<i<<" = "<<expanded_array1[i]<<std::endl;
        }

        delete fft_engine2;
        delete pfft_engine;

    }
    else
    {

        //now do the same process with a center padded array

        //zero out the expanded arrays
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
        //expanded_array1(loc1) = array2(mid);
        expanded_array1(loc1) = array2(mid)/2.0;
        expanded_array1(loc2) = array2(mid)/2.0;
        //now copy the second half of the array into the last 1/4 of the expanded array
        for(size_t i=0; i<N/2; i++)
        {
            expanded_array1(loc2+1+i) = array2(mid+1+i);
        }

        // for(size_t i=0; i<NM; i++)
        // {
        //     //std::cout<<"expanded array1 @ "<<i<<" = "<<expanded_array1[i]<<std::endl;
        // }

        //then we execute an inverse FFT to bring us back to original space
        FFT_TYPE* fft_engine2 = new FFT_TYPE();
        fft_engine2->SetForward();
        fft_engine2->SetArgs(&expanded_array1,&expanded_array2);
        fft_engine2->Initialize();
        fft_engine2->Execute();

        //now normalized the output array
        double norm = N;
        for(size_t i=0; i<NM; i++)
        {
            expanded_array2[i] /= norm;
        }

        for(size_t i=0; i<N; i++)
        {
            //std::cout<<"original array @ "<<i<<" = "<<array1[i]<<std::endl;
        }

        //std::cout << "--------------------------------------------------------------" << std::endl;

        //std::cout << "--------------------------------------------------------------" << std::endl;

        for(size_t i=0; i<N; i++)
        {
            //std::cout<<"DFT'd array @ "<<i<<" = "<<array2[i]<<std::endl;
        }


        for(size_t i=0; i<NM; i++)
        {
            //std::cout<<"manually zero center-padded interpolated array @  "<<i<<" = "<<expanded_array2[i]<<std::endl;
        }

        //std::cout << "--------------------------------------------------------------" << std::endl;

        //now use the zero-padded (center) fft engine to do the same thing
        bool check;
        PADDED_FFT_TYPE* pfft_engine = new PADDED_FFT_TYPE();
        pfft_engine->SetPaddingFactor(M);
        pfft_engine->SetCenterPadded();
        pfft_engine->SetForward();
        pfft_engine->SetArgs(&array2, &expanded_array1);
        // pfft_engine->SetInput(&array2);
        // pfft_engine->SetOutput(&expanded_array1);

        pfft_engine->SelectAllAxes();
        check = pfft_engine->Initialize();
        check = pfft_engine->Execute();

        for(size_t i=0; i<NM; i++)
        {
            expanded_array1[i] /= norm; //same normalization factor
            //std::cout<<"zero-padded interpolated array @ "<<i<<" = "<<expanded_array1[i]<<std::endl;
        }

        delete fft_engine2;
        delete pfft_engine;

    }



    //now run the same basic code as norm_fx

    int nlags = 2*N;
    ARRAY_TYPE xp_spec(4*nlags);
    ARRAY_TYPE S(4*nlags);
    ARRAY_TYPE xlag(4*nlags);
    ARRAY_TYPE output(4*nlags);
    ARRAY_TYPE output2(4*nlags);

    for (int i=0; i<4*nlags; i++){xp_spec[i] = 0.0;}
    for (int i=0; i<4*nlags; i++){S[i] = 0.0;}
    for (int i=0; i<4*nlags; i++){xlag[i] = 0.0;}

    for (int i=0; i<nlags/2; i++)
    {
        xp_spec[i] += array2[i];
    }

    //upper-sideband data
    for(int i = 0; i < nlags; i++)
    {
        S[i] += xp_spec[i];
    }

    FFT_TYPE* fft_engine3 = new FFT_TYPE();
    fft_engine3->SetForward();
    fft_engine3->SetArgs(&S, &xlag);
    fft_engine3->Initialize();
    fft_engine3->Execute();


    for (int i = 0; i < 2*nlags; i++)
    {
        /* Translate so i=nlags is central lag */
        // skip every other (interpolated) lag
        int j = 2 * (i - nlags);
        if (j < 0){j += 4 * nlags;}
        /* re-normalize back to single lag */
        output2[i] = xlag[j] / (double) (nlags / 2);
    }



    //select every-other
    for (int i = 0; i < 2*nlags; i++)
    {
        output[i] = xlag[2*i];
    }

    //cyclic shift 2nlags
    for (int i = 0; i < 2*nlags; i++)
    {
        int j = positive_modulo(i-nlags, 2*nlags);
        xlag[i] = output[j];
    }

    //normalize
    for (int i = 0; i < 2*nlags; i++)
    {
        output[i] = xlag[i] / (double) (nlags / 2);
    }



    //
    // for (int i = 0; i < 2*nlags; i++)
    // {
    //     /* Translate so i=nlags is central lag */
    //     // skip every other (interpolated) lag
    //     // int j = 2 * (i - nlags);
    //     // if (j < 0){j += 4 * nlags;}
    //
    //     // int j = MHO_NDArrayMath::Modulus(2 * (i - nlags) , 4*nlags);
    //     //if (j < 0){j += 4 * nlags;}
    //     int j = positive_modulo(2 * (i - nlags) , 4*nlags);
    //     /* re-normalize back to single lag */
    //     output[i] = xlag[j] / (double) (nlags / 2);
    // }
    //
    //
    //


    delete fft_engine;
    delete fft_engine3;

    return 0;
}
