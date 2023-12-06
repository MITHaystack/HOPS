#include "MHO_MultitonePhaseCorrection.hh"
#include "MHO_MathUtilities.hh"

#ifdef HOPS_USE_FFTW3
#include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#else
#include "MHO_MultidimensionalFastFourierTransform.hh"
#endif

namespace hops
{


MHO_MultitonePhaseCorrection::MHO_MultitonePhaseCorrection()
{
    fStationKey = "station";
    fRemStationKey = "remote_station";
    fRefStationKey = "reference_station";
    fRemStationMk4IDKey = "remote_station_mk4id";
    fRefStationMk4IDKey = "reference_station_mk4id";
    fChannelLabelKey = "channel_label";
    fSidebandLabelKey = "net_sideband";
    fBandwidthKey = "bandwidth";
    fSkyFreqKey = "sky_freq";
    fLowerSideband = "L";
    fUpperSideband = "U";

    fStationCode = "";
    fMk4ID = "";

    fImagUnit = MHO_Constants::imag_unit;
    fDegToRad = MHO_Constants::deg_to_rad;
};

MHO_MultitonePhaseCorrection::~MHO_MultitonePhaseCorrection(){};


bool
MHO_MultitonePhaseCorrection::ExecuteInPlace(visibility_type* in)
{
    std::size_t st_idx = DetermineStationIndex(in);
    if(st_idx != 0 && st_idx != 1){return false;}
    
    //loop over pol-products and apply pc-phases to the appropriate pol/channel
    auto pp_ax = &(std::get<POLPROD_AXIS>(*in) );
    auto chan_ax = &(std::get<CHANNEL_AXIS>(*in) );
    std::string chan_label;
    std::string pp_label;
    for(std::size_t pp=0; pp < pp_ax->GetSize(); pp++)
    {
        pp_label = pp_ax->at(pp);
        if( PolMatch(st_idx, pp_label) )
        {
            // for(auto pcal_it = fPCMap.begin(); pcal_it != fPCMap.end(); pcal_it++)
            // {
                // chan_label = pcal_it->first;
                // double pc_val = pcal_it->second;
                //TODO, may need to re-work this mapping method if too slow
                MHO_IntervalLabel ilabel;// = chan_ax->GetFirstIntervalWithKeyValue(fChannelLabelKey, chan_label);
    
                if(ilabel.IsValid())
                {
                    //grab the sideband, bandwidth,channel frequency
                    std::size_t ch = ilabel.GetLowerBound();
                    std::string net_sideband; 
                    std::string chan_label;
                    double sky_freq = (*chan_ax)(ch); //get the sky frequency of this channel
                    double bandwidth = 0;
                    auto other_labels = chan_ax->GetIntervalsWhichIntersect(ilabel);
                    for(auto olit = other_labels.begin(); olit != other_labels.end(); olit++)
                    {
                        if( olit->HasKey(fSidebandLabelKey) ){olit->Retrieve(fSidebandLabelKey, net_sideband);}
                        if( olit->HasKey(fBandwidthKey)){olit->Retrieve(fBandwidthKey, bandwidth);}
                    }

                    //figure out the upper/lower frequency limits for this channel
                    double lower_freq, upper_freq;
                    std::size_t start_idx, ntones;
                    DetermineChannelFrequencyLimits(sky_freq, bandwidth, net_sideband, lower_freq, upper_freq);
                    //determine the pcal tones indices associated with this channel
                    DetermineChannelToneIndexes(lower_freq, upper_freq, start_idx, ntones);

                    //need to fit the pcal data for the mean phase and delay for this channel, for each AP
                    //FitPCData(start_idx, ntones);

                    // visibility_element_type pc_phasor = std::exp( fImagUnit*pc_val*fDegToRad );
                    // 
                    // //conjugate phases for LSB data, but not for USB - TODO what about DSB?
                    // if(net_sideband == fLowerSideband){pc_phasor = std::conj(pc_phasor);}
                    // #pragma message("TODO FIXME - test all manual pc phase correction cases (ref/rem/USB/LSB/DSB)")
                    // 
                    // //retrieve and multiply the appropriate sub view of the visibility array
                    // auto chunk = in->SubView(pp, ch);
                    // chunk *= pc_phasor;
                // }
            }
        }
    }
    
    return true;
}


bool
MHO_MultitonePhaseCorrection::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}


std::size_t
MHO_MultitonePhaseCorrection::DetermineStationIndex(const visibility_type* in)
{
    //determine if the p-cal corrections are being applied to the remote or reference station
    std::string val;
    
    if(fMk4ID != "") //selection by mk4 id
    {
        in->Retrieve(fRemStationMk4IDKey, val);
        if(fMk4ID == val){return 1;}
        in->Retrieve(fRefStationMk4IDKey, val);
        if(fMk4ID == val){return 0;}
    }
    
    if(fStationCode != "")//seletion by 2-char station code
    {
        in->Retrieve(fRemStationKey, val);
        if(fStationCode == val){return 1;}
        in->Retrieve(fRefStationKey, val);
        if(fStationCode == val){return 0;}
    }
    
    msg_warn("calibration", "manual pcal, remote/reference station do not match selection."<< eom );
    return 2;
}

bool
MHO_MultitonePhaseCorrection::PolMatch(std::size_t station_idx, std::string& polprod)
{
    make_upper(polprod);
    return (fPol[0] == polprod[station_idx]);
}


