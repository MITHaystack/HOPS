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
    std::string usage = "TestControlFileParsePCPhases -c <control file> -b <baseline>";

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
        } \
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
    
    std::cout<< *ref_pcal << std::endl;

    return 0;
}
