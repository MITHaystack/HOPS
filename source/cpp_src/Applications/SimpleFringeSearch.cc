#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <getopt.h>

#include "msg.h"
#include "ffcontrol.h"
struct c_block* cb_head; //global extern kludge (due to stupid c-library interface)

//global messaging util
#include "MHO_Message.hh"

//needed to read hops files and extract objects from scan dir
#include "MHO_ScanDataStore.hh"

//control 
#include "MHO_ControlBlockWrapper.hh"

//operators
#include "MHO_NormFX.hh"
#include "MHO_SelectRepack.hh"
#include "MHO_FreqSpacing.hh"
#include "MHO_UniformGridPointsCalculator.hh"

#include "MHO_Reducer.hh"

#include "MHO_AbsoluteValue.hh"
#include "MHO_FunctorBroadcaster.hh"
#include "MHO_ExtremaSearch.hh"
#include "MHO_ManualChannelPhaseCorrection.hh"

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
    manual_pcal_type* ref_pcal = cb_wrapper.GetRefStationManualPCOffsets(); //ref_pcal.Resize(2,MAXFREQ);
    manual_pcal_type* rem_pcal = cb_wrapper.GetRemStationManualPCOffsets();// rem_pcal.Resize(2,MAXFREQ);

    //retrieve the (first) visibility and weight objects (currently assuming there is only one object per type)
    ch_visibility_type* bl_data = nullptr;
    ch_weight_type* wt_data = nullptr;
    MHO_ObjectTags* tags = nullptr;

    bl_data = conStore->RetrieveObject<ch_visibility_type>();
    wt_data = conStore->RetrieveObject<ch_weight_type>();
    tags = conStore->RetrieveObject<MHO_ObjectTags>();

    std::size_t wt_dim[ch_weight_type::rank::value];
    wt_data->GetDimensions(wt_dim);

    for(std::size_t i=0; i<ch_weight_type::rank::value; i++)
    {
        std::cout<<"weight size in dim: "<<i<<" = "<<wt_dim[i]<<std::endl;
    }

    //first find the index which corresponds to the specified pol product
    std::set< std::string > pp_values; pp_values.insert(polprod);
    auto* pp_axis = &(std::get<CH_POLPROD_AXIS>(*bl_data));
    std::vector<std::size_t> selected_pp = pp_axis->SelectMatchingIndexes(pp_values);

    //select data only from this polprod, and repack
    MHO_SelectRepack<ch_visibility_type> spack;
    ch_visibility_type* alt_data = new ch_visibility_type();
    spack.SelectAxisItems(0,selected_pp);

    // std::vector< std::size_t > selected_ap;
    // selected_ap.push_back(20);

    std::vector< std::size_t > selected_ch;
    for(std::size_t i=16;i<32; i++){selected_ch.push_back(i);}

    //pick out just the first channel and ap
    spack.SelectAxisItems(1,selected_ch);
    //spack.SelectAxisItems(2,selected_ap);

    spack.SetArgs(bl_data, alt_data);
    spack.Initialize();
    spack.Execute();

    //TODO, work out what to do with the axis interval labels in between operations
    //explicitly copy the channel axis labels here
    std::get<CH_CHANNEL_AXIS>(*alt_data).CopyIntervalLabels( std::get<CH_CHANNEL_AXIS>(*bl_data) );

    bl_data->Copy(*alt_data);

    std::size_t bl_dim[ch_visibility_type::rank::value];
    bl_data->GetDimensions(bl_dim);

    for(std::size_t i=0; i<ch_visibility_type::rank::value; i++)
    {
        std::cout<<"vis size in dim: "<<i<<" = "<<bl_dim[i]<<std::endl;
    }

    //apply manual pcal
    bool ok;
    MHO_ManualChannelPhaseCorrection pcal_correct;
    pcal_correct.SetArgs(bl_data, rem_pcal, bl_data);
    ok = pcal_correct.Initialize(); if(!ok){std::cout<<"flag1"<<std::endl;}
    ok = pcal_correct.Execute(); if(!ok){std::cout<<"flag2"<<std::endl;}

    pcal_correct.SetArgs(bl_data, ref_pcal, bl_data);
    ok = pcal_correct.Initialize(); if(!ok){std::cout<<"flag3"<<std::endl;}
    ok = pcal_correct.Execute(); if(!ok){std::cout<<"flag4"<<std::endl;}

    //output for the delay
    ch_visibility_type* sbd_data = bl_data->CloneEmpty();
    bl_dim[CH_FREQ_AXIS] *= 4; //normfx implementation demands this
    sbd_data->Resize(bl_dim);

    // //calculate frequency space for MBD
    // FreqSpacing(std::get<CH_CHANNEL_AXIS>(*bl_data));


    //re-run this exercise via the pure c++ function
    MHO_NormFX nfxOp;
    nfxOp.SetArgs(bl_data, wt_data, sbd_data);
    nfxOp.Initialize();
    nfxOp.Execute();

    //xform in the time (AP) axis to look for delay rate
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


    //collect the sky frequency values of each channel
    std::vector< double > chan_freqs;
    std::string sky_freq_key = "sky_freq";
    auto chan_ax = &(std::get<CH_CHANNEL_AXIS>(*bl_data) );
    chan_ax->CollectAxisElementLabelValues(sky_freq_key, chan_freqs);
    if(chan_freqs.size() != chan_ax->GetSize() ){msg_error("main", "channel axis and and sky frequency size mismatch" << eom);}

    //calculate the frequncy grid for MBD search
    MHO_UniformGridPointsCalculator gridCalc;
    gridCalc.SetPoints(chan_freqs);
    gridCalc.Calculate();

    std::cout<<"grid info: "<<gridCalc.GetGridStart()<<", "<<gridCalc.GetGridSpacing()<<", "<<gridCalc.GetNGridPoints()<<std::endl;

    sbd_data->GetDimensions(bl_dim);
    double gstart = gridCalc.GetGridStart();
    double gspace = gridCalc.GetGridSpacing();
    std::size_t ngrid_pts = gridCalc.GetNGridPoints();
    auto mbd_bin_map = gridCalc.GetGridIndexMap();

    ch_mbd_type mbd_data;
    mbd_data.Resize(bl_dim[0], ngrid_pts, bl_dim[2], bl_dim[3]);
    mbd_data.ZeroArray();
    // mbd_data.GetDimensions(bl_dim);

    auto mbd_ax = &(std::get<CH_CHANNEL_AXIS>(mbd_data) );
    for(std::size_t i=0; i<ngrid_pts;i++)
    {
        (*mbd_ax)(i) = i*gspace;
    }

    //fill in the mbd_data before we x-form it
    for(std::size_t pp=0; pp<bl_dim[0]; pp++)
    {
        for(std::size_t ch=0; ch<bl_dim[1]; ch++)
        {
            std::size_t mbd_bin = mbd_bin_map[ch];
            //std::cout<<"ch -> mbd = "<<ch<<", "<<mbd_bin<<std::endl;
            for(std::size_t dr=0; dr<bl_dim[2]; dr++)
            {
                for(std::size_t sbd=0; sbd<bl_dim[3]; sbd++)
                {
                    //if(mbd_bin == 0){std::cout<<"stuf = "<<(*sbd_data)(pp,ch,dr,sbd)<<std::endl;}
                    mbd_data(pp, mbd_bin, dr, sbd) = (*sbd_data)(pp,ch,dr,sbd);
                }
            }
        }
    }

    //now we are going to run a FFT on the mbd axis

    MHO_MultidimensionalFastFourierTransform< ch_mbd_type > fFFTEngine2;
    MHO_CyclicRotator< ch_mbd_type > fCyclicRotator2;
    fFFTEngine2.SetArgs(&mbd_data);
    fFFTEngine2.DeselectAllAxes();
    fFFTEngine2.SelectAxis(CH_CHANNEL_AXIS);
    fFFTEngine2.SetForward();
    status = fFFTEngine2.Initialize();

    if(!status){std::cout<<"FAIL1"<<std::endl;}

    fCyclicRotator2.SetOffset(CH_CHANNEL_AXIS, ngrid_pts/2);
    fCyclicRotator2.SetArgs(&mbd_data);
    status = fCyclicRotator2.Initialize();
    if(!status){std::cout<<"FAIL2"<<std::endl;}

    status = fFFTEngine2.Execute();
    if(!status){std::cout<<"FAIL3"<<std::endl;}
    status = fCyclicRotator2.Execute();
    if(!status){std::cout<<"FAIL4"<<std::endl;}


    #ifdef USE_ROOT

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

    auto dr_rate_ax = std::get<CH_TIME_AXIS>(*sbd_data);
    auto delay_ax = std::get<CH_FREQ_AXIS>(*sbd_data);

    //divide out the reference frequency (need to think about this )
    double rescale = 1.0/6.0;//ref freq = 6GHz (axis is then in nanosec/sec)
    for(std::size_t d=0; d<dr_rate_ax.GetSize(); d++)
    {
        dr_rate_ax(d) *= rescale;
    }




    for(std::size_t ch=0; ch<bl_dim[CH_CHANNEL_AXIS]; ch++)
    {
        std::stringstream ss;
        ss << "channel_test";
        ss << ch;

        auto c = cMan.CreateCanvas(ss.str().c_str(), 800, 800);
        auto ch_slice = sbd_data->SliceView(0,ch,":",":");
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
        auto mbd_ax = &(std::get<CH_CHANNEL_AXIS>(mbd_data));

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

        auto new_mbd_ax = std::get<CH_CHANNEL_AXIS>(mbd_data);
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