bool
MHO_MultitonePhaseCorrection::InitializeInPlace(visibility_type* /*in*/){ return true;}

bool
MHO_MultitonePhaseCorrection::InitializeOutOfPlace(const visibility_type* /*in*/, visibility_type* /*out*/){return true;}

void 
MHO_MultitonePhaseCorrection::DetermineChannelFrequencyLimits(double sky_freq, double bandwidth, std::string net_sideband, double& lower_freq, double& upper_freq)
{
    if(net_sideband == fUpperSideband)
    {
        lower_freq = sky_freq;
        upper_freq = sky_freq + bandwidth;
    }
    else //lower sideband
    {
        upper_freq = sky_freq;
        lower_freq = sky_freq - bandwidth;
    }
}

void 
MHO_MultitonePhaseCorrection::DetermineChannelToneIndexes(double lower_freq, double upper_freq, std::size_t& start_idx, std::size_t& ntones)
{
    auto tone_freq_ax = std::get<MTPCAL_FREQ_AXIS>(*fPCData);
    double start_tone_frequency = 0;
    start_idx = 0;
    ntones = 0;
    for(std::size_t j=0; j<tone_freq_ax.GetSize(); j++)
    {
        if( lower_freq <= tone_freq_ax(j) && tone_freq_ax(j) < upper_freq )
        {
            if(ntones == 0)
            {
                start_tone_frequency = tone_freq_ax(j) ;
                start_idx = j;
            }
            ntones++;
            std::cout<<"tone: "<<j<<" = "<<tone_freq_ax(j)<<std::endl;
        }
    }
    std::cout<<"start tone = "<<start_tone_frequency<<", start tone index = "<<start_idx<<", ntones = "<<ntones<<std::endl;

}


void 
MHO_MultitonePhaseCorrection::FitPCData(std::size_t pol_idx, std::size_t ap_idx, std::size_t tone_start_idx, std::size_t ntones)
{
    //should we channelize the pca-data? yes...but for now just do an FFT on one chunk to test
    //extract the p-cal data between the start/stop indices for this particular pol from the first AP
    using pcal_axis_pack = MHO_AxisPack< frequency_axis_type >;
    using pcal_type = MHO_TableContainer< std::complex<double>, pcal_axis_pack >;

    
    int FFTSIZE = 256; //default x-form size in pcalibrate
    pcal_type test;
    test.Resize(FFTSIZE);
    test.ZeroArray();

    auto tone_freq_ax = std::get<MTPCAL_FREQ_AXIS>(*fPCData);

    for(std::size_t j=0; j<ntones; j++)
    {
        std::complex<double> phasor = fPCData->at(pol_idx, ap_idx, tone_start_idx+j);
        double tone_freq = tone_freq_ax(tone_start_idx+j);
        test.at(j) = phasor;
        std::get<0>(test).at(j) = tone_freq;
    }

    double pc_tone_delta = 1e6*(std::get<0>(test)(1) - std::get<0>(test)(0));
    std::cout<<"PCAL TONE DELTA = "<<pc_tone_delta << std::endl;

    for(std::size_t i=0; i<FFTSIZE; i++)
    {
        std::get<0>(test).at(i) = i*pc_tone_delta; //actual freq is irrelevant
    }

    #ifdef HOPS_USE_FFTW3
    using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransformFFTW< pcal_type >;
    #else
    using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransform< pcal_type >;
    #endif

    FFT_ENGINE_TYPE fFFTEngine;


    fFFTEngine.SetArgs(&test);
    fFFTEngine.DeselectAllAxes();
    fFFTEngine.SelectAxis(0); //only perform padded fft on frequency (to lag) axis
    fFFTEngine.SetForward();//forward DFT

    bool ok;
    ok = fFFTEngine.Initialize();
    ok = fFFTEngine.Execute();

    double max_val = 0;
    int max_idx = 0;
    double max_del = 0;
    for(std::size_t i=0; i<FFTSIZE; i++)
    {
        std::complex<double> phasor = test.at(i);
        double abs_val = std::abs(phasor);
        if(abs_val > max_val)
        {
            max_val = abs_val;
            max_idx = i;
            max_del = std::get<0>(test)(i);
        }
    }

    double delay_delta = (std::get<0>(test)(1) - std::get<0>(test)(0));
    std::cout<<"delay_delta = "<<delay_delta<<std::endl;

    std::cout<<"max, max_idx, max_del = "<<max_val<<", "<<max_idx<<", "<<max_del<<std::endl;

    double ymax, ampmax;
    double y[3];
    double q[3];
    y[1] = max_val;
    y[0] =std::abs (test[(max_idx+FFTSIZE-1)%FFTSIZE]);
    y[2] = std::abs (test[(max_idx+FFTSIZE+1)%FFTSIZE]);
    MHO_MathUtilities::parabola(y, -1.0, 1.0, &ymax, &ampmax, q);

                        // DC is in 0th element
    double delay = (max_idx+ymax) / 256.0 / pc_tone_delta;

    double delay2 = (max_idx+ymax)*delay_delta;
    std::cout<<"ymax = "<<ymax<<std::endl;
    std::cout<<"DELAY = "<<delay<<std::endl;
    std::cout<<"DELAY2 = "<<delay2<<std::endl;

                        // find corresponding delay in suitable range
    double pc_amb = 1 / pc_tone_delta;

}


}//end of namespace
