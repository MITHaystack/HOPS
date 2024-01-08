#include "MHO_MultitonePhaseCorrection.hh"
#include "MHO_MathUtilities.hh"

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
    fStationIndex = 2;

    fImagUnit = MHO_Constants::imag_unit;
    fDegToRad = MHO_Constants::deg_to_rad;

    fPCPeriod = 30;

    //initialize the FFT engine
    fWorkspaceSize = 256;
    fPCWorkspace.Resize(fWorkspaceSize); //default size is 256
    fFFTEngine.SetArgs(&fPCWorkspace);
    fFFTEngine.SelectAllAxes();
    fFFTEngine.SetForward();//forward DFT

    bool ok;
    ok = fFFTEngine.Initialize(); // TODO FIXME...only need to initialize once!
};

MHO_MultitonePhaseCorrection::~MHO_MultitonePhaseCorrection(){};


bool
MHO_MultitonePhaseCorrection::ExecuteInPlace(visibility_type* in)
{
    //figure out if refrence or remote station in this baseline
    fStationIndex = DetermineStationIndex(in);
    if(fStationIndex == 2)
    {
        msg_error("calibration", "could not determine station index for multitone pcal operation." << eom);
        return false;
    }

    //loop over polarization in pcal data and pol-products
    //so we can apply the phase-cal to the appropriate pol/channel/ap
    auto pcal_pol_ax = &(std::get<MTPCAL_POL_AXIS>(*fPCData));
    auto vis_pp_ax = &(std::get<POLPROD_AXIS>(*in) );

    //loop over the p-cal polarizations
    for(std::size_t pc_pol=0; pc_pol < pcal_pol_ax->GetSize(); pc_pol++)
    {
        std::string pc_pol_label = pcal_pol_ax->at(pc_pol);
        //loop over the visibility pol-products
        for(std::size_t vis_pp=0; vis_pp < vis_pp_ax->GetSize(); vis_pp++)
        {
            std::string vis_pp_label = vis_pp_ax->at(vis_pp);
            //check if this pcal-pol matches the station's pol for this pol-product
            if( PolMatch(fStationIndex, pc_pol_label, vis_pp_label) )
            {
                //apply the phase-cal for all channels/APs
                ApplyPCData(pc_pol, vis_pp, in);
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


void
MHO_MultitonePhaseCorrection::ApplyPCData(std::size_t pc_pol, std::size_t vis_pp, visibility_type* in)
{
    auto vis_chan_ax = &(std::get<CHANNEL_AXIS>(*in) );
    auto vis_ap_ax = &(std::get<TIME_AXIS>(*in) );
    auto vis_freq_ax = &(std::get<FREQ_AXIS>(*in) );
    auto pcal_pol_ax = &(std::get<MTPCAL_POL_AXIS>(*fPCData) );
    auto tone_freq_ax = &(std::get<MTPCAL_FREQ_AXIS>(*fPCData) );

    std::string pc_pol_code = pcal_pol_ax->at(pc_pol);

    //workspace to store averaged phasors and corresponding tone freqs
    fPCWorkspace.ZeroArray();
    auto workspace_freq_ax = &(std::get<0>(fPCWorkspace));

    //grab the sampler delay vector
    std::vector<double> sampler_delays;
    pcal_pol_ax->RetrieveIndexLabelKeyValue(pc_pol, "sampler_delays", sampler_delays);
    std::cout<<"N sampler delays = "<<sampler_delays.size()<<std::endl;

    //now loop over the channels
    for(std::size_t ch=0; ch < vis_chan_ax->GetSize(); ch++)
    {
        //get channel's frequency info
        double sky_freq = (*vis_chan_ax)(ch); //get the sky frequency of this channel
        double bandwidth = 0;
        std::string net_sideband;

        bool key_present = vis_chan_ax->RetrieveIndexLabelKeyValue(ch, fSidebandLabelKey, net_sideband);
        if(!key_present){msg_error("calibration", "missing net_sideband label for channel "<< ch << "." << eom); }
        key_present = vis_chan_ax->RetrieveIndexLabelKeyValue(ch, fBandwidthKey, bandwidth);
        if(!key_present){msg_error("calibration", "missing bandwidth label for channel "<< ch << "." << eom);}

        //figure out the upper/lower frequency limits for this channel
        //std::cout<<"working on channel: "<<ch<<" with sky freq: "<<sky_freq<<" sideband: "<<net_sideband<< std::endl;
        double lower_freq, upper_freq;
        std::size_t start_idx, ntones;
        DetermineChannelFrequencyLimits(sky_freq, bandwidth, net_sideband, lower_freq, upper_freq);
        double chan_center_freq = 0.5*(lower_freq+upper_freq);
        //determine the pcal tones indices associated with this channel
        DetermineChannelToneIndexes(lower_freq, upper_freq, start_idx, ntones);

        //probably should bump up workspace if needed, but for now just bail out
        if(fWorkspaceSize < ntones )
        {
            msg_fatal("calibration", "number of pcal tones: "<< ntones << " exceeds workspace size of: " << fWorkspaceSize << eom);
            std::exit(1);
            //possible implementation to avoid erroring out:
            //fWorkspaceSize = NextLargestPowerOfTwo(2*ntones);
            //fPCWorkspace.Resize(fWorkspaceSize);
            //fFFTEngine.Initialize()
        }

        //grab the sampler delay associated with this channel
        std::size_t sampler_delay_index;
        double sampler_delay = 0.0;
        bool sd_ok = false;
        std::string sd_key;
        if(fStationIndex == 0){sd_key = "ref_sampler_index";}
        if(fStationIndex == 1){sd_key = "rem_sampler_index";}

        sd_ok = vis_chan_ax->RetrieveIndexLabelKeyValue(ch, sd_key, sampler_delay_index);
        if(sd_ok && sampler_delay_index < sampler_delays.size())
        {
            //TODO FIXME -- document units!
            sampler_delay = 1e-9*(sampler_delays[sampler_delay_index]); //sampler delays are specified in ns
        }
        else
        {
            sampler_delay = 0.0;
            msg_warn("calibration", "failed to retrieve sampler delay for station: "<< fMk4ID <<" channel: "<<ch<<"."<<eom);
        }

        //now need to fit the pcal data for the mean phase and delay for this channel, for each AP
        //TODO FIXME -- make sure the stop/start parameters are accounted for
        //this should be fine, provided if we trim the pcal data appropriately ahead use here

        double navg;
        std::size_t seg_start_ap, seg_end_ap;

        std::vector< double > pc_mag_segs;
        std::vector< double > pc_phase_segs;
        std::vector< double > pc_delay_segs;
        for(std::size_t ap=0; ap < vis_ap_ax->GetSize(); ap++)
        {
            if(ap % fPCPeriod == 0)
            {
                //start, clear accumulation, and set tone frequencies
                navg = 0.0;
                fPCWorkspace.ZeroArray();
                for(std::size_t i=0; i<ntones; i++){ workspace_freq_ax->at(i) = tone_freq_ax->at(start_idx + i); }
                seg_start_ap = ap;
                seg_end_ap = ap;
            }

            //sum the tone phasors
            //TODO FIXME -- NOTE!! This implementation assumes all tones are sequential and there are no missing tones!
            //true for now...but may not be once we add pc_tonemask support
            for(std::size_t i=0; i<ntones; i++){ fPCWorkspace(i) += fPCData->at(pc_pol, ap, start_idx+i); }
            navg += 1.0; //treat all pc weights as 1 for now (should probably use visibility weights?)

            //finish the average, do delay fit on last ap of segment or last ap
            if(ap % fPCPeriod == fPCPeriod-1 || ap == vis_ap_ax->GetSize()-1 )
            {
                seg_end_ap = ap+1;
                for(std::size_t i=0; i<ntones; i++){ fPCWorkspace(i) /= navg;}

                double pcal_model[3];
                FitPCData(ntones, chan_center_freq, sampler_delay, pcal_model);

                double pcmag = pcal_model[0];
                double pcphase = pcal_model[1];
                double pcdelay = pcal_model[2];

                double phase_shift = 0.0; // = pcal_model[1]/(4*)
                std::complex<double> pc_phasor = std::exp( -1.0*fImagUnit*( pcphase + phase_shift) );
                pc_mag_segs.push_back(pcmag);
                pc_phase_segs.push_back(pcphase*(180./M_PI));
                pc_delay_segs.push_back(pcdelay);

                for(std::size_t dap = seg_start_ap; dap < seg_end_ap; dap++)
                {
                        //std::complex<double> pc_phasor = std::exp( -2.0*M_PI*fImagUnit*(pcal_model[1]*deltaf) + -1.0*(pcal_model[0] + phase_shift) );
                        //in->SubView(vis_pp, ch, dap)  *= pc_phasor;
                }

            }

            //Now attach the multi-tone phase cal data to each channel
            //this is a bit of a hack...we probably ought to introduce a new data type which
            //encapsulates the reduces multi-tone pcal data
            std::string pc_mag_key;
            std::string pc_phase_key;
            std::string pc_delay_key;
            if(fStationIndex == 0)
            {
                pc_mag_key = "ref_mtpc_mag_";
                pc_phase_key = "ref_mtpc_phase_";
                pc_delay_key = "ref_mtpc_delays_";
            }
            if(fStationIndex == 1)
            {
                pc_mag_key = "rem_mtpc_mag_";
                pc_phase_key = "rem_mtpc_phase_";
                pc_delay_key = "rem_mtpc_delays_";
            }
            pc_mag_key += pc_pol_code;
            pc_phase_key += pc_pol_code;
            pc_delay_key += pc_pol_code;

            vis_chan_ax->InsertIndexLabelKeyValue(ch, pc_mag_key, pc_mag_segs);
            vis_chan_ax->InsertIndexLabelKeyValue(ch, pc_phase_key, pc_phase_segs);
            vis_chan_ax->InsertIndexLabelKeyValue(ch, pc_delay_key, pc_delay_segs);
        }
    }

    std::cout<<"channel axis meta data = "<< vis_chan_ax->GetMetaDataAsJSON().dump(2) <<std::endl;

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
MHO_MultitonePhaseCorrection::PolMatch(std::size_t station_idx, std::string& pc_pol, std::string& polprod)
{
    make_upper(polprod);
    make_upper(pc_pol);
    return (pc_pol[0] == polprod[station_idx]);
}

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
                start_tone_frequency = tone_freq_ax(j);
                start_idx = j;
            }
            ntones++;
        }
    }
}


void
MHO_MultitonePhaseCorrection::FitPCData(std::size_t ntones, double chan_center_freq, double sampler_delay, double* pcal_model)
{
    //copy the averaged tone data for later use when calculating mean phase
    pcal_type pc_data_copy;
    pc_data_copy.Resize(ntones);
    auto tone_freq_ax = &(std::get<0>(pc_data_copy));
    for(std::size_t i=0; i<ntones; i++)
    {
        pc_data_copy(i) = fPCWorkspace(i);
        (*tone_freq_ax)(i) = std::get<0>(fPCWorkspace)(i);
    }

    //calulate the pc delay ambiguity
    double pc_amb = 1.0/(std::get<0>(fPCWorkspace)(1) - std::get<0>(fPCWorkspace)(0));
    pc_amb *= 1e-6; //TODO FIXME - document units (converts to ns)

    //fFFTEngine already points to fPCWorkspace, so just execute
    bool ok = fFFTEngine.Execute();

    double max_val = 0;
    int max_idx = 0;
    double max_del = 0;
    for(std::size_t i=0; i<fWorkspaceSize; i++)
    {
        std::complex<double> phasor = fPCWorkspace(i);
        double abs_val = std::abs(phasor);
        if(abs_val > max_val)
        {
            max_val = abs_val;
            max_idx = i;
            max_del = std::get<0>(fPCWorkspace)(i);
        }
    }

    double delay_delta = (std::get<0>(fPCWorkspace)(1) - std::get<0>(fPCWorkspace)(0));
    double ymax, ampmax;
    double y[3];
    double q[3];
    y[1] = max_val;
    y[0] = std::abs( fPCWorkspace( (max_idx+fWorkspaceSize-1)%fWorkspaceSize) );
    y[2] = std::abs( fPCWorkspace( (max_idx+fWorkspaceSize+1)%fWorkspaceSize) );
    MHO_MathUtilities::parabola(y, -1.0, 1.0, &ymax, &ampmax, q);
    double delay = (max_idx+ymax)*delay_delta;

    delay *= 1e-6; //TODO FIXME - document proper units!

    // find bounds of allowable resolved delay
    double lo = delay + sampler_delay - pc_amb / 2.0;
    double hi = delay + sampler_delay + pc_amb / 2.0;
    while (hi < delay)  // shift delay left if necessary
        delay -= pc_amb;
    while (lo > delay)  // shift delay right if necessary
        delay += pc_amb;

    //std::cout<<"chan center freq = "<<chan_center_freq<<std::endl;
    //rotate each tone phasor by the delay (zero rot at center freq)
    std::complex<double> mean_phasor = 0.0;
    double navg = 0.0;
    for(std::size_t i=0; i<ntones; i++)
    {
        std::complex<double> phasor = pc_data_copy(i);
        double tone_freq = (*tone_freq_ax)(i);
        double deltaf = (chan_center_freq - tone_freq)*1e6;
        // std::cout<<"deltaf = "<<deltaf<<std::endl;
        double theta = 2.0 * M_PI * delay * deltaf;
        // std::cout<<"theta = "<<theta*(180.0/M_PI)<<std::endl;
        phasor *= std::exp(fImagUnit*theta );
        mean_phasor += phasor;
        navg += 1.0;
    }

    mean_phasor = mean_phasor / navg;

    pcal_model[0] = std::abs(mean_phasor); //magnitude
    pcal_model[1] = -1.0*std::arg(mean_phasor); //phase
    pcal_model[2] = delay; //delay

}


}//end of namespace
