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

    fStationCode = "";
    fMk4ID = "";

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
    std::size_t st_idx = DetermineStationIndex(in);

    std::cout<<"man delay op, st_idx = "<<st_idx<<" my pol = "<<fPol<<std::endl;

    if(st_idx != 0 && st_idx != 1){return false;}

    //loop over pol-products and apply pc-phases to the appropriate pol/channel
    auto pp_ax = &(std::get<POLPROD_AXIS>(*in) );
    auto chan_ax = &(std::get<CHANNEL_AXIS>(*in) );
    auto freq_ax = &(std::get<FREQ_AXIS>(*in) );
    
    std::string chan_label;
    std::string pp_label;
    
    for(std::size_t pp=0; pp < pp_ax->GetSize(); pp++)
    {
        pp_label = pp_ax->at(pp);
        if( PolMatch(st_idx, pp_label) )
        {
            std::cout<<"POL MATCH = "<<pp_label<<" st idx = "<<st_idx<<std::endl;
            for(auto pcal_it = fPCDelayMap.begin(); pcal_it != fPCDelayMap.end(); pcal_it++)
            {
                chan_label = pcal_it->first;
                double delay = pcal_it->second;
                //TODO, may need to re-work this mapping method if too slow
                const MHO_IntervalLabel* ilabel = chan_ax->GetFirstIntervalWithKeyValue(fChannelLabelKey, chan_label);
                if(ilabel != nullptr)
                {
                    std::size_t ch = ilabel->GetLowerBound();
                    
                    //get the channels bandwidth to determine effective sampling period
                    double bandwidth = 0;
                    double eff_sample_period = 0;
                    auto other_labels = chan_ax->GetIntervalsWhichIntersect(ch);
                    for(auto ol_it = other_labels.begin(); ol_it != other_labels.end(); ol_it++)
                    {
                        if((*ol_it)->HasKey(fBandwidthKey))
                        {
                            bool ok = (*ol_it)->Retrieve(std::string("bandwidth"), bandwidth);
                            if(ok)
                            {
                                bandwidth *= fMHzToHz;
                                eff_sample_period = 1.0/(2.0*bandwidth);
                                std::cout<<"CHANNEL BANDWIDTH = "<<bandwidth<<" ok = "<<ok<<std::endl;
                                std::cout<<"EFFECTIVE SAMPLE PERIOD = "<<eff_sample_period<<std::endl;
                                break;
                            }
                        }
                    }



                    //loop over spectral points calculating the phase correction from this delay at each point
                    std::size_t nsp = freq_ax->GetSize(); 
                    for(std::size_t sp=0; sp < nsp; sp++)
                    {
                        double deltaf = freq_ax->at(sp)*fMHzToHz; //-2e-3 * i / (2e6 * param->samp_period * nlags);
                        double theta = -2.0*fPi*deltaf*delay*fNanoSecToSecond;
                        
                        #pragma message("TODO FIXME -- geodetic phase shift treatment needs implementation (see normfx. line 398)" )
                        double phase_shift = -2.0*fPi*(1.0/4.0)*delay*fNanoSecToSecond/eff_sample_period; 
                        phase_shift *=  -( (double)(2*nsp) - 2.0) / (double)(2*nsp); //factor of 2 is from the way normfx zero-pads the data
                        theta += phase_shift;
                        
                        std::cout<<"FACTOR = "<<-( (double)(nsp) - 2.0) / (double)(nsp)<<std::endl;
                        // 
                        // 
                        // - 1e-3 * delay / (4e6 * param->samp_period);
                        // phase_shift *= - (double)(nlags - 2) / (double)(nlags);
                        // 
                        // 
                        
                        
                        std::cout<<"ch: "<<chan_label<<" sp = "<<sp<<" deltaf = "<<deltaf<<" delay = "<<delay<<" theta = "<<theta<<std::endl;
                        std::cout<<"phase shift = "<<phase_shift<<std::endl;
                        visibility_element_type pc_phasor = std::exp( fImagUnit*theta );
                        
                        auto chunk = in->SliceView(pp, ch, ":", sp); //select this spectral point across all APs
                        
                        #pragma message("TODO FIXME - pc delays_offs does not correct for the per-channel phase offset (see normfx.c)")
                        if(st_idx == 1){pc_phasor = std::conj(pc_phasor);} //conjugate for remote but not reference station
                        //retrieve and multiply the appropriate sub view of the visibility array

                        chunk *= pc_phasor;
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


std::size_t
MHO_ManualChannelDelayCorrection::DetermineStationIndex(const visibility_type* in)
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
MHO_ManualChannelDelayCorrection::PolMatch(std::size_t station_idx, std::string& polprod)
{
    make_upper(polprod);
    return (fPol[0] == polprod[station_idx]);
}


bool
MHO_ManualChannelDelayCorrection::InitializeInPlace(visibility_type* /*in*/){ return true;}

bool
MHO_ManualChannelDelayCorrection::InitializeOutOfPlace(const visibility_type* /*in*/, visibility_type* /*out*/){return true;}


}//end of namespace
