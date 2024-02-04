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

    fPCPeriod = 1;

    //initialize the FFT engine
    fWorkspaceSize = 256;
    fPCWorkspace.Resize(fWorkspaceSize); //default size is 256
    fFFTEngine.SetArgs(&fPCWorkspace);
    fFFTEngine.SelectAllAxes();
    fFFTEngine.SetForward();//forward DFT
    
    fWeights = nullptr;

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

    RepairMK4PCData(in);

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

    //now loop over the channels
    for(std::size_t ch=0; ch < vis_chan_ax->GetSize(); ch++)
    {
        //get channel's frequency info
        double sky_freq = (*vis_chan_ax)(ch); //get the sky frequency of this channel
        double bandwidth = 0;
        std::string net_sideband;

        bool key_present = vis_chan_ax->RetrieveIndexLabelKeyValue(ch, fSidebandLabelKey, net_sideband);
        if(!key_present){msg_error("calibration", "missing net_sideband label for channel "<< ch << " with sky_freq: "<<sky_freq << eom); }
        key_present = vis_chan_ax->RetrieveIndexLabelKeyValue(ch, fBandwidthKey, bandwidth);
        if(!key_present){msg_error("calibration", "missing bandwidth label for channel "<< ch << " with sky_freq: "<<sky_freq << eom);}

        //figure out the upper/lower frequency limits for this channel
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
        }

        if(ntones == 0)
        {
            msg_error("calibration", "no pcal tones found for channel: " << ch << " with sky_freq: "<<sky_freq << eom);
        }
        else 
        {
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
            std::vector< int > seg_start_aps;
            std::vector< int > seg_end_aps;

            std::size_t ap_start = 0;
            std::size_t ap_stop = std::min(vis_ap_ax->GetSize(), fPCData->GetDimension(1) );
            for(std::size_t ap=0; ap < ap_stop; ap++)
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
                #pragma message("TODO FIXME -- fix the phase cal phasor weights and implement pc_tonemask.")
                //TODO FIXME -- NOTE!! This implementation assumes all tones are sequential and there are no missing tones!
                //true for now...but may not be once we add pc_tonemask support
                double wght;
                for(std::size_t i=0; i<ntones; i++)
                {
                    wght = 1.0; //pc weights default to 1
                    if(fWeights != nullptr)
                    {
                        wght = fWeights->at(vis_pp, ch, ap, 0);
                    }
                    fPCWorkspace(i) += wght*( fPCData->at(pc_pol, ap, start_idx+i) ); 
                }
                navg += wght; 

                //finish the average, do delay fit on last ap of segment or last ap and append to list
                if(ap % fPCPeriod == fPCPeriod-1 || (ap == ap_stop-1) )
                {
                    seg_end_ap = ap+1;
                    for(std::size_t i=0; i<ntones; i++){ fPCWorkspace(i) /= navg;}
                    double pcal_model[3];
                    FitPCData(ntones, chan_center_freq, sampler_delay, pcal_model);
                    seg_start_aps.push_back(seg_start_ap);
                    seg_end_aps.push_back(seg_end_ap);
                    pc_mag_segs.push_back(pcal_model[0]);
                    pc_phase_segs.push_back(pcal_model[1]);
                    pc_delay_segs.push_back(pcal_model[2]);
                }
            }

            //Now attach the multi-tone phase cal data to each channel
            //this is a bit of a hack...we probably ought to introduce a new data type which
            //encapsulates the reduced multi-tone pcal data and store it in the container store
            std::string st_prefix = "ref";
            if(fStationIndex == 1){st_prefix = "rem";}
            std::string pc_seg_start_key = st_prefix + "_mtpc_seg_start_" + pc_pol_code;
            std::string pc_seg_end_key = st_prefix + "_mtpc_apseg_end_" + pc_pol_code;
            std::string pc_mag_key = st_prefix + "_mtpc_mag_" + pc_pol_code;
            std::string pc_phase_key = st_prefix + "_mtpc_phase_" + pc_pol_code;
            std::string pc_delay_key = st_prefix + "_mtpc_delays_" + pc_pol_code;
            vis_chan_ax->InsertIndexLabelKeyValue(ch, pc_seg_start_key, seg_start_aps);
            vis_chan_ax->InsertIndexLabelKeyValue(ch, pc_seg_end_key, seg_end_aps);
            vis_chan_ax->InsertIndexLabelKeyValue(ch, pc_mag_key, pc_mag_segs);
            vis_chan_ax->InsertIndexLabelKeyValue(ch, pc_phase_key, pc_phase_segs);
            vis_chan_ax->InsertIndexLabelKeyValue(ch, pc_delay_key, pc_delay_segs);
        }
    }










    //here follows the 'sampler_delay.c' code which averages the pcal delays over all 
    //channels and AP's which belong to the same sampler. We should investigate if 
    //this is really needed, or if it improves the post-fit residuals. It doesn't
    //seem to be necessary and overcomplicates the code.
    
    //loop over sampler delays and average
    for(std::size_t sd=0; sd<sampler_delays.size(); sd++)
    {
        std::vector< double > mean_pc_delay;
        bool first_pass = true;
        double n_avg = 0.0;
        //now loop over the channels to accumulate
        for(std::size_t ch=0; ch < vis_chan_ax->GetSize(); ch++)
        {
            //grab the sampler delay index associated with this channel
            std::size_t sampler_delay_index;
            double sampler_delay = 0.0;
            bool sd_ok = false;
            std::string sd_key;
            if(fStationIndex == 0){sd_key = "ref_sampler_index";}
            if(fStationIndex == 1){sd_key = "rem_sampler_index";}
            sd_ok = vis_chan_ax->RetrieveIndexLabelKeyValue(ch, sd_key, sampler_delay_index);

            if(sd == sampler_delay_index)
            {
                std::vector< double > pc_delay_segs;
                std::string st_prefix = "ref";
                if(fStationIndex == 1){st_prefix = "rem";}
                std::string pc_delay_key = st_prefix + "_mtpc_delays_" + pc_pol_code;
                vis_chan_ax->RetrieveIndexLabelKeyValue(ch, pc_delay_key, pc_delay_segs);
                
                std::size_t nsegs = pc_delay_segs.size();
                if(first_pass){mean_pc_delay.resize(nsegs, 0.0); first_pass = false;}
                for(std::size_t s=0; s<nsegs; s++)
                {
                    //std::cout<<"ch pc_delay seg = "<<ch<<", "<<s<<", "<<1e9*pc_delay_segs[s]<<std::endl;
                    mean_pc_delay[s] += pc_delay_segs[s]; 
                }
                n_avg += 1.0;
            }
        }

        //compute the average 
        for(std::size_t i=0; i<mean_pc_delay.size(); i++)
        {
            mean_pc_delay[i] /= n_avg;
        }
        
        //now loop over the channels and insert the 'averaged' delay to be applied
        for(std::size_t ch=0; ch < vis_chan_ax->GetSize(); ch++)
        {
            //grab the sampler delay index associated with this channel
            std::size_t sampler_delay_index;
            double sampler_delay = 0.0;
            bool sd_ok = false;
            std::string sd_key;
            if(fStationIndex == 0){sd_key = "ref_sampler_index";}
            if(fStationIndex == 1){sd_key = "rem_sampler_index";}
            sd_ok = vis_chan_ax->RetrieveIndexLabelKeyValue(ch, sd_key, sampler_delay_index);

            if(sd == sampler_delay_index)
            {
                std::string st_prefix = "ref";
                if(fStationIndex == 1){st_prefix = "rem";}
                std::string pc_delay_key = st_prefix + "_mtpc_delays_applied_" + pc_pol_code;
                vis_chan_ax->InsertIndexLabelKeyValue(ch, pc_delay_key, mean_pc_delay);
            }
        }
    }
    

    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    //now loop over the channels and actually apply the processed pcal phase 
    //and averaged-down delays
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

        std::vector< int > seg_start_aps;
        std::vector< int > seg_end_aps;
        std::vector< double > pc_mag_segs;
        std::vector< double > pc_phase_segs;
        std::vector< double > pc_delay_segs;
        
        std::string st_prefix = "ref";
        if(fStationIndex == 1){st_prefix = "rem";}
        std::string pc_seg_start_key = st_prefix + "_mtpc_seg_start_" + pc_pol_code;
        std::string pc_seg_end_key = st_prefix + "_mtpc_apseg_end_" + pc_pol_code;
        // std::string pc_mag_key = st_prefix + "_mtpc_mag_" + pc_pol_code;
        std::string pc_phase_key = st_prefix + "_mtpc_phase_" + pc_pol_code;

        std::string pc_delay_key = st_prefix + "_mtpc_delays_applied_" + pc_pol_code;

        //the string below is the old behavior (above is closer to the fourfit sampler_delays.c behavior)
        // std::string pc_delay_key = st_prefix + "_mtpc_delays_" + pc_pol_code;


        vis_chan_ax->RetrieveIndexLabelKeyValue(ch, pc_seg_start_key, seg_start_aps);
        vis_chan_ax->RetrieveIndexLabelKeyValue(ch, pc_seg_end_key, seg_end_aps);
        // vis_chan_ax->RetrieveIndexLabelKeyValue(ch, pc_mag_key, pc_mag_segs);
        vis_chan_ax->RetrieveIndexLabelKeyValue(ch, pc_phase_key, pc_phase_segs);
        vis_chan_ax->RetrieveIndexLabelKeyValue(ch, pc_delay_key, pc_delay_segs);
        
        for(std::size_t seg=0; seg < seg_start_aps.size(); seg++)
        {
            std::size_t seg_start_ap = seg_start_aps[seg];
            std::size_t seg_end_ap = seg_end_aps[seg];
            double pcphase = pc_phase_segs[seg];
            double pcdelay = pc_delay_segs[seg];

            #pragma message("TODO FIXME -- 'phase-shift' needs testing for both USB/LSB data as applied in norm_fx.c, line 396")
            double speriod = 1.0/(2.0*bandwidth*1e6);
            double phase_shift = -1.0 * pcdelay / (4.0*speriod) ;

            #pragma message("TODO FIXME -- make sure proper treatment of LSB/USB sidebands is done here.")
            std::complex<double> pc_phasor = std::exp( -1.0*fImagUnit*(pcphase) );

            //conjugate pc phasor when applied to reference station
            if(fStationIndex == 0){pc_phasor = std::conj(pc_phasor);}
            
            for(std::size_t dap = seg_start_ap; dap < seg_end_ap; dap++)
            {
                //apply phase offset correction
                in->SubView(vis_pp, ch, dap) *= pc_phasor;
                
                //std::cout<<"pol, ch, ap, phase/delay = "<<vis_pp<<", "<<ch<<", "<<dap<<", "<<pcphase<<", "<<1e9*pcdelay<<std::endl;

                //apply delay correction
                for(std::size_t sp=0; sp < vis_freq_ax->GetSize(); sp++)
                {
                    double deltaf = ( (*vis_freq_ax)(sp) )*1e6; //Hz
                    std::complex<double> pc_delay_phasor = std::exp( -2.0*M_PI*fImagUnit*(pcdelay*deltaf + phase_shift ) );
                    //conjugate pc phasor when applied to reference station
                    if(fStationIndex == 0){pc_delay_phasor = std::conj(pc_delay_phasor);}
                    (*in)(vis_pp, ch, dap, sp) *= pc_delay_phasor;
                }
            }
        }
    }

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

    msg_warn("calibration", "multitone pcal, remote/reference station do not match selection."<< eom );
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
    #pragma message("TODO FIXME -- need to retrieve the station delays for multitone pcal processing.")
    double station_delay = 0.0;

    //copy the averaged tone data for later use when calculating mean phase
    pcal_type pc_data_copy;
    pc_data_copy.Resize(ntones);
    pc_data_copy.ZeroArray();
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

    delay *= 1e-6; //TODO FIXME - document proper units! (this is seconds)

    // find bounds of allowable resolved delay
    double lo = station_delay + sampler_delay - pc_amb / 2.0;
    double hi = station_delay + sampler_delay + pc_amb / 2.0;
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
        double deltaf = (chan_center_freq - tone_freq)*1e6; //Hz
        // std::cout<<"deltaf = "<<deltaf<<std::endl;
        double theta = 2.0 * M_PI * delay * deltaf;
        // std::cout<<"theta = "<<theta*(180.0/M_PI)<<std::endl;
        phasor *= std::exp(fImagUnit*theta );
        mean_phasor += phasor;
        navg += 1.0;
    }

    #pragma message("TODO FIXME -- verify all sign/conjugation operations work properly for USB/LSB data.")
    mean_phasor = std::conj( mean_phasor ) / navg;

    pcal_model[0] = std::abs(mean_phasor); //magnitude
    pcal_model[1] = std::arg(mean_phasor); //phase
    pcal_model[2] = delay; //delay

}



