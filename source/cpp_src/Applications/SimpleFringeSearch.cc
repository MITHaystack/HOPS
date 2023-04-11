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
#include "ffmath.h"

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
#include "MHO_VisibilityPrecisionUpCaster.hh"
#include "MHO_WeightPrecisionUpCaster.hh"
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

#ifdef USE_ROOT
    #include "TApplication.h"
    #include "MHO_RootCanvasManager.hh"
    #include "MHO_RootGraphManager.hh"
#endif


using namespace hops;


using mbd_dr_axis_pack = MHO_AxisPack< MHO_Axis<double>, MHO_Axis<double> >;
using mbd_dr_type = MHO_TableContainer< visibility_element_type, mbd_dr_axis_pack>;
using mbd_dr_amp_type = MHO_TableContainer< double, mbd_dr_axis_pack>;

double total_ap_frac;

void fine_peak_interpolation(visibility_type* sbd_arr, weight_type* w_arr, MHO_Axis<double>* mbd_ax, MHO_Axis<double>* dr_ax, int c_mbdmax, int c_drmax, int c_sbdmax)
{
    //follow the algorithm of interp.c (SIMUL) mode, to fill out a cube and interpolate
    double drf[5][5][5];// 5x5x5 cube of fringe values
    double xlim[3][2]; //cube limits each dim
    double xi[3];
    double drfmax;

    auto chan_ax = &( std::get<CHANNEL_AXIS>(*sbd_arr) );
    auto ap_ax = &(std::get<TIME_AXIS>(*sbd_arr));
    auto sbd_ax = &( std::get<FREQ_AXIS>(*sbd_arr) );

    std::size_t nap = ap_ax->GetSize();


    std::size_t nchan = chan_ax->GetSize();

    double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);
    double dr_delta = dr_ax->at(1) - dr_ax->at(0);
    double mbd_delta = mbd_ax->at(1) - mbd_ax->at(0);

    double midpoint_time = ( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    std::cout<<"time midpoint = "<<midpoint_time<<std::endl;

    printf("max bin (sbd, mbd, dr) = %d, %d, %d\n", c_sbdmax, c_mbdmax, c_drmax );

    double ref_freq = 6e3; //6000 MHz gahh

    double sbd_lower = 1e30;
    double sbd_upper = -1e30;
    double mbd_lower = 1e30;
    double mbd_upper = -1e30;
    double dr_lower = 1e30;
    double dr_upper = -1e30;

    MHO_FringeRotation frot;
    std::size_t sbd_bin, dr_bin, mbd_bin;
    double sbd, dr, mbd;

    std::cout<< std::setprecision(14);
    for (std::size_t isbd=0; isbd<5; isbd++)
    {
        for (std::size_t imbd=0; imbd<5; imbd++)
        {
            for (std::size_t idr=0; idr<5; idr++)
            {

                std::complex<double> z = 0.0;

                // calculate location of this tabular point (should modulo % axis size)
                sbd_bin = (c_sbdmax + isbd - 2) % (int) sbd_ax->GetSize();
                dr_bin = (c_drmax + idr - 2 ) % (int) dr_ax->GetSize() ;
                mbd_bin = ( c_mbdmax + imbd - 2) % (int) mbd_ax->GetSize() ;;
                //
                sbd = sbd_ax->at(sbd_bin);
                dr =  (dr_ax->at(dr_bin) )*(1.0/ref_freq);
                mbd = (mbd_ax->at(mbd_bin));

                if(sbd < sbd_lower){sbd_lower = sbd;}
                if(sbd > sbd_upper){sbd_upper = sbd;}

                if(dr < dr_lower){dr_lower = dr;}
                if(dr > dr_upper){dr_upper = dr;}

                if(mbd < mbd_lower){mbd_lower = mbd;}
                if(mbd > mbd_upper){mbd_upper = mbd;}

                // sbd = status.max_delchan    +        isbd - 2;
                // mbd = status.mbd_max_global + 0.5 * (imbd - 2) * status.mbd_sep;
                // dr  = status.dr_max_global  + 0.5 * (idr - 2)  * status.rate_sep;

                // msg ("[interp]dr %le mbd %le sbd %d sbd_max(ns) %10.6f", -1,
                //  dr,mbd,sbd,status.sbd_max);
                                // counter-rotate data from all freqs. and AP's
                for(std::size_t fr = 0; fr < nchan; fr++)
                {
                    //double frq = pass->pass_data + fr;
                    double freq = (*chan_ax)(fr);//use sky-freq of this channel????
                    for(std::size_t ap = 0; ap < nap; ap++)
                    {
                        double tdelta = ap_ax->at(ap) + ap_delta/2.0 - midpoint_time; //need time difference from the f.r.t?
                        visibility_element_type vis = (*sbd_arr)(0,fr,ap,sbd_bin);
                        std::complex<double> vr = frot.vrot(tdelta, freq, ref_freq, dr, mbd);
                        std::complex<double> x = vis * vr;// vrot_mod(tdelta, dr, mbd, freq, ref_freq);
                        x *= (*w_arr)(0,fr,ap,0); //multiply by the 'weight'
                        z = z + x;
                    }
                }

                z = z * 1.0 / (double) total_ap_frac;
                drf[isbd][imbd][idr] = std::abs(z);
                //std::cout<<isbd<<", "<<imbd<<", "<<idr<<", "<<drf[isbd][imbd][idr]<<std::endl;
                //printf("%ld %le %le \n", sbd_bin, mbd, dr);
                printf ("drf[%ld][%ld][%ld] %lf \n", isbd, imbd, idr, drf[isbd][imbd][idr]);
            }
        }
    }



    xlim[0][0] = -1;// sbd_lower;// / status.sbd_sep - status.max_delchan + nl;
    xlim[0][1] = 1;//sbd_upper;// / status.sbd_sep - status.max_delchan + nl;

    xlim[1][0] = -2;//mbd_lower;// - status.mbd_max_global) / status.mbd_sep;
    xlim[1][1] = 2;//mbd_upper;// - status.mbd_max_global) / status.mbd_sep;

    xlim[2][0] = -2;//dr_lower;// - status.dr_max_global) / status.rate_sep;
    xlim[2][1] = 2;//dr_upper;// - status.dr_max_global) / status.rate_sep;

    std::cout<< "xlim's "<< xlim[0][0]<<", "<< xlim[0][1] <<", "<< xlim[1][0] <<", "<< xlim[1][1] <<", " << xlim[2][0] <<", "<< xlim[2][1] <<std::endl;
                                // find maximum value within cube via interpolation
    max555(drf, xlim, xi, &drfmax);

    std::cout<< "xi's "<< xi[0]<<", "<< xi[1] <<", "<< xi[2] <<std::endl;
    std::cout<<"drf max = "<<drfmax<<std::endl;



    // calculate location of this tabular point (should modulo % axis size)
    // std::size_t sbd_bin = loc[3];
    // std::size_t dr_bin = loc[2];
    //std::size_t
    sbd_bin = c_sbdmax;
    dr_bin = c_drmax;
    mbd_bin = c_mbdmax;

    sbd = sbd_ax->at(sbd_bin);// + 0.5*sbd_delta;
    dr =  (dr_ax->at(dr_bin) )*(1.0/ref_freq);
    mbd = (mbd_ax->at(mbd_bin)); 

    double sbd_change = xi[0] * sbd_delta;
    double mbd_change = xi[1] * mbd_delta;
    double dr_change =  (xi[2] * dr_delta)/ref_freq;

    double sbd_max = (sbd + sbd_change);
    double mbd_max_global = mbd + mbd_change;
    double dr_max_global  = dr + dr_change;

    std::cout<<"coarse location (sbd, mbd, dr) = "<<sbd<<", "<<mbd<<", "<<dr<<std::endl;
    std::cout<<"change (sbd, mbd, dr) = "<<sbd_change<<", "<<mbd_change<<", "<<dr_change<<std::endl;
    // std::cout<<"Peak location (sbd, mbd, dr) = "<<sbd_max<<", "<<mbd_max_global<<", "<<dr_max_global<<std::endl;
    std::cout<<"Peak max555, sbd "<<sbd_max<<" mbd "<<mbd_max_global<<" dr "<<dr_max_global<<std::endl;

}











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


    MHO_VisibilityPrecisionUpCaster up_caster;
    up_caster.SetArgs(bl_store_data, bl_data);
    up_caster.Initialize();
    up_caster.Execute();

    MHO_WeightPrecisionUpCaster wt_up_caster;
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

    // //apply the data weights
    // for(std::size_t pp =0; pp < bl_dim[0]; pp++)
    // {
    //     for(std::size_t ch=0; ch < bl_dim[1]; ch++)
    //     {
    //         for(std::size_t ap=0; ap < bl_dim[2]; ap++)
    //         {
    //             bl_data->SubView(pp, ch, ap) *= (*wt_data)(pp, ch, ap, 0);
    //         }
    //     }
    // }

    // //compute the sum of the weights
    // std::cout<<"weight at 0 = " << wt_data->at(0,0,0,0) <<std::endl;
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

    total_ap_frac = temp_weights[0];
    std::cout<<"reduced weights = "<<temp_weights[0]<<std::endl;

    //change weights uuid  to prevent collision with previous snapshot
    // MHO_UUIDGenerator gen;
    // MHO_UUID new_uuid = gen.GenerateUUID(); //random object id
    // temp_weights->SetObjectUUID(new_uuid);
    take_snapshot_here("test", "reduced_weights", __FILE__, __LINE__,  &temp_weights);

    //(*bl_data) *= 1.0/(*wt_data)[0];

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

    //take snapeshot of sbd data after normfx
    take_snapshot_here("test", "sbd", __FILE__, __LINE__, sbd_data);

    //run the transformation to delay rate space (this also involves a zero padded FFT)
    MHO_DelayRate drOp;
    //MHO_DelayRate drOp;
    visibility_type* sbd_dr_data = sbd_data->CloneEmpty();
    drOp.SetReferenceFrequency(6000.0);
    drOp.SetArgs(sbd_data, wt_data, sbd_dr_data);
    //drOp.SetArgs(sbd_data, sbd_dr_data);
    ok = drOp.Initialize();
    check_step_fatal(ok, "main", "dr initialization." << eom );
    ok = drOp.Execute();
    check_step_fatal(ok, "main", "dr execution." << eom );

    take_snapshot_here("test", "sbd_dr", __FILE__, __LINE__, sbd_dr_data);

    MHO_MBDelaySearch mbdSearch;
    mbdSearch.SetArgs(sbd_dr_data);
    ok = mbdSearch.Initialize();
    check_step_fatal(ok, "main", "mbd initialization." << eom );
    ok = mbdSearch.Execute();
    check_step_fatal(ok, "main", "mbd execution." << eom );

    int c_mbdmax = mbdSearch.GetMBDMaxBin();
    int c_sbdmax = mbdSearch.GetSBDMaxBin();
    int c_drmax = mbdSearch.GetDRMaxBin();

    auto mbd_ax_ptr = mbdSearch.GetMBDAxis();//&(std::get<0>(mbd_dr_data) );
    auto mbd_dr_ptr = mbdSearch.GetDRAxis(); //&(std::get<TIME_AXIS>(*sbd_dr_data) );


    //calculate the frequency grid for MBD search
    MHO_UniformGridPointsCalculator gridCalc;
    gridCalc.SetPoints( std::get<CHANNEL_AXIS>(*bl_data).GetData(), std::get<CHANNEL_AXIS>(*bl_data).GetSize() );
    gridCalc.Calculate();
    
    //std::cout<<"grid info: "<<gridCalc.GetGridStart()<<", "<<gridCalc.GetGridSpacing()<<", "<<gridCalc.GetNGridPoints()<<std::endl;
    sbd_dr_data->GetDimensions(bl_dim);
    double gstart = gridCalc.GetGridStart();
    double gspace = gridCalc.GetGridSpacing();
    std::size_t ngrid_pts = gridCalc.GetNGridPoints();
    auto mbd_bin_map = gridCalc.GetGridIndexMap();
    
    //some dims
    std::size_t nsdb = sbd_dr_data->GetDimension(FREQ_AXIS);
    std::size_t ndr = sbd_dr_data->GetDimension(TIME_AXIS);
    
    mbd_dr_type mbd_dr_data;
    mbd_dr_amp_type mbd_dr_amp_data;
    mbd_dr_data.Resize(ngrid_pts, ndr);
    mbd_dr_amp_data.Resize(ngrid_pts, ndr);
    
    //set up FFT and rotator engines
    MHO_MultidimensionalFastFourierTransform< mbd_dr_type > fFFTEngine2;
    MHO_CyclicRotator< mbd_dr_type > fCyclicRotator2;
    fFFTEngine2.SetArgs(&mbd_dr_data);
    fFFTEngine2.DeselectAllAxes();
    fFFTEngine2.SelectAxis(0);
    fFFTEngine2.SetForward();
    ok = fFFTEngine2.Initialize();
    check_step_fatal(ok, "main", "fft engine initialization." << eom );
    
    fCyclicRotator2.SetOffset(0, ngrid_pts/2);
    fCyclicRotator2.SetArgs(&mbd_dr_data);
    ok = fCyclicRotator2.Initialize();
    check_step_fatal(ok, "main", "cyclic rotation initialization." << eom );

    // loop over the single-band delay 'lags', computing the MBD/DR function
    // find the max for each SBD
    double maxmbd = 0.0;
    for(std::size_t sbd_idx=0; sbd_idx<nsdb; sbd_idx++)
    {
        mbd_dr_data.ZeroArray(); //zero out workspace
        mbd_dr_amp_data.ZeroArray();
    
        //set up the mbd delay axis (in frequency space)
        auto mbd_ax = &(std::get<0>(mbd_dr_data) );
        for(std::size_t i=0; i<ngrid_pts;i++)
        {
            mbd_ax->at(i) = i*gspace;
        }
    
        //set up the delay rate axis
        auto dr_ax = &(std::get<1>(mbd_dr_data) );
        for(std::size_t i=0;i<ndr;i++)
        {
            dr_ax->at(i) = std::get<TIME_AXIS>(*sbd_dr_data)(i);
        }
    
        //copy in the data from each channel for this SDB/DR
        for(std::size_t ch=0; ch<bl_dim[1]; ch++)
        {
            std::size_t mbd_bin = mbd_bin_map[ch];
            for(std::size_t dr_idx=0; dr_idx < ndr; dr_idx++)
            {
                 mbd_dr_data(mbd_bin, dr_idx) = (*sbd_dr_data)(0, ch, dr_idx, sbd_idx);
            }
            // mbd_dr_data.SliceView(mbd_bin, ":") = sbd_dr_data->SliceView(0, ch,":" ,sbd_idx); //TODO why does this not work??
        }
    
        //now run an FFT along the MBD axis and cyclic rotate
        ok = fFFTEngine2.Execute();
        check_step_fatal(ok, "main", "fft engine execution." << eom );
        ok = fCyclicRotator2.Execute();
        check_step_fatal(ok, "main", "cyclic rotation execution." << eom );
    
        //set the axes equal
        std::get<0>(mbd_dr_amp_data) = std::get<0>(mbd_dr_data);
        std::get<1>(mbd_dr_amp_data) = std::get<1>(mbd_dr_data);
    
        std::size_t total_mbd_dr_size = mbd_dr_data.GetSize();
        for(std::size_t i=0; i<total_mbd_dr_size; i++)
        {
            mbd_dr_amp_data[i] = std::abs(mbd_dr_data[i]);
        }
    
        //search for the peak in MBD and DR
        MHO_ExtremaSearch< mbd_dr_amp_type > mSearch;
        mSearch.SetArgs(&mbd_dr_amp_data);
        mSearch.Initialize();
        mSearch.Execute();
        std::size_t max_loc = mSearch.GetMaxLocation();
        std::size_t min_loc = mSearch.GetMinLocation();
        auto loc_array = mbd_dr_amp_data.GetIndicesForOffset(max_loc);
        double tmp_max = mbd_dr_amp_data[max_loc]/total_ap_frac;
    
        // std::cout<<"index sbd, mbd, dr: "<<sbd_idx<<", "<<loc_array[0]<<", "<<loc_array[1]<<std::endl;
        // std::cout<<"mbd, dr bins = "<< std::get<0>(mbd_dr_amp_data)(loc_array[0])<<", "<<std::get<1>(mbd_dr_amp_data)(loc_array[1]) <<std::endl;
        // std::cout<<"max value = "<<tmp_max<<std::endl;
    
        if(tmp_max > maxmbd)
        {
            maxmbd = tmp_max;
            c_mbdmax = loc_array[0];
            c_sbdmax = sbd_idx;
            c_drmax = loc_array[1];
        }
    
        if(sbd_idx == 255)
        {
            take_snapshot_here("test", "mbd_dr", __FILE__, __LINE__, &mbd_dr_amp_data);
        }
    }


    // 
    // 
    // 
    // // ////////////////////////////////////////////////////////////////////////////
    // // //FINE INTERPOLATION STEP (search over 5x5x5 grid around peak)
    // // ////////////////////////////////////////////////////////////////////////////
    
    fine_peak_interpolation(sbd_data, wt_data, mbd_ax_ptr, mbd_dr_ptr, c_mbdmax, c_drmax, c_sbdmax);



    ////////////////////////////////////////////////////////////////////////////
    //PLOTTING/DEBUG
    ////////////////////////////////////////////////////////////////////////////



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
