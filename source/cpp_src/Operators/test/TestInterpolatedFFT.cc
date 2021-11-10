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


int main(int argc, char** argv)
{

    int option = 1;

    const size_t ndim = 1;
    const size_t N = 16;
    const size_t M = 8;
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

    if(option == 0)
    {
        array1[N/3] = 3.0; //single peak
    }

    if(option == 1)
    {
        //double peak (true peak is mid-way between samples)
        array1[N/3] = 3.0;
        array1[N/3+1] = 3.0;
    }

    //array1[12] = 8.0;
    std::cout << "--------------------------------------------------------------" << std::endl;

    //then we execute an FFT to move to frequency space
    FFT_TYPE* fft_engine = new FFT_TYPE();
    fft_engine->SetBackward();
    fft_engine->SetArgs(&array1, &array2);
    // fft_engine->SetInput(&array1);
    // fft_engine->SetOutput(&array2);
    fft_engine->Initialize();
    fft_engine->Execute();

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


    if(false)
    {
        //now copy the array into the first portion of the expanded array
        for(size_t i=0; i<N; i++)
        {
            expanded_array1(i) = array2(i);
        }
    }
    else
    {
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

        for(size_t i=0; i<NM; i++)
        {
            std::cout<<"expanded array1 @ "<<i<<" = "<<expanded_array1[i]<<std::endl;
        }

    }

    //then we execute an inverse FFT to bring us back to original do_estimation
    FFT_TYPE* fft_engine2 = new FFT_TYPE();
    fft_engine2->SetForward();
    fft_engine2->SetArgs(&expanded_array1, &expanded_array2);
    // fft_engine2->SetInput(&expanded_array1);
    // fft_engine2->SetOutput(&expanded_array2);
    fft_engine2->Initialize();
    fft_engine2->Execute();

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






    int nlags = 2*N;
    MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> xp_spec(4*nlags);
    MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> S(4*nlags);
    MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> xlag(4*nlags);
    MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> output(2*nlags);

    for (int i=0; i<4*nlags; i++){xp_spec[i] = 0.0;}
    for (int i=0; i<4*nlags; i++){S[i] = 0.0;}
    for (int i=0; i<4*nlags; i++){xlag[i] = 0.0;}

    for (int i=0; i<nlags/2; i++)
    {
        xp_spec[i] += array2[i];
    }

    //lower-sideband data
    for(int i = 0; i < nlags; i++)
    {
        // //factor = 1.0;// datum->lsbfrac;
        // // DC+highest goes into middle element of the S array
        // int sindex;
        // if(i){sindex = 4*nlags-i;}
        // else{sindex = 2*nlags;}
        //
        // //sstd::complex<double> tmp2 = std::exp (I_complex * (status->lsb_phoff[0] - status->lsb_phoff[1]));
        // S[sindex] += std::conj (xp_spec[i] );// * tmp2 );


        //sstd::complex<double> tmp2 = std::exp (I_complex * (status->lsb_phoff[0] - status->lsb_phoff[1]));
        S[i] += xp_spec[i];


    }
    //
    // for (int i=0; i<4*nlags; i++){S[i] = S[i] * factor;}
    //
    // fFFTEngine.Execute();

    FFT_TYPE* fft_engine3 = new FFT_TYPE();
    fft_engine3->SetForward();
    fft_engine3->SetArgs(&S,&xlag);
    // fft_engine3->SetInput(&S);
    // fft_engine3->SetOutput(&xlag);
    fft_engine3->Initialize();
    fft_engine3->Execute();

    for (int i = 0; i < 2*nlags; i++)
    {
        /* Translate so i=nlags is central lag */
        // skip every other (interpolated) lag
        int j = 2 * (i - nlags);
        if (j < 0){j += 4 * nlags;}
        /* re-normalize back to single lag */
        output[i] = xlag[j] / (double) (nlags / 2);
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

    TGraph* g = new TGraph();
    TGraph* gint = new TGraph();
    TGraph* gunk = new TGraph();
    for(size_t i=0; i<N; i++)
    {
        g->SetPoint(i,i,std::abs(array1[i]));
    }

    for(size_t i=0; i<NM; i++)
    {
        double x = (double)i/(double)M;
        gint->SetPoint(i,x, std::abs(expanded_array2[i]) );
    }

    for(size_t i=0; i<2*nlags;i++)
    {
        double x=i;
        x /= 8;
        std::cout<<xlag[i]<<std::endl;
        gunk->SetPoint(i, x, std::abs(xlag[i])/(nlags/2.));
    }

    g->SetMarkerColor(1);
    g->SetMarkerStyle(24);
    g->SetLineColor(1);
    g->SetLineWidth(4);

    gint->SetMarkerColor(2);
    gint->SetMarkerStyle(25);
    gint->SetLineColor(2);

    gunk->SetMarkerColor(4);
    gunk->SetMarkerStyle(21);
    gunk->SetLineColor(4);

    std::string name("test");
    TCanvas* c = new TCanvas(name.c_str(),name.c_str(), 50, 50, 950, 850);
    c->SetFillColor(0);
    c->SetRightMargin(0.2);
    //mg->Draw("ap");

    gunk->Draw("ALP");
    g->Draw("LPSAME");
    gint->Draw("LPSAME");
    //gunk->Draw("LPSAME");

    App->Run();


    #endif




















    return 0;
}
