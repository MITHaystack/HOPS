#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <getopt.h>
#include <iomanip>


//fourfit control lib
#ifndef HOPS3_USE_CXX
extern "C"
{
#endif

#include "msg.h"
#include "ffcontrol.h"
struct c_block* cb_head; //global extern kludge (due to stupid c-library interface)

#ifndef HOPS3_USE_CXX
}
#endif

#define EXTRA_DEBUG

//global messaging util
#include "MHO_Message.hh"

//snapshot utility lib
#include "MHO_Snapshot.hh"

//needed to read hops files and extract objects from scan dir
#include "MHO_ScanDataStore.hh"

//control
#include "MHO_ControlBlockWrapper.hh"

//operators
#include "MHO_ElementTypeCaster.hh"
#include "MHO_NormFX.hh"
#include "MHO_SelectRepack.hh"
#include "MHO_FreqSpacing.hh"
#include "MHO_UniformGridPointsCalculator.hh"
#include "MHO_FringeRotation.hh"
#include "MHO_Reducer.hh"

#include "MHO_AbsoluteValue.hh"
#include "MHO_FunctorBroadcaster.hh"
#include "MHO_ExtremaSearch.hh"
#include "MHO_ManualChannelPhaseCorrection.hh"
#include "MHO_DelayRate.hh"
#include "MHO_MBDelaySearch.hh"
#include "MHO_InterpolateFringePeak.hh"

#include "MHO_ComputePlotData.hh"

#ifdef USE_ROOT
    #include "TApplication.h"
    #include "MHO_RootCanvasManager.hh"
    #include "MHO_RootGraphManager.hh"
#endif


using namespace hops;


