#include "MHO_ManualChannelDelayCorrection.hh"


namespace hops
{


MHO_ManualChannelDelayCorrection::MHO_ManualChannelDelayCorrection()
{
    fStationKey = "station";
    fRemStationKey = "remote_station";
    fRefStationKey = "reference_station";
    fRemStationMk4IDKey = "remote_station_mk4id";
    fRefStationMk4IDKey = "reference_station_mk4id";
    fChannelLabelKey = "channel_label";
    fBandwidthKey = "bandwidth";
    
    fSidebandLabelKey = "net_sideband";
    fLowerSideband = "L";
    fUpperSideband = "U";
    fStationIdentity = "";

    fImagUnit = MHO_Constants::imag_unit;
    fDegToRad = MHO_Constants::deg_to_rad;
    fNanoSecToSecond = MHO_Constants::nanosec_to_second;
    fMHzToHz = MHO_Constants::MHz_to_Hz;
    fPi = MHO_Constants::pi;

};

MHO_ManualChannelDelayCorrection::~MHO_ManualChannelDelayCorrection(){};


bool
MHO_ManualChannelDelayCorrection::ExecuteInPlace(visibility_type* in)
{
    //loop over reference (0) and remote (1) stations
    for(std::size_t st_idx = 0; st_idx < 2; st_idx++)
    {
        if(IsApplicable(st_idx,in))
        {

            //loop over pol-products and apply pc-phases to the appropriate pol/channel
            auto pp_ax = &(std::get<POLPROD_AXIS>(*in) );
            auto chan_ax = &(std::get<CHANNEL_AXIS>(*in) );
            auto freq_ax = &(std::get<FREQ_AXIS>(*in) );

            std::string chan_label;
            std::string pp_label;
            std::string bwkey = "bandwidth";

            for(std::size_t pp=0; pp < pp_ax->GetSize(); pp++)
            {
                pp_label = pp_ax->at(pp);
                if( PolMatch(st_idx, pp_label) )
                {
                    for(auto pcal_it = fPCDelayMap.begin(); pcal_it != fPCDelayMap.end(); pcal_it++)
                    {
                        chan_label = pcal_it->first;
                        double delay = pcal_it->second;

                        auto idx_list = chan_ax->GetMatchingIndexes(fChannelLabelKey, chan_label);
                        if(idx_list.size() == 1)
                        {
                            std::size_t ch = idx_list[0];
                            double bandwidth = 0;
                            bool bw_key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, bwkey, bandwidth);
                            
                            std::string delay_offset_key;
                            std::string pol_code = std::string(1, pp_label[st_idx] ); //get the polarization for the appropriate station (ref/rem)
                            if(st_idx == 0){delay_offset_key = "ref_delayoff_";}
                            if(st_idx == 1){delay_offset_key = "rem_delayoff_";}
                            delay_offset_key += pol_code;

                            //now attach the manual delay offset value to this pol/station
                            //it may be better to stash this information in a new data type 
                            //rather than attaching it as meta data here...
                            //also, if multiple delay offsets are applied, this will only capture the last one 
                            chan_ax->InsertIndexLabelKeyValue(ch, delay_offset_key, delay); //store as ns

                            if( bw_key_present )
                            {
                                //get the channels bandwidth to determine effective sampling period
                                bool ok = false;
                                //calculate effective sampling period for channel assuming Nyquist rate
                                bandwidth *= fMHzToHz;
                                double eff_sample_period = 1.0/(2.0*bandwidth);

                                //loop over spectral points calculating the phase correction from this delay at each point
                                std::size_t nsp = freq_ax->GetSize();
                                for(std::size_t sp=0; sp < nsp; sp++)
                                {
                                    double deltaf = freq_ax->at(sp)*fMHzToHz; //-2e-3 * i / (2e6 * param->samp_period * nlags);
                                    double theta = -2.0*fPi*deltaf*delay*fNanoSecToSecond;

                                    TODO_FIXME_MSG("TODO FIXME -- geodetic phase shift treatment needs implementation (see normfx. line 398)" )
                                    double phase_shift = -2.0*fPi*(1.0/4.0)*delay*fNanoSecToSecond/eff_sample_period; //where does factor of 1/4 come from (see normfx)
                                    phase_shift *=  -( (double)(2*nsp) - 2.0) / (double)(2*nsp); //factor of 2 is from the way normfx zero-pads the data
                                    theta += phase_shift;

                                    visibility_element_type pc_phasor = std::exp( fImagUnit*theta );
                                    
                                    // std::string net_sideband = "?";
                                    // bool nsb_key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, fSidebandLabelKey, net_sideband);
                                    // //conjugate phases for LSB data, but not for USB - TODO what about DSB?
                                    // if(net_sideband == fLowerSideband){pc_phasor = std::conj(pc_phasor);} //conjugate phase for LSB data
                                    // if(st_idx == 0){pc_phasor = std::conj(pc_phasor);} //conjugate phase for reference station offset

                                    //first impl behavior...working for EHT test case, but not checked everywhere
                                    if(st_idx == 1){pc_phasor = std::conj(pc_phasor);} //conjugate for remote but not reference station

                                    //retrieve and multiply the appropriate sub view of the visibility array
                                    auto chunk = in->SliceView(pp, ch, ":", sp); //select this spectral point (for this pol/channel) across all APs
                                    chunk *= pc_phasor;
                                }
                            }
                            else
                            {
                                msg_error("calibration", "channel: "<<chan_label<<" is missing bandwidth tag."<<eom);
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}


bool
MHO_ManualChannelDelayCorrection::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}


bool 
MHO_ManualChannelDelayCorrection::IsApplicable(std::size_t st_idx, const visibility_type* in)
{
    bool apply_correction = false;
    std::string val;
    std::string mk4id_key;
    std::string station_code_key;

    if(st_idx == 0)
    {
        mk4id_key = fRefStationMk4IDKey;
        station_code_key = fRefStationKey;
    }
    else
    {
        mk4id_key = fRemStationMk4IDKey;
        station_code_key = fRemStationKey;
    }

    if(fStationIdentity.size() == 1) //selection by mk4 id
    {
        in->Retrieve(mk4id_key, val);
        if(fStationIdentity == val || fStationIdentity == "?"){apply_correction = true;}
    }

    if(fStationIdentity.size() == 2)//selection by 2-char station code
    {
        in->Retrieve(station_code_key, val);
        if(fStationIdentity == val || fStationIdentity == "??"){apply_correction = true;}
    }

    return apply_correction;
}

bool
MHO_ManualChannelDelayCorrection::PolMatch(std::size_t station_idx, std::string& polprod)
{
    if(fPol == "?"){return true;} //wild card allows for the implementation of delay_offs (with no pol-specification)
    make_upper(polprod);
    return (fPol[0] == polprod[station_idx]);
}


bool
MHO_ManualChannelDelayCorrection::InitializeInPlace(visibility_type* /*in*/){ return true;}

bool
MHO_ManualChannelDelayCorrection::InitializeOutOfPlace(const visibility_type* /*in*/, visibility_type* /*out*/){return true;}


}//end of namespace
