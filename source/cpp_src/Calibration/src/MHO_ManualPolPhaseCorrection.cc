#include "MHO_ManualPolPhaseCorrection.hh"


namespace hops
{


MHO_ManualPolPhaseCorrection::MHO_ManualPolPhaseCorrection()
{
    fStationKey = "station";
    fRemStationKey = "remote_station";
    fRefStationKey = "reference_station";
    fRemStationMk4IDKey = "remote_station_mk4id";
    fRefStationMk4IDKey = "reference_station_mk4id";
    fChannelLabelKey = "channel_label";

    fStationCode = "";
    fMk4ID = "";

    fImagUnit = MHO_Constants::imag_unit;
    fDegToRad = MHO_Constants::deg_to_rad;

    fPhaseOffset = 0.0;
};

MHO_ManualPolPhaseCorrection::~MHO_ManualPolPhaseCorrection(){};


bool
MHO_ManualPolPhaseCorrection::ExecuteInPlace(visibility_type* in)
{
    std::size_t st_idx = DetermineStationIndex(in);
    if(st_idx != 0 && st_idx != 1){return false;}

    //loop over pol-products and apply pc-phases to the appropriate pol/channel
    auto pp_ax = &(std::get<POLPROD_AXIS>(*in) );
    std::string pp_label;
    for(std::size_t pp=0; pp < pp_ax->GetSize(); pp++)
    {
        pp_label = pp_ax->at(pp);
        if( PolMatch(st_idx, pp_label) )
        {
            visibility_element_type pc_phasor = std::exp( fImagUnit*fPhaseOffset*fDegToRad );

            //conjugate the phase for the reference station, but not remote?
            //should this behavior change depending on the USB/LSB?
            #pragma message("TODO FIXME - test all manual pc phase correction cases (ref/rem/USB/LSB/DSB)")
            if(st_idx == 1){pc_phasor = std::conj(pc_phasor);} //conjugate for remote but not reference station
            //retrieve and multiply the appropriate sub view of the visibility array
            auto chunk = in->SubView(pp);
            chunk *= pc_phasor;
        }
    }

    return true;
}


bool
MHO_ManualPolPhaseCorrection::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}


std::size_t
MHO_ManualPolPhaseCorrection::DetermineStationIndex(const visibility_type* in)
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

    msg_warn("calibration", "manual per-pol phase correction, (remote,reference) " <<
        "stations: ("<<ref<<", "<<rem<<") do not match selection "<<fMk4ID<<"."<< eom );

    //msg_warn("calibration", "manual per-pol phase correction, remote/reference station do not match selection."<< eom );
    return 2;
}

bool
MHO_ManualPolPhaseCorrection::PolMatch(std::size_t station_idx, std::string& polprod)
{
    make_upper(polprod);
    return (fPol[0] == polprod[station_idx]);
}


bool
MHO_ManualPolPhaseCorrection::InitializeInPlace(visibility_type* /*in*/){ return true;}

bool
MHO_ManualPolPhaseCorrection::InitializeOutOfPlace(const visibility_type* /*in*/, visibility_type* /*out*/){return true;}


}//end of namespace
