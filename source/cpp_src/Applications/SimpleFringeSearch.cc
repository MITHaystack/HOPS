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

//global messaging util
#include "MHO_Message.hh"

//handles reading directories, listing files etc.
#include "MHO_DirectoryInterface.hh"

//needed to read hops files and extract objects
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerFileInterface.hh"

//norm fx 
#include "MHO_NormFX.hh"



using namespace hops;


int main(int argc, char** argv)
{

    set_progname("SimpleFringeSearch");
    set_msglev(3);
    // set_msglev(-4);

    std::string usage = "SimpleFringeSearch -d <directory> -c <control file> -b <baseline> -p <pol. product>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string directory = "";
    std::string control_file = "";
    std::string baseline = "";
    std::string polprod = "";

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

    if( directory == "" || baseline == "" || polprod == "" || control_file == "") 
    {
        std::cout << usage << std::endl;
        return 1;
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

    std::ifstream ifs(root_file);
    mho_json vexInfo = json::parse(ifs);

    //locate the corel file that contains the baseline of interest (this is primitive)
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

    //read the entire file into memory (obviously we will want to optimize this in the future)
    MHO_ContainerStore conStore;
    MHO_ContainerFileInterface conInter;
    conInter.SetFilename(corel_file);
    conInter.PopulateStoreFromFile(conStore); //reads in all the objects in a file

    //retrieve the (first) visibility and weight objects (currently assuming there is only one object per type)
    ch_visibility_type* bl_data = nullptr;
    ch_weight_type* wt_data = nullptr;
    MHO_ObjectTags* tags = nullptr;

    bl_data = conStore.RetrieveObject<ch_visibility_type>();
    wt_data = conStore.RetrieveObject<ch_weight_type>();
    tags = conStore.RetrieveObject<MHO_ObjectTags>();

    std::cout<<bl_data<<" "<<wt_data<<std::endl;



    std::size_t wt_dim[ch_weight_type::rank::value];
    wt_data->GetDimensions(wt_dim);

    for(std::size_t i=0; i<ch_weight_type::rank::value; i++)
    {
        std::cout<<"weight size in dim: "<<i<<" = "<<wt_dim[i]<<std::endl;
    }

    //temporary testing...we want to pull out only the specified pol-product (e.g. XX)
    //so for now we crudely create a copy from a slice view 

    //first find the index which corresponds to the specified pol product 
    std::size_t pp_index = 0;
    auto* pp_axis = &(std::get<CH_POLPROD_AXIS>(*bl_data));
    for(std::size_t pi = 0; pi < pp_axis->GetSize(); pi++)
    {
        std::cout<<pi<<" = "<< (*pp_axis)[pi] << std::endl;
        if( (*pp_axis)[pi] == polprod )
        {
            pp_index = pi;
        }
    }

    // auto bl_slice = bl_data->SliceView(pp_index, ":", ":", ":");
    // ch_visibility_type* selected_bl_data = new ch_visibility_type();
    // selected_bl_data->Copy(bl_slice);

    std::size_t bl_dim[ch_visibility_type::rank::value];
    bl_data->GetDimensions(bl_dim);

    for(std::size_t i=0; i<ch_visibility_type::rank::value; i++)
    {
        std::cout<<"vis size in dim: "<<i<<" = "<<bl_dim[i]<<std::endl;
    }


    //output for the delay 
    ch_visibility_type* sbd_data = bl_data->CloneEmpty();
    bl_dim[CH_FREQ_AXIS] *= 4; //normfx implementation demands this
    sbd_data->Resize(bl_dim);

    //re-run this exercise via the pure c++ function
    MHO_NormFX nfxOp;
    nfxOp.SetArgs(bl_data, wt_data, sbd_data);
    nfxOp.Initialize();
    nfxOp.Execute();

    return 0;
}
