#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>

#include "MHO_Message.hh"
#include "MHO_MK4VexInterface.hh"
#include "MHO_MK4CorelInterface.hh"
#include "MHO_MK4StationInterface.hh"

#include "MHO_Visibilities.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_VisibilityChannelizer.hh"
#include "MHO_WeightChannelizer.hh"
#include "MHO_StationCoordinates.hh"

#include "MHO_DirectoryInterface.hh"

#include "MHO_NormFX.hh"

extern "C"
{
    #include "mk4_data.h"
    #include "vex.h"
    #include "pass_struct.h"
    #include "param_struct.h"
    #include "write_lock_mechanism.h"

    /* External structure declarations */
    struct mk4_corel cdata;
    struct type_param param;
    struct type_status status;
    struct mk4_fringe fringe;
    struct type_plot plot;
    struct c_block *cb_head;

    int
    default_cblock (struct c_block *cb_ptr);

    int
    set_defaults();

    int
    organize_data (
    struct mk4_corel *cdata,
    struct scan_struct *ovex,
    struct ivex_struct *ivex,
    struct mk4_sdata *sdata,
    struct freq_corel *corel,
    struct type_param* param);

    void
    norm_fx (
    struct type_pass* pass,
    struct type_param* param,
    struct type_status* status,
    int fr,
    int ap);

    int
    make_passes (
    struct scan_struct *ovex,
    struct freq_corel *corel,
    struct type_param *param,
    struct type_pass **pass,
    int *npass);


    int baseline, base, ncorel_rec, lo_offset, max_seq_no;
    int do_only_new = FALSE;
    int test_mode = FALSE;
    int write_xpower = FALSE;
    int do_accounting = FALSE;
    int do_estimation = FALSE;
    int refringe = FALSE;
    int ap_per_seg = 0;
    int reftime_offset = 0;

    //global variables provided for signal handler clean up of lock files
    lockfile_data_struct global_lockfile_data;

    int msglev = -2;
    char progname[] = "test";

}


using namespace hops;


//read and fill-in the vex data as json and vex struct objects
bool GetVex(MHO_DirectoryInterface& dirInterface,
            MHO_MK4VexInterface& vexInterface,
            json& json_vex,
            struct vex*& root)
{
    //get list of all the files (and directories) in directory
    std::vector< std::string > allFiles;
    dirInterface.GetFileList(allFiles);
    std::string root_file;
    dirInterface.GetRootFile(allFiles, root_file);

    //convert root file ovex data to JSON, and export root/ovex ptr
    vexInterface.OpenVexFile(root_file);
    bool ovex_ok = vexInterface.ExportVexFileToJSON(json_vex);
    root = vexInterface.GetVex();
    return ovex_ok;
}
////////////////////////////////////////////////////////////////////////////////



// read a corel file and fill in old style and new style data containers
bool GetCorel(MHO_DirectoryInterface& dirInterface,
              MHO_MK4CorelInterface& corelInterface,
              const std::string& baseline,
              struct mk4_corel*& pcdata,
              ch_baseline_data_type*& ch_bl_data,
              ch_baseline_weight_type*& ch_bl_wdata
)
{
    bool corel_ok = false;
    // ch_baseline_data_type* ch_bl_data = nullptr;
    // ch_baseline_weight_type* ch_bl_wdata = nullptr;
    std::string root_file;
    std::vector< std::string > allFiles, corelFiles;
    dirInterface.GetFileList(allFiles);
    dirInterface.GetRootFile(allFiles, root_file);
    dirInterface.GetCorelFiles(allFiles, corelFiles);

    for(auto it = corelFiles.begin(); it != corelFiles.end(); it++)
    {
        std::string st_pair, root_code;
        std::string input_basename = dirInterface.GetBasename(*it);
        dirInterface.SplitCorelFileBasename(input_basename, st_pair, root_code);
        if(st_pair == baseline)
        {
            std::cout<<"found corel file: "<< *it <<std::endl;
            corelInterface.SetCorelFile(*it);
            corelInterface.SetVexFile(root_file);
            corelInterface.ExtractCorelFile();
            baseline_data_type* bl_data = corelInterface.GetExtractedVisibilities();
            baseline_weight_type* bl_wdata = corelInterface.GetExtractedWeights();

            MHO_VisibilityChannelizer channelizer;
            channelizer.SetInput(bl_data);
            ch_bl_data = new ch_baseline_data_type();
            channelizer.SetOutput(ch_bl_data);
            bool init = channelizer.Initialize();
            bool exe = false;
            if(init)
            {
                std::cout<<"initialization done"<<std::endl;
                exe = channelizer.ExecuteOperation();
                if(exe){std::cout<<"vis channelizer done"<<std::endl;}
            }

            MHO_WeightChannelizer wchannelizer;
            wchannelizer.SetInput(bl_wdata);
            ch_bl_wdata = new ch_baseline_weight_type();
            wchannelizer.SetOutput(ch_bl_wdata);
            bool winit = wchannelizer.Initialize();
            bool wexe = false;
            if(winit)
            {
                std::cout<<"initialization done"<<std::endl;
                wexe = wchannelizer.ExecuteOperation();
                if(wexe){std::cout<<"weight channelizer done"<<std::endl;}
            }

            //get the raw mk4 type data
            pcdata = corelInterface.GetCorelData();

            if(exe && wexe){corel_ok = true;}
            break;
        }
    }
    return corel_ok;
};
////////////////////////////////////////////////////////////////////////////////



