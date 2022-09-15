#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <utility>
#include <map>
#include <getopt.h>


#include "ffcontrol.h"
#include "msg.h"
struct c_block* cb_head; //global extern kludge

#include "MHO_Message.hh"
#include "MHO_VexParser.hh"

#include "MHO_DirectoryInterface.hh"
#include "MHO_BinaryFileInterface.hh"

// #include "MHO_Reducer.hh"
// #include "MHO_FunctorBroadcaster.hh"
// #include "MHO_MultidimensionalFastFourierTransform.hh"

#include "MHO_ContainerDefinitions.hh"
//#include "MHO_ChannelizedRotationFunctor.hh"




using namespace hops;


int main(int argc, char** argv)
{

    set_progname("blah");
    set_msglev(-4);

    std::string usage = "SimpleFringeSearch -d <directory> -c <control file> -b <baseline> -p <pol. product>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string directory;
    std::string control_file;
    std::string baseline;
    std::string polprod;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"directory", required_argument, 0, 'd'},
                                          {"control", required_argument, 0, 'c'},
                                          {"baseline", required_argument, 0, 'b'},
                                          {"polarization-product", required_argument, 0, 'p'}};

    static const char* optString = "hd:c:b:p:";

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
            case ('c'):
                control_file = std::string(optarg);
                break;
            case ('b'):
                baseline = std::string(optarg);
                break;
            case ('p'):
                polprod = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    //parse the control file
    cb_head = (struct c_block *) malloc (sizeof (struct c_block) );
    struct c_block* cb_out = (struct c_block *) malloc (sizeof (struct c_block) );
    nullify_cblock (cb_head);     
    default_cblock( cb_head );
    nullify_cblock (cb_out);
    default_cblock(cb_out);
    char bl[2]; bl[0] = baseline[0]; bl[1] = baseline[1];
    std::string src = " ";
    char fgroup = 'X';
    int time = 0;

    std::cout<<control_file<<std::endl;
    std::cout<<bl[0]<<bl[1]<<" "<<fgroup<<std::endl;

    int retval = construct_cblock(const_cast<char*>(control_file.c_str()), cb_head, cb_out, bl, const_cast<char*>(src.c_str()), fgroup, time);
    std::cout<<"c block retval = "<<retval<<std::endl;

    //print pc_phases 

    // struct dstats pc_phase_offset[2];// manual phase offset applied to all channels, by pol 
    // struct dstats pc_phase[MAXFREQ][2];/* phase cal phases by channel and pol 
    //                                           for manual or additive pcal */

    for(unsigned int p=0; p<2; p++)
    {
        for(std::size_t ch=0; ch<MAXFREQ; ch++)
        {
            std::cout<<"chan: "<< ch <<" ref-pc: "<< cb_out->pc_phase[ch][p].ref << " rem-pc: " << cb_out->pc_phase[ch][p].rem << std::endl;
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

    dirInterface.GetFileList(allFiles);
    dirInterface.GetFilesMatchingExtention(corFiles, "cor");
    dirInterface.GetFilesMatchingExtention(staFiles, "sta");
    dirInterface.GetFilesMatchingExtention(jsonFiles, "json");

    //check that there is only one json file
    std::string root_file = "";
    if(jsonFiles.size() != 1)
    {
        msg_fatal("main", "There are "<<jsonFiles.size()<<" root files." << eom);
        std::exit(1);
    }
    else
    {
        root_file = jsonFiles[0];
    }

    //open the root (json version of the vex) file
    // MHO_VexParser vparser;
    // vparser.SetVexFile(root_file);
    // mho_json vexInfo = vparser.ParseVex();

    std::ifstream ifs(root_file);
    mho_json vexInfo = json::parse(ifs);

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

    //now open and read the (channelized) baseline visibility data
    ch_visibility_type* bl_data = new ch_visibility_type();
    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToRead(corel_file);
    if(status)
    {
        MHO_FileKey key;
        inter.Read(*bl_data, key);
    }
    else
    {
        msg_fatal("main", "Could not open file for visibility data." << eom);
        inter.Close();
        std::exit(1);
    }
    inter.Close();

    std::size_t bl_dim[CH_VIS_NDIM];
    bl_data->GetDimensions(bl_dim);




    return 0;
}
