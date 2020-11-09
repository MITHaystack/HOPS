#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <utility>
#include <map>
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

#include "MHOMessage.hh"
#include "MHOTokenizer.hh"
#include "MHOMK4VexInterface.hh"
#include "MHOMK4CorelInterface.hh"

#include "MHOVisibilities.hh"
#include "MHOChannelizedVisibilities.hh"
#include "MHOVisibilityChannelizer.hh"

#include "MHOMultidimensionalFastFourierTransform.hh"


using namespace hops;


int main(int argc, char** argv)
{
    std::string usage = "TestSingleBandDelay -r <root_filename> -f <corel_filename>";

    MHOMessage::GetInstance().AcceptAllKeys();
    MHOMessage::GetInstance().SetMessageLevel(eDebug);

    std::string root_filename;
    std::string corel_filename;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"root (vex) file", required_argument, 0, 'r'},
                                          {"corel file", required_argument, 0, 'f'}};

    static const char* optString = "hr:f:";

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
            case ('r'):
                root_filename = std::string(optarg);
                break;
            case ('f'):
                corel_filename = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }


    MHOMK4CorelInterface mk4inter;

    mk4inter.SetCorelFile(corel_filename);
    mk4inter.SetVexFile(root_filename);
    baseline_data_type* bl_data = mk4inter.ExtractCorelFile();


    MHOVisibilityChannelizer channelizer;
    channelizer.SetInput(bl_data);
    ch_baseline_data_type* ch_bl_data = new ch_baseline_data_type();
    channelizer.SetOutput(ch_bl_data);
    bool init = channelizer.Initialize();
    bool exe = channelizer.ExecuteOperation();
    if(exe){std::cout<<"channelizer done"<<std::endl;}

    //now that the data has been organized by channel, we can take
    //each chunk and (fourier) transform it as needed
    //to do this we create a 'wrapper' to interface with each data chunk
    std::size_t data_dims[CH_VIS_NDIM];
    ch_bl_data->GetDimensions(data_dims);
    std::size_t channel_dims[2] = {data_dims[CH_TIME_AXIS], data_dims[CH_FREQ_AXIS]};
    MHOArrayWrapper< std::complex<double>, 2> channel_wrapper(channel_dims); 
    channel_wrapper.SetExternalData( &( ch_bl_data->at(0,0,0,0) ) , channel_dims);

    //now we run a 2-d FFT on the time and freq axes over each channel's data
    MHOMultidimensionalFastFourierTransform<2>* fft_engine_2d = new MHOMultidimensionalFastFourierTransform<2>();
    for(std::size_t pp=0; pp<data_dims[CH_POLPROD_AXIS]; pp++)
    {
        for(std::size_t ch=0; ch<data_dims[CH_CHANNEL_AXIS]; ch++)
        {
            //point the wrapper to the appropriate chunk of data
            channel_wrapper.SetExternalData( &( ch_bl_data->at(pp,ch,0,0) ), channel_dims);
            fft_engine_2d->SetForward();
            fft_engine_2d->SetInput(&channel_wrapper);
            fft_engine_2d->SetOutput(&channel_wrapper);
            fft_engine_2d->Initialize();
            fft_engine_2d->ExecuteOperation();
        }
    }

    std::cout<<"done with the FFT's"<<std::endl;

    std::vector< std::vector< std::complex<double> > > sbd;
    sbd.resize(data_dims[CH_POLPROD_AXIS]);

    //now collapse the time and channel axis (channels only over the first 8 chans --one sampler)
    for(std::size_t pp=0; pp<data_dims[CH_POLPROD_AXIS]; pp++)
    {
        sbd[pp].resize(data_dims[CH_FREQ_AXIS], std::complex<double>(0.0, 0.0) );
        for(std::size_t ch=0; ch<data_dims[CH_CHANNEL_AXIS]; ch++)
        {
            for(std::size_t t=0; t<data_dims[CH_TIME_AXIS]; t++)
            {
                for(std::size_t f=0; f<data_dims[CH_FREQ_AXIS]; f++)
                {
                    sbd[pp][f] += ch_bl_data->at(pp,ch,t,f);
                }
            }
        }
    }

    //now we want determine the 'single band delay' axis
    auto freq_axis_ptr = &(std::get<CH_FREQ_AXIS>( *ch_bl_data ) );
    MHOArrayWrapper< double, 1> sbd_axis(data_dims[CH_FREQ_AXIS]);
    int n = data_dims[CH_FREQ_AXIS];
    int n02 = n/2;
    for(std::size_t f=0; f<data_dims[CH_FREQ_AXIS]; f++)
    {
        int tmp = f;
        sbd_axis(f) = (tmp - n02)*(1.0/32e6);
        std::cout<<"sbd_axis: "<<f<<" = "<<sbd_axis(f)<<std::endl;
    }


    #ifdef USE_ROOT
    
    std::cout<<"starting root plotting"<<std::endl;
    //ROOT stuff for plots
    TApplication* App = new TApplication("PowerPlot",&argc,argv);
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
    

    TGraph* g_amp[ data_dims[CH_POLPROD_AXIS] ];
    TCanvas* c[data_dims[CH_POLPROD_AXIS] ];


    for(std::size_t pp=0; pp<data_dims[CH_POLPROD_AXIS]; pp++)
    {
        std::stringstream ss; 
        ss << pp;
        c[pp] = new TCanvas(ss.str().c_str(),ss.str().c_str(), 50, 50, 950, 850);
        c[pp]->SetFillColor(0);
        c[pp]->SetRightMargin(0.2);
        g_amp[pp] = new TGraph();
        for(std::size_t f=0; f<data_dims[CH_FREQ_AXIS]; f++)
        {
            g_amp[pp]->SetPoint(f, sbd_axis.at(f), std::abs( sbd[pp][f]) );
        }
    }


    for(std::size_t pp=0; pp<data_dims[CH_POLPROD_AXIS]; pp++)
    {
        c[pp]->cd();
        g_amp[pp]->SetMarkerStyle(20);
        g_amp[pp]->SetMarkerSize(1);
        g_amp[pp]->Draw("APL");
        c[pp]->Update();
    }
    
    App->Run();
    
    #endif //USE_ROOT






    return 0;
}