//note: we normally wouldn't bother passing around two station interfaces, but
//we need to keep a pointer to the raw mk4 data, so for now keeping the interface
//classes around is the simplest way to do this with the least work
bool GetStationData(MHO_DirectoryInterface& dirInterface,
                    MHO_MK4StationInterface& refInterface,
                    MHO_MK4StationInterface& remInterface,
                    const std::string& baseline,
                    struct mk4_sdata*& ref_sdata,
                    struct mk4_sdata*& rem_sdata,
                    station_coord_data_type*& ref_stdata,
                    station_coord_data_type*& rem_stdata)
{
    std::string ref_st, rem_st, root_file;
    ref_st = baseline.at(0);
    rem_st = baseline.at(1);
    std::vector< std::string > allFiles, stationFiles;
    dirInterface.GetFileList(allFiles);
    dirInterface.GetStationFiles(allFiles, stationFiles);
    dirInterface.GetRootFile(allFiles, root_file);
    bool ref_ok = false;
    bool rem_ok = false;
    for(auto it = stationFiles.begin(); it != stationFiles.end(); it++)
    {

        std::string st, root_code;
        std::string input_basename = dirInterface.GetBasename(*it);
        dirInterface.SplitStationFileBasename(input_basename, st, root_code);

        if(st == ref_st)
        {
            std::cout<<"ref station file: "<< *it <<std::endl;
            refInterface.SetStationFile(*it);
            refInterface.SetVexFile(root_file);
            ref_stdata = refInterface.ExtractStationFile();
            ref_sdata = refInterface.GetStationData();
            ref_ok = true;
        }

        if(st == rem_st)
        {
            std::cout<<"rem station file: "<< *it <<std::endl;
            remInterface.SetStationFile(*it);
            remInterface.SetVexFile(root_file);
            rem_stdata = remInterface.ExtractStationFile();
            rem_sdata = remInterface.GetStationData();
            rem_ok = true;
        }

        if(ref_ok && rem_ok){break;}
    }

    return (ref_ok && rem_ok);

};

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    std::string usage = "TestNormFX -i <input_directory> -b <baseline>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string input_dir;
    std::string baseline;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"input_directory", required_argument, 0, 'i'},
                                          {"baseline", required_argument, 0, 'b'}};

    static const char* optString = "hi:b:";

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
            case ('i'):
                input_dir = std::string(optarg);
                break;
            case ('b'):
                baseline = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    //split the baseline into reference/remote station IDs
    if(baseline.size() != 2){msg_fatal("main", "Baseline: "<<baseline<<" is not of length 2."<<eom);}

    //directory interface, load up the directory information
    MHO_DirectoryInterface dirInterface;
    dirInterface.SetCurrentDirectory(input_dir);
    dirInterface.ReadCurrentDirectory();

    //get the root (ovex) file information
    MHO_MK4VexInterface vexInterface;
    json json_vex;
    struct vex* root = nullptr;
    bool ovex_ok = GetVex(dirInterface, vexInterface, json_vex, root);

    //the corel file information for this baseline
    MHO_MK4CorelInterface corelInterface;
    struct mk4_corel* pcdata = nullptr;
    ch_baseline_data_type* ch_bl_data = nullptr;
    ch_baseline_weight_type* ch_bl_wdata = nullptr;
    bool corel_ok = GetCorel(dirInterface, corelInterface, baseline, pcdata, ch_bl_data, ch_bl_wdata);
    std::cout<<"data ptrs = "<<pcdata<<", "<<ch_bl_data<<", "<<ch_bl_wdata<<std::endl;

    //get the station data information for the ref/rem stations of this baseline
    MHO_MK4StationInterface refInterface, remInterface;
    struct mk4_sdata* ref_sdata = nullptr;
    struct mk4_sdata* rem_sdata = nullptr;
    struct mk4_sdata* sdata = new mk4_sdata[MAXSTATIONS];
    station_coord_data_type* ref_stdata = nullptr;
    station_coord_data_type* rem_stdata = nullptr;
    bool sta_ok = GetStationData(dirInterface, refInterface, remInterface, baseline, ref_sdata, rem_sdata, ref_stdata, rem_stdata);
    sdata[0] = *ref_sdata;
    sdata[1] = *rem_sdata;

