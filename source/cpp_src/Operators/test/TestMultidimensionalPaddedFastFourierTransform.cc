#include "MHO_Message.hh"
#include "MHO_FastFourierTransform.hh"
#include "MHO_MultidimensionalPaddedFastFourierTransform.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"

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
    array1[N/3+1]  = 1.0;
    array1[N/3+2]  = 2.0;

    //then we execute an FFT to move to frequency space 
    FFT_TYPE* fft_engine = new FFT_TYPE();
    fft_engine->SetBackward();
    fft_engine->SetInput(&array1);
    fft_engine->SetOutput(&array2);
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
        fft_engine2->SetInput(&expanded_array1);
        fft_engine2->SetOutput(&expanded_array2);
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
            std::cout<<"original array @ "<<i<<" = "<<array1[i]<<std::endl;
        }

        std::cout << "--------------------------------------------------------------" << std::endl;

        for(size_t i=0; i<N; i++)
        {
            std::cout<<"DFT'd array @ "<<i<<" = "<<array2[i]<<std::endl;
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
        check = pfft_engine->Execute();

        for(size_t i=0; i<NM; i++)
        {
            expanded_array1[i] /= norm; //same normalization factor
            std::cout<<"zero-padded interpolated array @ "<<i<<" = "<<expanded_array1[i]<<std::endl;
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
        //     std::cout<<"expanded array1 @ "<<i<<" = "<<expanded_array1[i]<<std::endl;
        // }

        //then we execute an inverse FFT to bring us back to original space
        FFT_TYPE* fft_engine2 = new FFT_TYPE();
        fft_engine2->SetForward();
        fft_engine2->SetInput(&expanded_array1);
        fft_engine2->SetOutput(&expanded_array2);
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
            std::cout<<"original array @ "<<i<<" = "<<array1[i]<<std::endl;
        }

        std::cout << "--------------------------------------------------------------" << std::endl;

        std::cout << "--------------------------------------------------------------" << std::endl;

        for(size_t i=0; i<N; i++)
        {
            std::cout<<"DFT'd array @ "<<i<<" = "<<array2[i]<<std::endl;
        }


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
        check = pfft_engine->Execute();

        for(size_t i=0; i<NM; i++)
        {
            expanded_array1[i] /= norm; //same normalization factor
            std::cout<<"zero-padded interpolated array @ "<<i<<" = "<<expanded_array1[i]<<std::endl;
        }

        delete fft_engine2;
        delete pfft_engine;

    }



    //now run the same basic code as norm_fx 

    int nlags = 2*N;
    MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> xp_spec(4*nlags);
    MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> S(4*nlags);
    MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> xlag(4*nlags);
    MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> output(4*nlags);
    MHO_NDArrayWrapper< std::complex<FPTYPE>, ndim> output2(4*nlags);

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
    fft_engine3->SetInput(&S);
    fft_engine3->SetOutput(&xlag);
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

    TGraph* gunk_real = new TGraph();
    TGraph* gunk_imag = new TGraph();

    TGraph* gunk2_real = new TGraph();
    TGraph* gunk2_imag = new TGraph();

    for(size_t i=0; i<N; i++)
    {
        g_real->SetPoint(i,i,array1[i].real() );
        g_imag->SetPoint(i,i,array1[i].imag() );
    }

    for(size_t i=0; i<NM; i++)
    {
        double x = (double)i/(double)M; //rescale back to original spacing
        gint_real->SetPoint(i,x,expanded_array1[i].real() );
        gint_imag->SetPoint(i,x,expanded_array1[i].imag() );
    }

    //have to do the following in two parts to keep the ordering correct
    //what purpose does the shift have?
    size_t count=0;
    for(size_t i=0; i<2*nlags;i++)
    {
        //double x = (i+nlags)%(2*nlags);
        double x = (double)i/4.; //rescale and shift back to original spacing so we can compare
        //std::cout<<output[i].real()<<std::endl;
        gunk_real->SetPoint(count, x, output[i].real());
        gunk_imag->SetPoint(count, x, output[i].imag());
        count++;
    }


    for(size_t i=0; i<2*nlags;i++)
    {
        double x = (double)i/4.;
        //x /= 4; //rescale and shift back to original spacing so we can compare
        //std::cout<<output[i].real()<<std::endl;
        gunk2_real->SetPoint(count, x, output2[i].real());
        gunk2_imag->SetPoint(count, x, output2[i].imag());
        count++;
    }




    g_real->SetMarkerColor(1);
    g_real->SetMarkerStyle(24);
    g_real->SetLineColor(1);
    g_real->SetLineWidth(4);
    g_imag->SetMarkerColor(1);
    g_imag->SetMarkerStyle(24);
    g_imag->SetLineColor(1);
    g_imag->SetLineWidth(4);

    gint_real->SetMarkerColor(2);
    gint_real->SetMarkerStyle(25);
    gint_real->SetLineColor(2);
    gint_imag->SetMarkerColor(2);
    gint_imag->SetMarkerStyle(25);
    gint_imag->SetLineColor(2);

    gunk_real->SetMarkerColor(4);
    gunk_real->SetMarkerStyle(29);
    gunk_real->SetLineColor(4);
    gunk_real->SetLineWidth(5);

    gunk_imag->SetMarkerColor(4);
    gunk_imag->SetMarkerStyle(29);
    gunk_imag->SetLineColor(4);
    gunk_imag->SetLineWidth(5);

    gunk2_real->SetMarkerColor(3);
    gunk2_real->SetMarkerStyle(26);
    gunk2_real->SetLineColor(3);
    gunk2_imag->SetMarkerColor(3);
    gunk2_imag->SetMarkerStyle(26);
    gunk2_imag->SetLineColor(3);


    std::string name("test");
    TCanvas* c = new TCanvas(name.c_str(),name.c_str(), 50, 50, 950, 850);
    c->SetFillColor(0);
    c->SetRightMargin(0.2);
    c->Divide(1,2);
    c->cd(1);
    //mg->Draw("ap");

    gint_real->Draw("ALP");
    gint_real->GetYaxis()->SetTitle("Real");
    g_real->Draw("LPSAME");
    gunk_real->Draw("LPSAME");
    gunk2_real->Draw("LPSAME");

    c->cd(2);

    //gunk->Draw("ALP");
    gunk_imag->Draw("ALP");
    gunk_imag->GetYaxis()->SetTitle("Imag");
    gint_imag->Draw("LPSAME");
    g_imag->Draw("LPSAME");
    gunk2_imag->Draw("LPSAME");
    //gunk->Draw("LPSAME");

    App->Run();


    #endif 





    return 0;
}
