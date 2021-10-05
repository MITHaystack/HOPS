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

#include "MHO_Message.hh"

#include "MHO_DirectoryInterface.hh"
#include "MHO_BinaryFileInterface.hh"

#include "MHO_Reducer.hh"
#include "MHO_FunctorBroadcaster.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"

#include "MHO_Visibilities.hh"
#include "MHO_ChannelizedVisibilities.hh"
//#include "MHO_ChannelizedRotationFunctor.hh"




using namespace hops;


int main(int argc, char** argv)
{
    std::string usage = "TestSingleBandDelay -d <directory> -b <baseline>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string directory;
    std::string baseline;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"directory", required_argument, 0, 'd'},
                                          {"baseline", required_argument, 0, 'b'}};

    static const char* optString = "hd:b:";

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
            case ('d'):
                directory = std::string(optarg);
                break;
            case ('b'):
                baseline = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }




    //read the directory file list
    std::vector< std::string > allFiles;
    std::vector< std::string > corFiles;
    std::vector< std::string > staFiles;
    std::vector< std::string > jsonFiles;
    MHO_DirectoryInterface dirInterface;
    dirInterface.SetCurrentDirectory(directory);
    dirInterface.ReadCurrentDirectory();

    std::cout<<"directory = "<<dirInterface.GetCurrentDirectory()<<std::endl;

    dirInterface.GetFileList(allFiles);
    dirInterface.GetFilesMatchingExtention(corFiles, "cor");
    dirInterface.GetFilesMatchingExtention(staFiles, "sta");
    dirInterface.GetFilesMatchingExtention(jsonFiles, "json");


    // for(auto it = allFiles.begin(); it != allFiles.end(); it++)
    // {
    //     std::cout<<"file: "<< *it <<std::endl;
    // }

    for(auto it = corFiles.begin(); it != corFiles.end(); it++)
    {
        std::cout<<"cor: "<< *it <<std::endl;
    }

    for(auto it = staFiles.begin(); it != staFiles.end(); it++)
    {
        std::cout<<"sta: "<< *it <<std::endl;
    }


    for(auto it = jsonFiles.begin(); it != jsonFiles.end(); it++)
    {
        std::cout<<"json: "<< *it <<std::endl;
    }

    //check that there is only one json file
    std::string root_file = "";
    if(jsonFiles.size() != 1)
    {
        msg_fatal("main", "There are: "<<jsonFiles.size()<<" root files." << eom);
        std::exit(1);
    }
    else
    {
        root_file = jsonFiles[0];
    }

    //locate the corel file that contains the baseline of interest
    std::string corel_file = "";
    bool found_baseline = false;
    for(auto it = corFiles.begin(); it != corFiles.end(); it++)
    {
        std::size_t index = it->find(baseline);
        if(index != std::string::npos)
        {
            corel_file = *it;
            found_baseline = true;
        }
    }

    if(!found_baseline)
    {
        msg_fatal("main", "Could not find a file for baseline: "<< baseline << eom);
        std::exit(1);
    }

    std::cout<<"Will use root file: "<<root_file<<std::endl;
    std::cout<<"Will use corel file: "<<corel_file<<std::endl;

    //now open and read the (channelized) baseline visibility data
    ch_baseline_data_type* bl_data = new ch_baseline_data_type();
    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToRead(corel_file);
    if(status)
    {
        MHO_FileKey key;
        inter.Read(*bl_data, key);
        //std::cout<<"baseline object label = "<<blabel<<std::endl;
        std::cout<<"Total size of baseline data = "<<bl_data->GetSerializedSize()<<std::endl;
    }
    else
    {
        std::cout<<" error opening file to read"<<std::endl;
        inter.Close();
        std::exit(1);
    }
    inter.Close();

    std::size_t bl_dim[CH_VIS_NDIM];
    bl_data->GetDimensions(bl_dim);

    //create an array with 4x the number of 'lags', for zero padded interpolation
    std::size_t nbl_dim[CH_VIS_NDIM];
    bl_data->GetDimensions(nbl_dim);
    nbl_dim[CH_FREQ_AXIS] = 4*nbl_dim[CH_FREQ_AXIS];

    ch_baseline_data_type* nbl_data = new ch_baseline_data_type();
    nbl_data->Resize(nbl_dim);




    //now that the data has been organized by channel, we can take
    //each chunk and (fourier) transform it as needed
    //to do this we create a 'wrapper' about each chunk of data
    //this case, that chunk is the visibilities of a single (channel)
    //with axes of time-by-freq

    // std::size_t channel_dims[2] = {data_dims[CH_TIME_AXIS], data_dims[CH_FREQ_AXIS]};
    // MHO_NDArrayWrapper< std::complex<double>, 2> channel_wrapper(channel_dims);
    // channel_wrapper.SetExternalData( &( ch_bl_data->at(0,0,0,0) ) , channel_dims);

    //now we run an FFT on the freq axis only over each channel's data
    MHO_MultidimensionalFastFourierTransform<VFP_TYPE, CH_VIS_NDIM>* fft_engine =
        new MHO_MultidimensionalFastFourierTransform<VFP_TYPE, CH_VIS_NDIM>();
    fft_engine->DeselectAllAxes(); //default is to do all axes, so deselect them
    fft_engine->SelectAxis(CH_FREQ_AXIS); //only execute FFTs along the freq axis
    fft_engine->SetForward();
    fft_engine->SetInput(bl_data);
    fft_engine->SetOutput(bl_data);
    fft_engine->Initialize();
    fft_engine->Execute();


    std::cout<<"done fft on freq axis per channel"<<std::endl;

    #ifdef USE_ROOT
    //#ifdef DO_NOT_BUILD

    std::cout<<"starting root plotting"<<std::endl;

    int fake_argc = 0;
    char** fake_argv = nullptr;
    //ROOT stuff for plots
    TApplication* App = new TApplication("test",&fake_argc,fake_argv);
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


    //just want to take a look at the single band delay function of one channel
    //for each AP
    TMultiGraph* mg = new TMultiGraph();
    std::vector<TGraph*> graphs;
    std::size_t pol_prod = 1;
    std::size_t channel = 0;
    std::size_t nAPs = bl_dim[CH_TIME_AXIS];
    std::size_t nLags = bl_dim[CH_FREQ_AXIS];
    std::vector<double> sum; sum.resize(nLags,0);

    for(std::size_t ap = 0; ap < nAPs; ap++)
    {
        std::cout<<"ap = "<<ap<<std::endl;
        TGraph* g = new TGraph();
        for(std::size_t lag = 0; lag < nLags; lag++)
        {
            visibility_type x = bl_data->at(pol_prod, channel, ap, lag);
            double mag = abs(x);
            sum[lag] += mag;
            g->SetPoint(lag, lag, mag);
        }
        mg->Add(g);
    }

    TGraph* gsum = new TGraph();
    for(std::size_t lag = 0; lag < nLags; lag++)
    {
        gsum->SetPoint(lag,lag, sum[lag]);
    }
    gsum->SetLineColor(2);
    mg->Add(gsum);

    std::string name("test");
    TCanvas* c = new TCanvas(name.c_str(),name.c_str(), 50, 50, 950, 850);
    c->SetFillColor(0);
    c->SetRightMargin(0.2);
    c->cd();

    mg->Draw("ALP");



    App->Run();

    //#endif
    #endif //USE_ROOT










































    //
    // //now that the data has been organized by channel, we can take
    // //each chunk and (fourier) transform it as needed
    // //to do this we create a 'wrapper' about each chunk of data
    // //this case, that chunk is the visibilities of a single (channel)
    // //with axes of time-by-freq
    //
    // std::size_t channel_dims[2] = {data_dims[CH_TIME_AXIS], data_dims[CH_FREQ_AXIS]};
    // MHO_NDArrayWrapper< std::complex<double>, 2> channel_wrapper(channel_dims);
    // channel_wrapper.SetExternalData( &( ch_bl_data->at(0,0,0,0) ) , channel_dims);
    //
    // //now we run a 2-d FFT on the time and freq axes over each channel's data
    // MHO_MultidimensionalFastFourierTransform<2>* fft_engine_2d = new MHO_MultidimensionalFastFourierTransform<2>();
    // for(std::size_t pp=0; pp<data_dims[CH_POLPROD_AXIS]; pp++)
    // {
    //     for(std::size_t ch=0; ch<data_dims[CH_CHANNEL_AXIS]; ch++)
    //     {
    //         //point the wrapper to the appropriate chunk of data
    //         channel_wrapper.SetExternalData( &( ch_bl_data->at(pp,ch,0,0) ), channel_dims);
    //         fft_engine_2d->SetForward();
    //         fft_engine_2d->SetInput(&channel_wrapper);
    //         fft_engine_2d->SetOutput(&channel_wrapper);
    //         fft_engine_2d->Initialize();
    //         fft_engine_2d->Execute();
    //     }
    // }
    //
    // // std::cout<<"done with the FFT's"<<std::endl;
    // // std::vector< std::vector< std::vector< std::complex<double> > > > sbd;
    // // sbd.resize(data_dims[CH_POLPROD_AXIS]);
    // //
    // // //now collapse the time and channel axis (channels only over the first 8 chans --one sampler)
    // // for(std::size_t pp=0; pp<data_dims[CH_POLPROD_AXIS]; pp++)
    // // {
    // //     sbd[pp].resize(data_dims[CH_TIME_AXIS]);
    // //     for(std::size_t t=0; t<data_dims[CH_TIME_AXIS]; t++)
    // //     {
    // //         sbd[pp][t].resize(data_dims[CH_FREQ_AXIS], std::complex<double>(0.0, 0.0) );
    // //         for(std::size_t f=0; f<data_dims[CH_FREQ_AXIS]; f++)
    // //         {
    // //             for(std::size_t ch=0; ch<data_dims[CH_CHANNEL_AXIS]; ch++)
    // //             {
    // //                 sbd[pp][t][f] += ch_bl_data->at(pp,ch,t,f);
    // //             }
    // //         }
    // //     }
    // //
    // // }
    //
    // //TODO FIXME check this calculation
    // //lets compute the values of the transformed (freq) axis --
    // //this ought to give us the values of the 'single band delay' axis
    // MHO_NDArrayWrapper< double, 1> sbd_axis(data_dims[CH_FREQ_AXIS]);
    // int n = data_dims[CH_FREQ_AXIS];
    // int n02 = n/2;
    // for(std::size_t f=0; f<data_dims[CH_FREQ_AXIS]; f++)
    // {
    //     int tmp = f;
    //     sbd_axis(f) = (tmp - n02)*(1.0/( freq_axis_ptr->at(data_dims[CH_FREQ_AXIS]-1) - freq_axis_ptr->at(0) )  );
    //     std::cout<<"sbd_axis: "<<f<<" = "<<sbd_axis(f)<<std::endl;
    // }
    //
    // //TODO FIXME check this calculation
    // //lets compute the values of the transformed (time) axis --
    // //this ought to give us the values of the 'delay-rate' axis
    // MHO_NDArrayWrapper< double, 1> dr_axis(data_dims[CH_TIME_AXIS]);
    // int dn = data_dims[CH_TIME_AXIS];
    // int dn02 = dn/2;
    // for(std::size_t t=0; t<data_dims[CH_TIME_AXIS]; t++)
    // {
    //     int tmp = t;
    //     dr_axis(t) = (tmp - dn02)*(1.0/( time_axis_ptr->at(data_dims[CH_TIME_AXIS]-1) - time_axis_ptr->at(0) )  );
    //     std::cout<<"dr_axis: "<<t<<" = "<<dr_axis(t)<<std::endl;
    // }
    //
    //
    // //now we want to reduce (sum) the data along the "channel" axis/dimension
    // //Presumably, once we find the correct delay-rate and delay we can apply
    // //this (rotate the data of each channel) so that all of the visibilities
    // //will add coherently
    //
    // MHO_Reducer< ch_baseline_data_type::value_type,
    //             MHO_CompoundSum,
    //             ch_baseline_data_type::rank::value > summation;
    //
    // //sum all the data along the channel axis
    // summation.ReduceAxis(CH_CHANNEL_AXIS);




    // MHO_ChannelizedRotationFunctor sbd_rotation;
    // sbd_rotation.SetReferenceFrequency(0.0);
    // sbd_rotation.SetReferenceTime(0.0);
    // sbd_rotation.SetDelayRate(0.0);
    // sbd_rotation.SetSingleBandDelay(0.0);
    //
    // ch_baseline_data_type* rotated_ch_bl_data = new ch_baseline_data_type();
    // MHO_FunctorBroadcaster<ch_baseline_data_type, ch_baseline_data_type> broadcaster;
    // broadcaster.SetInput(copy_ch_bl_data);
    // broadcaster.SetOutput(rotated_ch_bl_data);
    // broadcaster.SetFunctor(&sbd_rotation);








    // #ifdef USE_ROOT
    // //#ifdef DO_NOT_BUILD
    //
    // std::cout<<"starting root plotting"<<std::endl;
    // //ROOT stuff for plots
    // TApplication* App = new TApplication("PowerPlot",&argc,argv);
    // TStyle* myStyle = new TStyle("Plain", "Plain");
    // myStyle->SetCanvasBorderMode(0);
    // myStyle->SetPadBorderMode(0);
    // myStyle->SetPadColor(0);
    // myStyle->SetCanvasColor(0);
    // myStyle->SetTitleColor(1);
    // myStyle->SetPalette(1,0);   // nice color scale for z-axis
    // myStyle->SetCanvasBorderMode(0); // gets rid of the stupid raised edge around the canvas
    // myStyle->SetTitleFillColor(0); //turns the default dove-grey background to white
    // myStyle->SetCanvasColor(0);
    // myStyle->SetPadColor(0);
    // myStyle->SetTitleFillColor(0);
    // myStyle->SetStatColor(0); //this one may not work
    // const int NRGBs = 5;
    // const int NCont = 48;
    // double stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
    // double red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
    // double green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
    // double blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
    // TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
    // myStyle->SetNumberContours(NCont);
    // myStyle->cd();
    //
    //
    // TGraph2D* g_amp[ data_dims[CH_POLPROD_AXIS] ];
    // TGraph* g_amp1[ data_dims[CH_POLPROD_AXIS] ];
    // TCanvas* c[data_dims[CH_POLPROD_AXIS] ];
    // std::vector< std::pair<double,double> > max_sbd_loc; max_sbd_loc.resize(4);
    // for(std::size_t pp=0; pp<data_dims[CH_POLPROD_AXIS]; pp++)
    // {
    //     std::stringstream ss;
    //     ss << pp;
    //     c[pp] = new TCanvas(ss.str().c_str(),ss.str().c_str(), 50, 50, 950, 850);
    //     c[pp]->SetFillColor(0);
    //     c[pp]->SetRightMargin(0.2);
    //     g_amp[pp] = new TGraph2D();
    //     g_amp1[pp] = new TGraph();
    //     std::size_t count = 0;
    //
    //     double sbd_max = 0;
    //     double sbd_max_location = 0;
    //
    //     for(std::size_t f=0; f<data_dims[CH_FREQ_AXIS]; f++)
    //     {
    //         std::complex<double> sum(0,0);
    //         for(std::size_t t=0; t<data_dims[CH_TIME_AXIS]; t++)
    //         {
    //             sum += sbd[pp][t][f];
    //             g_amp[pp]->SetPoint(count, sbd_axis(f), dr_axis(t), std::abs( sbd[pp][t][f] ) );
    //             count++;
    //         }
    //         g_amp1[pp]->SetPoint(f, sbd_axis(f), std::abs( sum ) );
    //
    //         if( sbd_max < std::abs( sum ) )
    //         {
    //             sbd_max = std::abs( sum );
    //             sbd_max_location = sbd_axis(f);
    //         }
    //     }
    //
    //     max_sbd_loc[pp] = std::make_pair(sbd_max, sbd_max_location);
    //
    //     std::cout<<"sbd_max amp, loc = "<<sbd_max<<", "<<sbd_max_location<<std::endl;
    //
    //     c[pp]->cd();
    //     g_amp1[pp]->SetMarkerStyle(20);
    //     g_amp1[pp]->SetMarkerSize(1);
    //     g_amp1[pp]->Draw("APL");
    //     //g_amp[pp]->Draw("COLZ");
    //     c[pp]->Update();
    // }
    //
    //
    //
    //
    // // for(std::size_t pp=0; pp<data_dims[CH_POLPROD_AXIS]; pp++)
    // // {
    // //     c[pp]->cd();
    // //     g_amp[pp]->SetMarkerStyle(20);
    // //     g_amp[pp]->SetMarkerSize(1);
    // //     g_amp[pp]->Draw("APL");
    // //     c[pp]->Update();
    // // }
    //
    // App->Run();
    //
    // //#endif
    // #endif //USE_ROOT







    return 0;
}
