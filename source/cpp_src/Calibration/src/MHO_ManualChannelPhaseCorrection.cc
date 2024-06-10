#include "MHO_ManualChannelPhaseCorrection.hh"


namespace hops
{


MHO_ManualChannelPhaseCorrection::MHO_ManualChannelPhaseCorrection()
{
    fStationKey = "station";
    fRemStationKey = "remote_station";
    fRefStationKey = "reference_station";
    fRemStationMk4IDKey = "remote_station_mk4id";
    fRefStationMk4IDKey = "reference_station_mk4id";
    fChannelLabelKey = "channel_label";
    fSidebandLabelKey = "net_sideband";
    fLowerSideband = "L";
    fUpperSideband = "U";

    fStationCode = "";
    fMk4ID = "";

    fImagUnit = MHO_Constants::imag_unit;
    fDegToRad = MHO_Constants::deg_to_rad;


};

MHO_ManualChannelPhaseCorrection::~MHO_ManualChannelPhaseCorrection(){};


bool
MHO_ManualChannelPhaseCorrection::ExecuteInPlace(visibility_type* in)
{
    std::size_t st_idx = DetermineStationIndex(in);
    if(st_idx != 0 && st_idx != 1){return false;}

    //loop over pol-products and apply pc-phases to the appropriate pol/channel
    auto pp_ax = &(std::get<POLPROD_AXIS>(*in) );
    auto chan_ax = &(std::get<CHANNEL_AXIS>(*in) );

    std::string chan_label;
    std::string pp_label;
    std::string pc_phase_key;
    std::string pol_code;
    for(std::size_t pp=0; pp < pp_ax->GetSize(); pp++)
    {
        pp_label = pp_ax->at(pp);
        if( PolMatch(st_idx, pp_label) )
        {
            pol_code = std::string(1, pp_label[st_idx] ); //get the polarization for the appropriate station (ref/rem)
            if(st_idx == 0){pc_phase_key = "ref_pcphase_";}
            if(st_idx == 1){pc_phase_key = "rem_pcphase_";}
            pc_phase_key += pol_code;
            for(auto pcal_it = fPCMap.begin(); pcal_it != fPCMap.end(); pcal_it++)
            {
                chan_label = pcal_it->first;
                double pc_val = pcal_it->second;
                auto idx_list = chan_ax->GetMatchingIndexes(fChannelLabelKey, chan_label);
                if(idx_list.size() == 1)
                {
                    std::size_t ch = idx_list[0];
                    std::string net_sideband = "?";
                    bool nsb_key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, fSidebandLabelKey, net_sideband);
                    visibility_element_type pc_phasor = std::exp( fImagUnit*pc_val*fDegToRad );

                    //conjugate phases for LSB data, but not for USB - TODO what about DSB?
                    #pragma message("TODO FIXME - test all manual pc phase correction cases (ref/rem/USB/LSB/DSB)")
                    #pragma message("TODO FIXME X2 - make sure we are not confusing ref/rem with USB/LSB signs.")
                    if(net_sideband == fLowerSideband){pc_phasor = std::conj(pc_phasor);}
                    if(st_idx == 0){pc_phasor = std::conj(pc_phasor);}

                    //retrieve and multiply the appropriate sub view of the visibility array
                    auto chunk = in->SubView(pp, ch);
                    chunk *= pc_phasor;

                    //now attach the manual pcal value to this channel/pol/station
                    //it would probably be better to stash this information in
                    //a new data type rather than attaching it as meta data here
                    chan_ax->InsertIndexLabelKeyValue(ch, pc_phase_key, pc_val*fDegToRad);
                }
            }
        }
    }
    return true;
}


bool
MHO_ManualChannelPhaseCorrection::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}


std::size_t
MHO_ManualChannelPhaseCorrection::DetermineStationIndex(const visibility_type* in)
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
    if(fStationCode == "??" || fMk4ID == "?"){return 1;}

    msg_warn("calibration", "manual per-channel phase correction, (remote,reference) " <<
        "stations: ("<<ref<<", "<<rem<<") do not match selection "<<fMk4ID<<"."<< eom );

    //msg_warn("calibration", "manual phase correction, remote/reference station do not match selection."<< eom );
    return 2;
}

bool
MHO_ManualChannelPhaseCorrection::PolMatch(std::size_t station_idx, std::string& polprod)
{
    make_upper(polprod);
    return (fPol[0] == polprod[station_idx]);
}


bool
MHO_ManualChannelPhaseCorrection::InitializeInPlace(visibility_type* /*in*/){ return true;}

bool
MHO_ManualChannelPhaseCorrection::InitializeOutOfPlace(const visibility_type* /*in*/, visibility_type* /*out*/){return true;}


}//end of namespace
