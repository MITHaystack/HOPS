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

//operators
#include "MHO_NormFX.hh"
#include "MHO_SelectRepack.hh"

#include "MHO_Reducer.hh"

#include "MHO_AbsoluteValue.hh"
#include "MHO_FunctorBroadcaster.hh"


#ifdef USE_ROOT
    #include "TApplication.h"
    #include "MHO_RootCanvasManager.hh"
    #include "MHO_RootGraphManager.hh"
#endif



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
    std::vector<std::size_t> selected_pp;
    auto* pp_axis = &(std::get<CH_POLPROD_AXIS>(*bl_data));
    for(std::size_t pi = 0; pi < pp_axis->GetSize(); pi++)
    {
        std::cout<<pi<<" = "<< (*pp_axis)[pi] << std::endl;
        if( (*pp_axis)[pi] == polprod )
        {
            pp_index = pi;
            selected_pp.push_back(pi);
        }
    }

    //select data only from this polprod, and repack
    MHO_SelectRepack<ch_visibility_type> spack;
    ch_visibility_type* alt_data = new ch_visibility_type();
    spack.SelectAxisItems(0,selected_pp);

    // std::vector< std::size_t > selected_ap;
    // selected_ap.push_back(20);
    // 
    // std::vector< std::size_t > selected_ch;
    // selected_ch.push_back(0);
    // 
    // //pick out just the first channel and ap
    // spack.SelectAxisItems(1,selected_ch);
    // spack.SelectAxisItems(2,selected_ap); 

    spack.SetArgs(bl_data, alt_data);
    spack.Initialize();
    spack.Execute();

    //TODO, work out what to do with the axis interval labels in between operations 
    //explicitly copy the channel axis labels here
    std::get<CH_CHANNEL_AXIS>(*alt_data).CopyIntervalLabels( std::get<CH_CHANNEL_AXIS>(*bl_data) );
    

    //DEBUG dump this to json
    MHO_ContainerStore conStore2;
    MHO_UUIDGenerator gen;
    MHO_ContainerDictionary conDict;
    MHO_UUID type_uuid = conDict.GetUUIDFor<ch_visibility_type>();
    MHO_UUID object_uuid = gen.GenerateUUID();
    conStore2.AddContainerObject(alt_data, type_uuid, object_uuid, "blah", 0);
    MHO_ContainerFileInterface conInter2;
    conInter2.SetFilename("doh.json");

    //convert the entire store to json 
    json root;    
    int detail = eJSONAll;
    conInter2.ConvertStoreToJSON(conStore2,root,detail);

    //open and dump to file 
    std::ofstream outFile("./test.json", std::ofstream::out);
    outFile << root;
    outFile.close();

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

    //xform in the time (AP) axis
    MHO_MultidimensionalFastFourierTransform< ch_visibility_type > fFFTEngine;
    MHO_CyclicRotator<ch_visibility_type> fCyclicRotator;


    bool status;
    fFFTEngine.SetArgs(sbd_data);
    fFFTEngine.DeselectAllAxes();
    fFFTEngine.SelectAxis(CH_TIME_AXIS);
    fFFTEngine.SetForward();
    status = fFFTEngine.Initialize();

    fCyclicRotator.SetOffset(CH_TIME_AXIS, bl_dim[CH_TIME_AXIS]/2);
    fCyclicRotator.SetArgs(sbd_data);
    status = fCyclicRotator.Initialize();

    fFFTEngine.Execute();
    fCyclicRotator.Execute();

    // MHO_Reducer<ch_visibility_type, MHO_CompoundSum>* reducer = new MHO_Reducer<ch_visibility_type, MHO_CompoundSum>();
    // reducer->SetArgs(sbd_data);
    // reducer->ReduceAxis(1);
    // //reducer->ReduceAxis(2);
    // //reducer->ReduceAxis(NDIM-1);
    // bool init = reducer->Initialize();
    // bool exe = reducer->Execute();


    // typedef MHO_AbsoluteValue<ch_visibility_type> absType;
    // MHO_FunctorBroadcaster<ch_visibility_type, absType > abs_broadcast;
    // abs_broadcast.SetArgs(sbd_data);
    // abs_broadcast.Initialize();
    // abs_broadcast.Execute();


    #ifdef USE_ROOT

    std::cout<<"starting root plotting"<<std::endl;

    //ROOT stuff for plots

    // int dummy_argc = 0;
    // char tmp = '\0';
    // char* argv_placeholder = &tmp;
    // char** dummy_argv = &argv_placeholder;
    // 
    // TApplication* App = new TApplication("test",&dummy_argc,dummy_argv);
    // 
    // MHO_RootCanvasManager cMan;
    // MHO_RootGraphManager gMan;
    // 
    // for(std::size_t ch=0; ch<32; ch++)
    // {
    //     std::stringstream ss;
    //     ss << "channel_test";
    //     ss << ch;
    // 
    //     auto c = cMan.CreateCanvas(ss.str().c_str(), 800, 800);
    //     auto ch_slice = sbd_data->SliceView(0,ch,0,":");
    //     auto gr = gMan.GenerateComplexGraph1D(ch_slice, std::get<CH_FREQ_AXIS>(*sbd_data), 4 );
    // 
    //     c->cd(1);
    //     gr->Draw("APL");
    //     c->Update();
    // }
    // App->Run();


    std::cout<<"starting root plotting"<<std::endl;

    //ROOT stuff for plots

    int dummy_argc = 0;
    char tmp = '\0';
    char* argv_placeholder = &tmp;
    char** dummy_argv = &argv_placeholder;
    
    TApplication* App = new TApplication("test",&dummy_argc,dummy_argv);

    MHO_RootCanvasManager cMan;
    MHO_RootGraphManager gMan;

    for(std::size_t ch=0; ch<32; ch++)
    {
        std::stringstream ss;
        ss << "channel_test";
        ss << ch;

        auto c = cMan.CreateCanvas(ss.str().c_str(), 800, 800);
        auto ch_slice = sbd_data->SliceView(0,ch,":",":");
        auto gr = gMan.GenerateComplexGraph2D(ch_slice, std::get<CH_TIME_AXIS>(*sbd_data),  std::get<CH_FREQ_AXIS>(*sbd_data), 4 );

        c->cd(1);
        gr->Draw("SURF1");
        c->Update();
    }
    App->Run();


    #endif



    return 0;
}