////////////////////////////////////////////////////////////////////////////////
//now we need to set up the param, pass, and other data org structs

    struct type_pass pass;
    //global params (defined above as extern for linking reasons)
    // struct type_param param;
    // struct c_block *cb_head;
    // struct type_status status;

    // struct mk4_fringe fringe; //not used
    // struct type_plot plot; //not used


    param.acc_period = root->evex->ap_length;
    param.speedup = root->evex->speedup_factor;
    param.pol = POLMASK_LL;// POL_ALL;
    param.first_plot = 0;
    param.nplot_chans = 0;
    param.fmatch_bw_pct = 25.0;
    param.pc_mode[0] = MANUAL;
    param.pc_mode[1] = MANUAL;

    //control block, default
    cb_head = &(pass.control);
    cb_head->fmatch_bw_pct = 25.0;
    default_cblock(cb_head);
    set_defaults();

    struct type_pass* pass_ptr = &pass;
    pass_ptr->npols = 1;
    struct freq_corel* corel = &(pass_ptr->pass_data[0]);
    pcdata->nalloc = 0;
    fringe.nalloc = 0;
    for (int i=0; i<MAXSTATIONS; i++){sdata[i].nalloc = 0;}
    for (int i=0; i<MAXFREQ; i++){ corel[i].data_alloc = FALSE;}

    int npass = 0;
    int retval = organize_data(pcdata, root->ovex, root->ivex, sdata, corel, &param);
    int passretval = make_passes (root->ovex, corel, &param, &pass_ptr, &npass);

    std::cout<<"st1 = "<<pcdata->t100->baseline[0]<<std::endl;
    std::cout<<"st2 = "<<pcdata->t100->baseline[1]<<std::endl;
    std::cout<<"date = "<< std::string(pcdata->id->date,16)<<std::endl;
    std::cout<<"npass = "<<npass<<std::endl;
    std::cout<<"nlags = "<<param.nlags<<std::endl;
    std::cout<<"pass.pol = "<<pass.pol<<std::endl;

    //allocate space for sbdelay
    struct data_corel *datum;
    hops_complex *sbarray, *sbptr;
    int size = 2 * param.nlags * pass_ptr->nfreq * pass_ptr->num_ap;
    sbarray = (hops_complex *)calloc (size, sizeof (hops_scomplex));
    if (sbarray == NULL)
    {
        std::cout<<"mem alloc failure"<<std::endl;
        return (-1);
    }
    sbptr = sbarray;
    for (int fr=0; fr<pass_ptr->nfreq; fr++)
    {
        for (int ap=0; ap<pass_ptr->num_ap; ap++)
        {
            datum = pass_ptr->pass_data[fr].data + ap + pass_ptr->ap_off;
            datum->sbdelay = sbptr;
            sbptr += 2*param.nlags;
            for(int i=0; i<4; i++)
            {
                //we don't want to do anything with p-cal right now,
                //so just set it all to 1.0
                //(otherwise they default to zero, and nothing gets done)
                datum->pc_phasor[i][0] = 1.0;
                datum->pc_phasor[i][1] = 0.0;
            }
        }
    }

    for (int fr=0; fr<pass_ptr->nfreq; fr++)
    {
        for (int ap=0; ap<pass_ptr->num_ap; ap++)
        {
            norm_fx(&pass, &param, &status, fr, ap);
        }
    }

    std::cout<<"param.nlags = "<<param.nlags<<std::endl;
    for (int fr=0; fr<pass_ptr->nfreq; fr++)
    {
        for (int ap=0; ap<pass_ptr->num_ap; ap++)
        {
            datum = pass_ptr->pass_data[fr].data + ap + pass_ptr->ap_off;
            for(int i=0; i < 2*param.nlags; i++)
            {
                std::cout<<"datum @ "<<i<<" = "<<datum->sbdelay[i]<<std::endl;
            }
        }
    }







    return 0;
}
