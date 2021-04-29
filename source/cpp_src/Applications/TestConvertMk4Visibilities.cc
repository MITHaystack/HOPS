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
#include "MHO_Tokenizer.hh"
#include "MHO_MK4VexInterface.hh"
#include "MHO_MK4CorelInterface.hh"

#include "MHO_Reducer.hh"
#include "MHO_FunctorBroadcaster.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"

#include "MHO_Visibilities.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_VisibilityChannelizer.hh"
#include "MHO_ChannelizedRotationFunctor.hh"

#include "MHO_BinaryFileStreamer.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_ClassIdentityMap.hh"

using namespace hops;


int main(int argc, char** argv)
{
    std::string usage = "TestConvertMk4Visibilities -r <root_filename> -f <corel_filename> -o <output_filename>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string root_filename;
    std::string corel_filename;
    std::string output_filename;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"root (vex) file", required_argument, 0, 'r'},
                                          {"corel file", required_argument, 0, 'f'},
                                          {"output file", required_argument, 0, 'o'}};

    static const char* optString = "hr:f:o:";

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
            case ('o'):
                output_filename = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }


    MHO_MK4CorelInterface mk4inter;

    mk4inter.SetCorelFile(corel_filename);
    mk4inter.SetVexFile(root_filename);
    baseline_data_type* bl_data = mk4inter.ExtractCorelFile();

    MHO_VisibilityChannelizer channelizer;
    channelizer.SetInput(bl_data);
    ch_baseline_data_type* ch_bl_data = new ch_baseline_data_type();
    channelizer.SetOutput(ch_bl_data);
    bool init = channelizer.Initialize();
    bool exe = channelizer.ExecuteOperation();
    if(exe){std::cout<<"channelizer done"<<std::endl;}
    std::string index_filename = output_filename + ".index";

    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(output_filename, index_filename);

    if(status)
    {
        uint32_t label = 0xFFFFFFFF;
        inter.Write(*ch_bl_data, label);
        inter.Close();
    }
    else
    {
        std::cout<<"error opening file"<<std::endl;
    }

    inter.Close();

    return 0;
}
