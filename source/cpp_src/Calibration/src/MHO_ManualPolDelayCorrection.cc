#include "MHO_ManualPolDelayCorrection.hh"


namespace hops
{


MHO_ManualPolDelayCorrection::MHO_ManualPolDelayCorrection()
{
    fStationKey = "station";
    fRemStationKey = "remote_station";
    fRefStationKey = "reference_station";
    fRemStationMk4IDKey = "remote_station_mk4id";
    fRefStationMk4IDKey = "reference_station_mk4id";

    fSidebandLabelKey = "net_sideband";
    fLowerSideband = "L";
    fUpperSideband = "U";

    fStationCode = "";
    fMk4ID = "";

    fImagUnit = MHO_Constants::imag_unit;
    fNanoSecToSecond = MHO_Constants::nanosec_to_second;
    fMHzToHz = MHO_Constants::MHz_to_Hz;
    fPi = MHO_Constants::pi;

    fRefFreq = 0.0;
    fDelayOffset = 0.0;
};

MHO_ManualPolDelayCorrection::~MHO_ManualPolDelayCorrection(){};


bool
MHO_ManualPolDelayCorrection::ExecuteInPlace(visibility_type* in)
{
    std::size_t st_idx = DetermineStationIndex(in);
    if(st_idx != 0 && st_idx != 1){return false;}

    //loop over pol-products and apply pc-phases to the appropriate pol/channel/freq
    auto pp_ax = &(std::get<POLPROD_AXIS>(*in) );
    auto chan_ax = &(std::get<CHANNEL_AXIS>(*in) );
    std::string pp_label;
    for(std::size_t pp=0; pp < pp_ax->GetSize(); pp++)
    {
        double delay = fDelayOffset*fNanoSecToSecond;
        pp_label = pp_ax->at(pp);
        if( PolMatch(st_idx, pp_label) )
        {
            std::string delay_offset_key;
            std::string pol_code = std::string(1, pp_label[st_idx] ); //get the polarization for the appropriate station (ref/rem)
            if(st_idx == 0){delay_offset_key = "ref_delayoff_";}
            if(st_idx == 1){delay_offset_key = "rem_delayoff_";}
            delay_offset_key += pol_code;

            //now attach the manual delay offset value to this pol/station
            //it may be better to stash this information in a new data type 
            //rather than attaching it as meta data here...
            //also, if multiple delay offsets are applied, this will only capture the last one 
            pp_ax->InsertIndexLabelKeyValue(pp, delay_offset_key, fDelayOffset); //store as ns

            for(std::size_t ch=0; ch<chan_ax->GetSize(); ch++)
            {
                double chan_freq = chan_ax->at(ch);
                double deltaf = fMHzToHz*(chan_freq - fRefFreq); //is this strictly correct?...this ignores slope across channel width
                double theta = 2.0*fPi*deltaf*delay;

                visibility_element_type pc_phasor = std::exp( fImagUnit*theta );
                
                // std::string net_sideband = "?";
                // bool nsb_key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, fSidebandLabelKey, net_sideband);
                // //conjugate phases for LSB data, but not for USB - TODO what about DSB?
                // if(net_sideband == fLowerSideband){pc_phasor = std::conj(pc_phasor);} //conjugate phase for LSB data
                // if(st_idx == 0){pc_phasor = std::conj(pc_phasor);} //conjugate phase for reference station offset

                //first impl behavior...working for EHT test case, but not checked everywhere
                if(st_idx == 1){pc_phasor = std::conj(pc_phasor);} //conjugate for remote but not reference station

                //retrieve and multiply the appropriate sub view of the visibility array
                auto chunk = in->SubView(pp, ch);
                chunk *= pc_phasor;
            }
        }
    }

    return true;
}


bool
MHO_ManualPolDelayCorrection::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}


std::size_t
MHO_ManualPolDelayCorrection::DetermineStationIndex(const visibility_type* in)
{
    //determine if the p-cal corrections are being applied to the remote or reference station
    std::string val;
    std::string rem, ref;

    if(fMk4ID != "") //selection by mk4 id
    {
        in->Retrieve(fRemStationMk4IDKey, rem);
        in->Retrieve(fRefStationMk4IDKey, ref);
        if(fMk4ID == rem){return 1;}
        if(fMk4ID == ref){return 0;}
    }

    if(fStationCode != "")//seletion by 2-char station code
    {
        in->Retrieve(fRemStationKey, val);
        if(fStationCode == val){return 1;}
        in->Retrieve(fRefStationKey, val);
        if(fStationCode == val){return 0;}
    }

    //wildcard, it doesn't matter, so just return as rem station
    if(fStationCode == "??" || fMk4ID == "?"){return 0;}

    msg_warn("calibration", "manual per-pol delay correction, (remote,reference) " <<
        "stations: ("<<ref<<", "<<rem<<") do not match selection "<<fMk4ID<<"."<< eom );
    return 2;
}

bool
MHO_ManualPolDelayCorrection::PolMatch(std::size_t station_idx, std::string& polprod)
{
    make_upper(polprod);
    return (fPol[0] == polprod[station_idx]);
}


bool
MHO_ManualPolDelayCorrection::InitializeInPlace(visibility_type* /*in*/){ return true;}

bool
MHO_ManualPolDelayCorrection::InitializeOutOfPlace(const visibility_type* /*in*/, visibility_type* /*out*/){return true;}


}//end of namespace
