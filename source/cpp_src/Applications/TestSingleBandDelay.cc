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
    ch_baseline_data_type* ch_bl_data = new ch_baseline_data_type(); //no size specified yet
    channelizer.SetOutput(ch_bl_data);

    bool init = channelizer.Initialize();
    bool exe = channelizer.ExecuteOperation();

    if(exe){std::cout<<"channelizer done"<<std::endl;}

    //now that the data has been organized by channel, we can take
    //each chunk and (fourier) transform it as needed





    // #ifdef USE_ROOT
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
    // auto* x_axis = &(std::get<TIME_AXIS>(*bl_data));
    // auto* y_axis = &(std::get<FREQ_AXIS>(*bl_data));
    //
    // size_t x_axis_size = x_axis->GetSize();
    // size_t y_axis_size = y_axis->GetSize();
    //
    // //just plot phase/amp for a single channel
    // // TGraph2D *gr = new TGraph2D(x_axis_size*y_axis_size);
    // // TGraph2D *gb = new TGraph2D(x_axis_size*y_axis_size);
    // TGraph* g_amp = new TGraph();
    // TGraph* g_ph = new TGraph();
    //
    //
    // size_t count = 0;
    // auto ch = y_axis->GetFirstIntervalWithKeyValue(std::string("channel"), 12);
    //
    // if(ch != nullptr)
    // {
    //     size_t low = ch->GetLowerBound();
    //     size_t up = ch->GetUpperBound();
    //     for(size_t j=low; j<up; j++) //freq axis
    //     {
    //         std::cout<<j<<", "<<y_axis->at(j)<<std::endl;
    //         std::complex<double> sum = 0.0;
    //         for(size_t i=0; i<x_axis_size; i++) //time axis
    //         {
    //             std::complex<double> vis = bl_data->at(0,i,j); //only first pol-product
    //             sum += vis;
    //             //g->SetPoint(j,y_axis->at(j),std::arg(vis));
    //             //gr->SetPoint(count, x_axis->at(i), y_axis->at(j), std::arg(vis) );
    //             //gb->SetPoint(count, x_axis->at(i), y_axis->at(j), std::abs(vis) );
    //         }
    //         g_amp->SetPoint(j, y_axis->at(j), std::abs(sum));
    //         g_ph->SetPoint(j, y_axis->at(j), std::arg(sum));
    //         count++;
    //     }
    // }
    //
    //
    //
    // std::string name = "test";
    // TCanvas* c = new TCanvas(name.c_str(),name.c_str(), 50, 50, 950, 850);
    // c->SetFillColor(0);
    // c->SetRightMargin(0.2);
    // c->Divide(1,2);
    // c->cd(1);
    // g_amp->SetMarkerStyle(20);
    // g_amp->SetMarkerSize(1);
    // g_amp->Draw("APL");
    // c->Update();
    // c->cd(2);
    // g_ph->SetMarkerStyle(20);
    // g_ph->SetMarkerSize(1);
    // g_ph->Draw("APL");
    // c->Update();
    //
    // App->Run();
    //
    // #endif //USE_ROOT






    return 0;
}
