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
#include <getopt.h>

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
    const size_t N = 64;
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
    array1[N/3+1]  = 3.0;

    //then we execute an FFT to move to frequency space 
    FFT_TYPE* fft_engine = new FFT_TYPE();
    fft_engine->SetBackward();
    fft_engine->SetInput(&array1);
    fft_engine->SetOutput(&array2);
    fft_engine->Initialize();
    fft_engine->ExecuteOperation();

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
        fft_engine2->SetInput(&expanded_array1);
        fft_engine2->SetOutput(&expanded_array2);
        fft_engine2->Initialize();
        fft_engine2->ExecuteOperation();

        //now normalize the output array 
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
            std::cout<<"manually zero end-padded interpolated array @ "<<i<<" = "<<expanded_array2[i]<<std::endl;
        }

        std::cout << "--------------------------------------------------------------" << std::endl;

        //now use the zero-padded (end) fft engine to do the same thing
        bool check;
        PADDED_FFT_TYPE* pfft_engine = new PADDED_FFT_TYPE();
        
        pfft_engine->SetPaddingFactor(M);
        pfft_engine->SetEndPadded();
        //pfft_engine->SetCenterPadded();
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
        //     std::cout<<"expanded array1 @ "<<i<<" = "<<expanded_array1[i]<<std::endl;
        // }

        //then we execute an inverse FFT to bring us back to original space
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
            std::cout<<"manually zero center-padded interpolated array @  "<<i<<" = "<<expanded_array2[i]<<std::endl;
        }

        std::cout << "--------------------------------------------------------------" << std::endl;

        //now use the zero-padded (center) fft engine to do the same thing
        bool check;
        PADDED_FFT_TYPE* pfft_engine = new PADDED_FFT_TYPE();
        
        pfft_engine->SetPaddingFactor(M);
        pfft_engine->SetCenterPadded();
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

    }



    #ifdef USE_ROOT

    std::cout<<"starting root plotting"<<std::endl;

    //ROOT stuff for plots
    TApplication* App = new TApplication("Plot",&argc,argv);
    TStyle* myStyle = new TStyle("Plain", "Plain");
    myStyle->SetCanvasBorderMode(0);
    myStyle->SetPadBorderMode(0);
    myStyle->SetPadColor(0);
    myStyle->SetCanvasColor(0);
    myStyle->SetTitleColor(1);
    myStyle->SetPalette(1,0);   // nice color scale for z-axis
    myStyle->SetCanvasBorderMode(0); // gets rid of the stupid raised edge around the canvas
    myStyle->SetTitleFillColor(0); //turns the default dove-grey background to white
    myStyle->SetCanvasColor(0);
    myStyle->SetPadColor(0);
    myStyle->SetTitleFillColor(0);
    myStyle->SetStatColor(0); //this one may not work
    const int NRGBs = 5;
    const int NCont = 48;
    double stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
    double red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
    double green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
    double blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
    TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    myStyle->SetNumberContours(NCont);
    myStyle->cd();

    TGraph* g_real = new TGraph();  
    TGraph* g_imag = new TGraph();  

    TGraph* gint_real = new TGraph();
    TGraph* gint_imag = new TGraph();

    for(size_t i=0; i<N; i++)
    {
        g_real->SetPoint(i,i,array1[i].real() );
        g_imag->SetPoint(i,i,array1[i].imag() );
    }

    for(size_t i=0; i<NM; i++)
    {
        double x = (double)i/(double)M;
        gint_real->SetPoint(i,x,expanded_array2[i].real() );
        gint_imag->SetPoint(i,x,expanded_array2[i].imag() );
    }

    g_real->SetMarkerColor(1);
    g_real->SetMarkerStyle(24);
    g_real->SetLineColor(1);
    g_real->SetLineWidth(4);

    gint_real->SetMarkerColor(2);
    gint_real->SetMarkerStyle(25);
    gint_real->SetLineColor(2);

    g_imag->SetMarkerColor(1);
    g_imag->SetMarkerStyle(24);
    g_imag->SetLineColor(1);
    g_imag->SetLineWidth(4);

    gint_imag->SetMarkerColor(2);
    gint_imag->SetMarkerStyle(25);
    gint_imag->SetLineColor(2);

    std::string name("test");
    TCanvas* c = new TCanvas(name.c_str(),name.c_str(), 50, 50, 950, 850);
    c->SetFillColor(0);
    c->SetRightMargin(0.2);
    c->Divide(1,2);
    c->cd(1);
    //mg->Draw("ap");

    g_real->Draw("ALP");
    gint_real->Draw("LPSAME");

    c->cd(2);

    //gunk->Draw("ALP");
    g_imag->Draw("ALP");
    gint_imag->Draw("LPSAME");
    //gunk->Draw("LPSAME");

    App->Run();


    #endif 





    return 0;
}
