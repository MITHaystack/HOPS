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

    //we don't bother normalizing anything, just test ability to search for a delay

    auto* x_axis = &(std::get<TIME_AXIS>(*bl_data));
    auto* y_axis = &(std::get<FREQ_AXIS>(*bl_data));

    std::cout<<"trying to find channel 0"<<std::endl;
    for(int i=0; i<32; i++)
    {
        auto ch = y_axis->GetFirstIntervalWithKeyValue(std::string("channel"), i);
        size_t low = ch->GetLowerBound();
        size_t up = ch->GetUpperBound();
        std::cout<<"channel: "<<i<<": "<<low<<", "<<up<<std::endl;
    }





    return 0;
}
