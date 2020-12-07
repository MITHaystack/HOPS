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

#include "MHOReducer.hh"
#include "MHOFunctorBroadcaster.hh"
#include "MHOMultidimensionalFastFourierTransform.hh"

#include "MHOVisibilities.hh"
#include "MHOChannelizedVisibilities.hh"
#include "MHOVisibilityChannelizer.hh"
#include "MHOChannelizedRotationFunctor.hh"




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

    //clone the data for later use
    ch_baseline_data_type* copy_ch_bl_data = ch_bl_data->Clone();

    std::size_t data_dims[CH_VIS_NDIM];
    ch_bl_data->GetDimensions(data_dims);
    auto freq_axis_ptr = &(std::get<CH_FREQ_AXIS>( *ch_bl_data ) );
    auto time_axis_ptr = &(std::get<CH_TIME_AXIS>( *ch_bl_data ) );


    // double pass_low =  86242e6 - 86226e6;
    // double pass_high = 86242.5e6 - 86226e6;
    // for(std::size_t pp=0; pp<data_dims[CH_POLPROD_AXIS]; pp++)
    // {
    //     for(std::size_t ch=0; ch<data_dims[CH_CHANNEL_AXIS]; ch++)
    //     {
    //         for(std::size_t t=0; t<data_dims[CH_TIME_AXIS]; t++)
    //         {
    //             for(std::size_t f=0; f<data_dims[CH_FREQ_AXIS]; f++)
    //             {
    //                 if( freq_axis_ptr->at(f) < pass_low || freq_axis_ptr->at(f) > pass_high )
    //                 {
    //                     //zero-out data outside of passband
    //                     std::cout<<"zeroing out data at:" << freq_axis_ptr->at(f)<<std::endl;
    //                     ch_bl_data->at(pp,ch,t,f) = std::complex<double>(0.0, 0.0);
    //                 }
    //             }
    //         }
    //     }
    // }

    //now that the data has been organized by channel, we can take
    //each chunk and (fourier) transform it as needed
    //to do this we create a 'wrapper' about each chunk of data
    //this case, that chunk is the visibilities of a single (channel)
    //with axes of time-by-freq

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

    // std::cout<<"done with the FFT's"<<std::endl;
    // std::vector< std::vector< std::vector< std::complex<double> > > > sbd;
    // sbd.resize(data_dims[CH_POLPROD_AXIS]);
    // 
    // //now collapse the time and channel axis (channels only over the first 8 chans --one sampler)
    // for(std::size_t pp=0; pp<data_dims[CH_POLPROD_AXIS]; pp++)
    // {
    //     sbd[pp].resize(data_dims[CH_TIME_AXIS]);
    //     for(std::size_t t=0; t<data_dims[CH_TIME_AXIS]; t++)
    //     {
    //         sbd[pp][t].resize(data_dims[CH_FREQ_AXIS], std::complex<double>(0.0, 0.0) );
    //         for(std::size_t f=0; f<data_dims[CH_FREQ_AXIS]; f++)
    //         {
    //             for(std::size_t ch=0; ch<data_dims[CH_CHANNEL_AXIS]; ch++)
    //             {
    //                 sbd[pp][t][f] += ch_bl_data->at(pp,ch,t,f);
    //             }
    //         }
    //     }
    // 
    // }

    //TODO FIXME check this calculation
    //lets compute the values of the transformed (freq) axis --
    //this ought to give us the values of the 'single band delay' axis
    MHOArrayWrapper< double, 1> sbd_axis(data_dims[CH_FREQ_AXIS]);
    int n = data_dims[CH_FREQ_AXIS];
    int n02 = n/2;
    for(std::size_t f=0; f<data_dims[CH_FREQ_AXIS]; f++)
    {
        int tmp = f;
        sbd_axis(f) = (tmp - n02)*(1.0/( freq_axis_ptr->at(data_dims[CH_FREQ_AXIS]-1) - freq_axis_ptr->at(0) )  );
        std::cout<<"sbd_axis: "<<f<<" = "<<sbd_axis(f)<<std::endl;
    }

    //TODO FIXME check this calculation
    //lets compute the values of the transformed (time) axis --
    //this ought to give us the values of the 'delay-rate' axis
    MHOArrayWrapper< double, 1> dr_axis(data_dims[CH_TIME_AXIS]);
    int dn = data_dims[CH_TIME_AXIS];
    int dn02 = dn/2;
    for(std::size_t t=0; t<data_dims[CH_TIME_AXIS]; t++)
    {
        int tmp = t;
        dr_axis(t) = (tmp - dn02)*(1.0/( time_axis_ptr->at(data_dims[CH_TIME_AXIS]-1) - time_axis_ptr->at(0) )  );
        std::cout<<"dr_axis: "<<t<<" = "<<dr_axis(t)<<std::endl;
    }


    //now we want to reduce (sum) the data along the "channel" axis/dimension
    //Presumably, once we find the correct delay-rate and delay we can apply 
    //this (rotate the data of each channel) so that all of the visibilities 
    //will add coherently

    MHOReducer< ch_baseline_data_type::value_type,
                MHOCompoundSum<ch_baseline_data_type::value_type>,
                ch_baseline_data_type::rank::value > summation;
    
    //sum all the data along the channel axis 
    summation.ReduceAxis(CH_CHANNEL_AXIS); 




    // MHOChannelizedRotationFunctor sbd_rotation;
    // sbd_rotation.SetReferenceFrequency(0.0);
    // sbd_rotation.SetReferenceTime(0.0);
    // sbd_rotation.SetDelayRate(0.0);
    // sbd_rotation.SetSingleBandDelay(0.0);
    // 
    // ch_baseline_data_type* rotated_ch_bl_data = new ch_baseline_data_type();
    // MHOFunctorBroadcaster<ch_baseline_data_type, ch_baseline_data_type> broadcaster;
    // broadcaster.SetInput(copy_ch_bl_data);
    // broadcaster.SetOutput(rotated_ch_bl_data);
    // broadcaster.SetFunctor(&sbd_rotation);
    

    
    
    
    


    #ifdef USE_ROOT
    //#ifdef DO_NOT_BUILD

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


    TGraph2D* g_amp[ data_dims[CH_POLPROD_AXIS] ];
    TGraph* g_amp1[ data_dims[CH_POLPROD_AXIS] ];
    TCanvas* c[data_dims[CH_POLPROD_AXIS] ];
    std::vector< std::pair<double,double> > max_sbd_loc; max_sbd_loc.resize(4);
    for(std::size_t pp=0; pp<data_dims[CH_POLPROD_AXIS]; pp++)
    {
        std::stringstream ss;
        ss << pp;
        c[pp] = new TCanvas(ss.str().c_str(),ss.str().c_str(), 50, 50, 950, 850);
        c[pp]->SetFillColor(0);
        c[pp]->SetRightMargin(0.2);
        g_amp[pp] = new TGraph2D();
        g_amp1[pp] = new TGraph();
        std::size_t count = 0;

        double sbd_max = 0;
        double sbd_max_location = 0;

        for(std::size_t f=0; f<data_dims[CH_FREQ_AXIS]; f++)
        {
            std::complex<double> sum(0,0);
            for(std::size_t t=0; t<data_dims[CH_TIME_AXIS]; t++)
            {
                sum += sbd[pp][t][f];
                g_amp[pp]->SetPoint(count, sbd_axis(f), dr_axis(t), std::abs( sbd[pp][t][f] ) );
                count++;
            }
            g_amp1[pp]->SetPoint(f, sbd_axis(f), std::abs( sum ) );

            if( sbd_max < std::abs( sum ) )
            {
                sbd_max = std::abs( sum );
                sbd_max_location = sbd_axis(f);
            }
        }

        max_sbd_loc[pp] = std::make_pair(sbd_max, sbd_max_location);

        std::cout<<"sbd_max amp, loc = "<<sbd_max<<", "<<sbd_max_location<<std::endl;

        c[pp]->cd();
        g_amp1[pp]->SetMarkerStyle(20);
        g_amp1[pp]->SetMarkerSize(1);
        g_amp1[pp]->Draw("APL");
        //g_amp[pp]->Draw("COLZ");
        c[pp]->Update();
    }




    // for(std::size_t pp=0; pp<data_dims[CH_POLPROD_AXIS]; pp++)
    // {
    //     c[pp]->cd();
    //     g_amp[pp]->SetMarkerStyle(20);
    //     g_amp[pp]->SetMarkerSize(1);
    //     g_amp[pp]->Draw("APL");
    //     c[pp]->Update();
    // }

    App->Run();

    //#endif
    #endif //USE_ROOT







    return 0;
}