int main(int argc, char** argv)
{

    set_progname("SimpleFringeSearch");
    set_msglev(-1);
    // set_msglev(-4);

    std::string usage = "SimpleFringeSearch -d <directory> -c <control file> -b <baseline> -p <pol. product>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    MHO_Snapshot::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().SetExecutableName(std::string("SimpleFringeSearch"));

    std::string directory = "";
    std::string control_file = "";
    std::string baseline = "";
    std::string polprod = "";
    bool ok;

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

    ////////////////////////////////////////////////////////////////////////////
    //INITIAL SCAN DIRECTORY
    ////////////////////////////////////////////////////////////////////////////


    //initialize the scan store from this directory
    MHO_ScanDataStore scanStore;
    scanStore.SetDirectory(directory);
    scanStore.Initialize();
    if( !scanStore.IsValid() )
    {
        msg_fatal("main", "cannot initialize a valid scan store from this directory: " << directory << eom);
        std::exit(1);
    }

    //load root file and container store for this baseline
    mho_json vexInfo = scanStore.GetRootFileData();
    MHO_ContainerStore* conStore = scanStore.LoadBaseline(baseline);

    if(conStore == nullptr)
    {
        msg_fatal("main", "Could not find a file for baseline: "<< baseline << eom);
        std::exit(1);
    }


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

    double ref_freq = cb_wrapper.GetReferenceFrequency();//grab the reference frequency

    ////////////////////////////////////////////////////////////////////////////
    //LOAD DATA
    ////////////////////////////////////////////////////////////////////////////

    //retrieve the (first) visibility and weight objects (currently assuming there is only one object per type)
    visibility_store_type* bl_store_data = nullptr;
    weight_store_type* wt_store_data = nullptr;

    visibility_type bl_data_obj;
    weight_type wt_data_obj;
    visibility_type* bl_data = &bl_data_obj;
    weight_type* wt_data = &wt_data_obj;

    MHO_ObjectTags* tags = nullptr;

    bl_store_data = conStore->RetrieveObject<visibility_store_type>();
    wt_store_data = conStore->RetrieveObject<weight_store_type>();
    tags = conStore->RetrieveObject<MHO_ObjectTags>();

    if(bl_store_data == nullptr)
    {
        msg_fatal("main", "failed to read visibility data from the .cor file." <<eom);
        std::exit(1);
    }

    if(wt_store_data == nullptr)
    {
        msg_fatal("main", "failed to read weight data from the .cor file." <<eom);
        std::exit(1);
    }

    if(tags == nullptr)
    {
        msg_warn("main", "failed to read tag data from the .cor file." <<eom);
    }

    MHO_ElementTypeCaster<visibility_store_type, visibility_type> up_caster;
    up_caster.SetArgs(bl_store_data, bl_data);
    up_caster.Initialize();
    up_caster.Execute();

    MHO_ElementTypeCaster< weight_store_type, weight_type> wt_up_caster;
    wt_up_caster.SetArgs(wt_store_data, wt_data);
    wt_up_caster.Initialize();
    wt_up_caster.Execute();

    std::size_t wt_dim[weight_type::rank::value];
    wt_data->GetDimensions(wt_dim);

    ////////////////////////////////////////////////////////////////////////////
    //APPLY COARSE DATA SELECTION
    ////////////////////////////////////////////////////////////////////////////
    //select data repack
    MHO_SelectRepack<visibility_type> spack;
    MHO_SelectRepack<weight_type> wtspack;

    //first find indexes which corresponds to the specified pol product
    std::vector<std::size_t> selected_pp = (&(std::get<POLPROD_AXIS>(*bl_data)))->SelectMatchingIndexes(polprod);

    //select some specified AP's
    // std::vector< std::size_t > selected_ap;
    // selected_ap.push_back(20);

    //select first 8 channels for testing
    std::vector< std::size_t > selected_ch;
    for(std::size_t i=0;i<8; i++){selected_ch.push_back(i);}
    //for(std::size_t i=0;i<2; i++){selected_ch.push_back(i);}

    //specify the indexes we want on each axis
    spack.SelectAxisItems(0,selected_pp);
    spack.SelectAxisItems(1,selected_ch);

    wtspack.SelectAxisItems(0,selected_pp);
    wtspack.SelectAxisItems(1,selected_ch);
    //spack.SelectAxisItems(2,selected_ap);


    visibility_type* alt_data = new visibility_type();
    weight_type* alt_wt_data = new weight_type();

    spack.SetArgs(bl_data, alt_data);
    spack.Initialize();
    spack.Execute();

    wtspack.SetArgs(wt_data, alt_wt_data);
    wtspack.Initialize();
    wtspack.Execute();

    //TODO, work out what to do with the axis interval labels in between operations
    //explicitly copy the channel axis labels here
    std::get<CHANNEL_AXIS>(*alt_data).CopyIntervalLabels( std::get<CHANNEL_AXIS>(*bl_data) );
    std::get<CHANNEL_AXIS>(*alt_wt_data).CopyIntervalLabels( std::get<CHANNEL_AXIS>(*wt_data) );

    wt_data->Copy(*alt_wt_data);
    bl_data->Copy(*alt_data);

    delete alt_data;
    delete alt_wt_data;

    std::size_t bl_dim[visibility_type::rank::value];
    bl_data->GetDimensions(bl_dim);

    //take a snapshot
    take_snapshot_here("test", "visib", __FILE__, __LINE__, bl_data);
    take_snapshot_here("test", "weights", __FILE__, __LINE__,  wt_data);

    // //compute the sum of the weights
    weight_type temp_weights;
    temp_weights.Copy(*wt_data);
    MHO_Reducer<weight_type, MHO_CompoundSum> wt_reducer;
    wt_reducer.SetArgs(&temp_weights);
    for(std::size_t i=0; i<weight_type::rank::value; i++)
    {
        wt_reducer.ReduceAxis(i);
    }
    wt_reducer.Initialize();
    wt_reducer.Execute();

    double total_ap_frac = temp_weights[0];
    std::cout<<"reduced weights = "<<temp_weights[0]<<std::endl;

    wt_data->Insert("total_summed_weights", total_ap_frac);

    //change weights uuid  to prevent collision with previous snapshot
    // MHO_UUIDGenerator gen;
    // MHO_UUID new_uuid = gen.GenerateUUID(); //random object id
    // temp_weights->SetObjectUUID(new_uuid);
    take_snapshot_here("test", "reduced_weights", __FILE__, __LINE__,  &temp_weights);

    ////////////////////////////////////////////////////////////////////////////
    //APPLY DATA CORRECTIONS (A PRIORI -- PCAL)
    ////////////////////////////////////////////////////////////////////////////

    //apply manual pcal
    //construct the pcal array...need to re-think how we are going to move control block info around (scalar parameters vs. arrays etc)
    manual_pcal_type* ref_pcal = cb_wrapper.GetRefStationManualPCOffsets();
    manual_pcal_type* rem_pcal = cb_wrapper.GetRemStationManualPCOffsets();

    MHO_ManualChannelPhaseCorrection pcal_correct;

    pcal_correct.SetArgs(bl_data, rem_pcal, bl_data);
    ok = pcal_correct.Initialize();
    check_step_error(ok, "main", "ref pcal initialization." << eom );
    ok = pcal_correct.Execute();
    check_step_error(ok, "main", "ref pcal execution." << eom );

    pcal_correct.SetArgs(bl_data, ref_pcal, bl_data);
    ok = pcal_correct.Initialize();
    check_step_error(ok, "main", "rem pcal initialization." << eom );
    ok = pcal_correct.Execute();
    check_step_error(ok, "main", "rem pcal execution." << eom );


    //output for the delay
    visibility_type* sbd_data = bl_data->CloneEmpty();
    bl_dim[FREQ_AXIS] *= 4; //normfx implementation demands this
    sbd_data->Resize(bl_dim);

    ////////////////////////////////////////////////////////////////////////////
    //COARSE SBD, DR, MBD SEARCH ALGO
    ////////////////////////////////////////////////////////////////////////////

    //run norm-fx via the wrapper class (x-form to SBD space)
    MHO_NormFX nfxOp;
    nfxOp.SetArgs(bl_data, wt_data, sbd_data);
    ok = nfxOp.Initialize();
    check_step_fatal(ok, "main", "normfx initialization." << eom );

    ok = nfxOp.Execute();
    check_step_fatal(ok, "main", "normfx execution." << eom );

    //take snapshot of sbd data after normfx
    take_snapshot_here("test", "sbd", __FILE__, __LINE__, sbd_data);

    //run the transformation to delay rate space (this also involves a zero padded FFT)
    MHO_DelayRate drOp;
    visibility_type* sbd_dr_data = sbd_data->CloneEmpty();
    drOp.SetReferenceFrequency(ref_freq);
    drOp.SetArgs(sbd_data, wt_data, sbd_dr_data);
    ok = drOp.Initialize();
    check_step_fatal(ok, "main", "dr initialization." << eom );
    ok = drOp.Execute();
    check_step_fatal(ok, "main", "dr execution." << eom );

    take_snapshot_here("test", "sbd_dr", __FILE__, __LINE__, sbd_dr_data);

    //coarse SBD/MBD/DR search (locates max bin)
    MHO_MBDelaySearch mbdSearch;
    mbdSearch.SetArgs(sbd_dr_data);
    ok = mbdSearch.Initialize();
    check_step_fatal(ok, "main", "mbd initialization." << eom );
    ok = mbdSearch.Execute();
    check_step_fatal(ok, "main", "mbd execution." << eom );

    int c_mbdmax = mbdSearch.GetMBDMaxBin();
    int c_sbdmax = mbdSearch.GetSBDMaxBin();
    int c_drmax = mbdSearch.GetDRMaxBin();

    std::cout<<"SBD/MBD/DR max bins = "<<c_sbdmax<<", "<<c_mbdmax<<", "<<c_drmax<<std::endl;


    ////////////////////////////////////////////////////////////////////////////
    //FINE INTERPOLATION STEP (search over 5x5x5 grid around peak)
    ////////////////////////////////////////////////////////////////////////////
    MHO_InterpolateFringePeak fringeInterp;
    fringeInterp.SetReferenceFrequency(ref_freq);
    fringeInterp.SetMaxBins(c_sbdmax, c_mbdmax, c_drmax);

    fringeInterp.SetSBDArray(sbd_data);
    fringeInterp.SetWeights(wt_data);

    //TODO fix me -- we shouldn't be referencing internal members of the MHO_MBDelaySearch class workspace
    //Figure out how best to present this axis data to the fine-interp function.
    fringeInterp.SetMBDAxis( mbdSearch.GetMBDAxis());
    fringeInterp.SetDRAxis( mbdSearch.GetDRAxis());

    fringeInterp.Initialize();
    fringeInterp.Execute();

    //todo ought to make this a more uniform/cleaner interface (probably using the common label map store)
    double sbdelay = fringeInterp.GetSBDelay();
    double mbdelay = fringeInterp.GetMBDelay();
    double drate = fringeInterp.GetDelayRate();

    ////////////////////////////////////////////////////////////////////////////
    //PLOTTING/DEBUG
    ////////////////////////////////////////////////////////////////////////////

    MHO_ComputePlotData mk_plotdata;

    mk_plotdata.SetReferenceFrequency(ref_freq);
    mk_plotdata.SetMBDelay(mbdelay);
    mk_plotdata.SetDelayRate(drate);
    mk_plotdata.SetSBDelay(sbdelay);
    mk_plotdata.SetSBDArray(sbd_data);
    mk_plotdata.SetWeights(wt_data);

    auto sbd_amp = mk_plotdata.calc_sbd();




    //#ifdef USE_ROOT
    #ifdef NOT_DISABLED

    std::cout<<"starting root plotting"<<std::endl;

    //ROOT stuff for plots

    int dummy_argc = 0;
    char tmp = '\0';
    char* argv_placeholder = &tmp;
    char** dummy_argv = &argv_placeholder;

    TApplication* App = new TApplication("test",&dummy_argc,dummy_argv);

    MHO_RootCanvasManager cMan;
    MHO_RootGraphManager gMan;

    MHO_ExtremaSearch< MHO_NDArrayView< visibility_element_type, 2 > > mSearch;
    MHO_ExtremaSearch< MHO_NDArrayView< visibility_element_type, 1 > > mbdSearch;

    auto dr_rate_ax = std::get<TIME_AXIS>(*sbd_dr_data);
    auto delay_ax = std::get<FREQ_AXIS>(*sbd_dr_data);

    //divide out the reference frequency (need to think about this )
    double rescale = 1.0/6.0;//ref freq = 6GHz (axis is then in nanosec/sec)
    for(std::size_t d=0; d<dr_rate_ax.GetSize(); d++)
    {
        dr_rate_ax(d) *= rescale;
    }




    for(std::size_t ch=0; ch<bl_dim[CHANNEL_AXIS]; ch++)
    {
        std::stringstream ss;
        ss << "channel_test";
        ss << ch;

        auto c = cMan.CreateCanvas(ss.str().c_str(), 800, 800);
        auto ch_slice = sbd_dr_data->SliceView(0,ch,":",":");
        mSearch.SetArgs(&ch_slice);
        mSearch.Initialize();
        mSearch.Execute();

        //really dumb/simple way of looking at the location of the delay/dr_rate max location
        double vmax = mSearch.GetMax();
        double vmin = mSearch.GetMin();
        std::size_t max_loc = mSearch.GetMaxLocation();
        std::size_t min_loc = mSearch.GetMinLocation();
        auto loc_array = ch_slice.GetIndicesForOffset(max_loc);
        std::cout<<ss.str()<<": max = "<<vmax<<" at index location: ("<<loc_array[0]<<", "<<loc_array[1] <<")  = ("
        <<dr_rate_ax(loc_array[0])<<", "<<delay_ax(loc_array[1])<<") " <<std::endl;
        visibility_element_type val = ch_slice(loc_array[0], loc_array[1]);
        std::cout<<"max mag = "<<std::abs(val)<<", arg = "<<std::arg(val)*(180./M_PI)<<std::endl;

        auto gr = gMan.GenerateComplexGraph2D(ch_slice, dr_rate_ax, delay_ax, ROOT_CMPLX_PLOT_ABS );

        //at the sbd, dr max locations, lets look for the mbd max too
        auto mbd_slice = mbd_data.SliceView(0, ":", loc_array[0], loc_array[1]);
        auto mbd_ax = &(std::get<CHANNEL_AXIS>(mbd_data));

        if(ch==0)
        {
            //TODO FIXME
            //for some reason our MBD plot axis is flipped w.r.t to fourfit plot
            //is this a sign convention? due to LSB vs USB? Don't know right now
            double fudge_factor = -1; //sign flip
            for(std::size_t d=0; d<mbd_ax->GetSize(); d++)
            {
                (*mbd_ax)(d) *= fudge_factor;
            }
        }


        // for(std::size_t x=0;x<mbd_slice.GetSize(); x++)
        // {
        //     std::cout<<"x, val = "<<x<<", "<<(*mbd_ax)(x)<<", "<<mbd_slice(x)<<std::endl;
        // }

        mbdSearch.SetArgs(&mbd_slice);
        mbdSearch.Initialize();
        mbdSearch.Execute();

        auto new_mbd_ax = std::get<CHANNEL_AXIS>(mbd_data);
        auto gr2 = gMan.GenerateComplexGraph1D(mbd_slice, *mbd_ax, ROOT_CMPLX_PLOT_ABS );

        std::size_t max_mbd_loc = mbdSearch.GetMaxLocation();
        auto mbd_loc_array = mbd_slice.GetIndicesForOffset(max_mbd_loc);
        std::cout<<"mbd max located at: "<<mbd_loc_array[0]<<" = "<<(*mbd_ax)(mbd_loc_array[0])<<std::endl;



        c->cd();
        c->Divide(1,2);

        c->cd(1);
        c->SetTopMargin(0.1);
        c->SetRightMargin(0.2);

        gr->SetTitle( "Fringe; delay rate (ns/s); Single band delay (#mus); Amp");
        gr->Draw("COLZ");
        gr->GetHistogram()->GetXaxis()->CenterTitle();
        gr->GetHistogram()->GetYaxis()->CenterTitle();
        gr->GetHistogram()->GetZaxis()->CenterTitle();
        gr->GetHistogram()->GetXaxis()->SetTitleOffset(1.2);
        gr->GetHistogram()->GetYaxis()->SetTitleOffset(1.08);
        gr->GetHistogram()->GetZaxis()->SetTitleOffset(-0.4);
        gr->Draw("COLZ");



        c->Update();
        c->cd(2);
        c->SetTopMargin(0.1);
        c->SetRightMargin(0.2);

        gr2->SetTitle( "Fringe; MBD; Amp");
        gr2->Draw("APL");


        c->Update();
    }
    App->Run();


    #endif



    return 0;
}