void MHO_MultitonePhaseCorrection::RepairMK4PCData(visibility_type* vis)
{
    #pragma message("TODO FIXME -- fix hardcoded pcal spacing!!")
    double pcal_spacing = 5.0; //TODO FIXME HARDCODED PCAL SPACING

    //only perform this operation if the pcal data originated from mark4 type309s
    std::string data_origin;
    fPCData->Retrieve("origin", data_origin);
    if(data_origin == "mark4")
    {
        //first loop over the pcal freq axis and extract the channel indexes and ranges 
        auto pc_tone_ax = &(std::get<MTPCAL_FREQ_AXIS>(*fPCData));
        auto chan_ax = &(std::get<CHANNEL_AXIS>(*vis));
        std::map< std::size_t, std::pair<std::size_t, std::size_t> > chanidx2range;
        
        auto interval_objs = pc_tone_ax->GetMatchingIntervalLabels("channel_index");
        for(std::size_t i=0; i<interval_objs.size(); i++) 
        {
            std::size_t channel_idx = interval_objs[i]["channel_index"].get<int>();
            std::size_t low = interval_objs[i]["lower_index"].get<int>();
            std::size_t high = interval_objs[i]["upper_index"].get<int>();
            chanidx2range[channel_idx] = std::make_pair(low, high);
        }

        double sky_freq = 0;
        double bandwidth = 0;
        std::string net_sideband;

        //fix the tone frequency axis (deduce this from the channel boundaries and pcal tone spacing)
        for(auto it = chanidx2range.begin(); it != chanidx2range.end(); it++)
        {
            std::size_t ch = it->first; //channel index
            std::size_t start = it->second.first; //tone start index
            std::size_t stop = it->second.second; //tone stop index

            //make sure to grab the channel that matches the channel index
            //we need to do this in case the 'freqs' command has been applied to sub-select channels
            std::size_t ch_idx = 0;
            for(ch_idx=0; ch_idx<chan_ax->GetSize(); ch_idx++)
            {
                int chid = -1;
                chan_ax->RetrieveIndexLabelKeyValue(ch_idx, "channel", chid);
                if(chid == ch)
                {
                    std::string channel_label;
                    chan_ax->RetrieveIndexLabelKeyValue(ch_idx, "channel_label", channel_label);
                    break;
                }
            }

            if( ch_idx < chan_ax->GetSize() ) //will be equal to chan_ax size if channel not found
            {
                //get the channel frequency info and range
                sky_freq = (*chan_ax)(ch_idx); //get the sky frequency of this channel
                bandwidth = 0;
                net_sideband;
                bool key_present = chan_ax->RetrieveIndexLabelKeyValue(ch_idx, fSidebandLabelKey, net_sideband);
                if(!key_present){msg_error("calibration", "missing net_sideband label for channel "<< ch_idx << " with sky_freq: "<<sky_freq << eom); }
                key_present = chan_ax->RetrieveIndexLabelKeyValue(ch_idx, fBandwidthKey, bandwidth);
                if(!key_present){msg_error("calibration", "missing bandwidth label for channel "<< ch_idx << " with sky_freq: "<<sky_freq << eom);}

                //figure out the upper/lower frequency limits for this channel
                double lower_freq, upper_freq;
                std::size_t start_idx, ntones;
                DetermineChannelFrequencyLimits(sky_freq, bandwidth, net_sideband, lower_freq, upper_freq);

                //figure out the number of tones in this channel (better match stop-start)
                int c = std::floor(lower_freq/pcal_spacing); 
                if( (lower_freq - c*pcal_spacing) > 0 ){c += 1;} //first tone in channel is c*pcal_spacing
                int d = std::floor(upper_freq/pcal_spacing);
                if( (upper_freq - d*pcal_spacing) > 0 ){d += 1;} //d is first tone just beyond the channel

                if( (d - c) == (stop - start) ) //number of tones matches
                {
                    std::size_t ntones = stop - start;
                    for(std::size_t ti=0; ti < ntones; ti++)
                    {
                        //loop over the tone indexes and figure out the tone frequencies 
                        //which are multiples of the pcal spacing within the channel
                        pc_tone_ax->at(start+ti) = (c+ti)*pcal_spacing; 
                    }
                }
            }
        }

        //rescale all the phasors by the sample_period
        double sample_period = 1.0/(2.0*bandwidth*1e6);
        (*fPCData) *= sample_period;

    }
}

}//end of namespace
