#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <getopt.h>
#include <iomanip>

#include "msg.h"
#include "ffcontrol.h"
struct c_block* cb_head; //global extern kludge (due to stupid c-library interface)

//global messaging util
#include "MHO_Message.hh"

//control wrapper
#include "MHO_ControlBlockWrapper.hh"

using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "TestControlFileParsePCPhases -c <control file>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string control_file = "";
    bool ok;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"control", required_argument, 0, 'c'}};

    static const char* optString = "hc:";

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
            case ('c'):
                control_file = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    if( control_file == "")
    {
        std::cout << usage << std::endl;
        return 1;
    }

    std::string baseline = "GE";

    //make a fake vex object with just the information we need 
    std::string vex_info_string = " \
    { \
        \"$SITE\": \
        { \
            \"GGAO12M\": \
            { \
                \"mk4_site_ID\": \"G\", \
                \"site_ID\": \"Gs\", \
                \"site_name\": \"GGAO12M\", \
                \"site_type\": \"fixed\" \
            }, \
            \"WESTFORD\": \
            { \
                \"mk4_site_ID\": \"E\", \
                \"site_ID\": \"Wf\", \
                \"site_name\": \"WESTFORD\", \
                \"site_type\": \"fixed\" \
            } \
        }, \
        \"VEX_rev\": \"ovex\" \
    }" ;

    mho_json vexInfo = mho_json::parse(vex_info_string);

    ////////////////////////////////////////////////////////////////////////////
    //CONTROL BLOCK CONSTRUCTION
    ////////////////////////////////////////////////////////////////////////////
    
    //parse the control file
    cb_head = (struct c_block *) malloc (sizeof (struct c_block) );
    struct c_block* cb_out = (struct c_block *) malloc (sizeof (struct c_block) );
    char bl[2]; bl[0] = baseline[0]; bl[1] = baseline[1];
    std::string src = " ";
    char fgroup = 'X';
    int time = 0;
    int retval = construct_cblock(const_cast<char*>(control_file.c_str()), cb_head, cb_out, bl, const_cast<char*>(src.c_str()), fgroup, time);
    MHO_ControlBlockWrapper cb_wrapper(cb_out, vexInfo, baseline);
    
    //construct the pcal array...need to re-think how we are going to move control block info around (scalar parameters vs. arrays etc)
    manual_pcal_type* ref_pcal = cb_wrapper.GetRefStationManualPCOffsets(); 
    manual_pcal_type* rem_pcal = cb_wrapper.GetRemStationManualPCOffsets();
    
    std::vector<double> EX_pcp = {-1.9,  -1.3,  2.1,  -6.6,  -3.3,  8.4,  17.3,  18.2,  5.9,  11.1,  8.4,  8.3,  6.7,  5.5,  -0.6,  3.7,  -23.2,  -22.3,  -24.9,  -23.3, -21.6,  -19.7,  -16.0,  -14.4,  22.1,  15.1,  13.4,  4.8,  4.1,  2.6,  9.2,  13.5};
    std::vector<double> EY_pcp = {-14.3,  -5.0,  -5.0,  -27.7,  -10.6,  8.0,  26.2,  39.9,  6.5,  12.5,  16.1,  22.2,  20.9,  11.3,  7.6,  10.4,  -21.1,  -21.8,  -27.9,  -29.8, -19.7,  -19.7,  -14.3,  -9.1,  10.2,  8.7,  6.1,  11.0,  4.3,  -3.1,  2.6,  2.8};
    std::vector<double> GY_pcp = {-0.4,  3.1,  4.4,  2.3,  -4.3,  -2.4,  0.4,  1.1,  -0.9,  2.0,  1.4,  0.1,  1.0,  1.2,  -0.1,  -2.4,  -0.8,  -0.5,  -0.1,  0.4,  0.3,  0.5,  -0.8,  -0.8,  -4.0,  -6.3,  -3.6,  -3.5,  -4.4,  -4.9,  -1.4,  -1.2};

    double total_delta = 0;
    double count = 0;
    for(std::size_t i=0; i<EX_pcp.size(); i++)
    {
        count += 1;
        double delta = EX_pcp[i] - rem_pcal->at(0,i);
        total_delta += std::fabs(delta);
    }

    for(std::size_t i=0; i<EY_pcp.size(); i++)
    {
        count += 1;
        double delta = EY_pcp[i] - rem_pcal->at(1,i);
        total_delta += std::fabs(delta);
    }

    for(std::size_t i=0; i<GY_pcp.size(); i++)
    {
        count += 1;
        double delta = GY_pcp[i] - ref_pcal->at(1,i);
        total_delta += std::fabs(delta);
    }

    std::cout<<"Mean delta = "<<total_delta/count<<std::endl;

    if(total_delta/count > 5e-7)
    {
        std::cout<<"Difference in pc_phases exceeds tolerance of 1e-7."<<std::endl;
        return 1;
    }
    else
    {
        std::cout<<"Pass."<<std::endl;
    }

    return 0;
}
