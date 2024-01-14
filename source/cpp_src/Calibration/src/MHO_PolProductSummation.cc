#include "MHO_PolProductSummation.hh"


namespace hops
{


MHO_PolProductSummation::MHO_PolProductSummation()
{
    // fStationKey = "station";
    // fRemStationKey = "remote_station";
    // fRefStationKey = "reference_station";
    // fRemStationMk4IDKey = "remote_station_mk4id";
    // fRefStationMk4IDKey = "reference_station_mk4id";
    // fChannelLabelKey = "channel_label";
    // 
    // fStationCode = "";
    // fMk4ID = "";
    // 
    // fImagUnit = MHO_Constants::imag_unit;
    // fDegToRad = MHO_Constants::deg_to_rad;
    // 
    // fPhaseOffset = 0.0;
};

MHO_PolProductSummation::~MHO_PolProductSummation(){};


// bool
// MHO_PolProductSummation::ExecuteInPlace(visibility_type* in)
// {
    // std::size_t st_idx = DetermineStationIndex(in);
    // if(st_idx != 0 && st_idx != 1){return false;}
    // 
    // //loop over pol-products and apply pc-phases to the appropriate pol/channel
    // auto pp_ax = &(std::get<POLPROD_AXIS>(*in) );
    // std::string pp_label;
    // for(std::size_t pp=0; pp < pp_ax->GetSize(); pp++)
    // {
    //     pp_label = pp_ax->at(pp);
    //     if( PolMatch(st_idx, pp_label) )
    //     {
    //         visibility_element_type pc_phasor = std::exp( fImagUnit*fPhaseOffset*fDegToRad );
    // 
    //         //conjugate the phase for the reference station, but not remote?
    //         //should this behavior change depending on the USB/LSB?
    //         #pragma message("TODO FIXME - test all manual pc phase correction cases (ref/rem/USB/LSB/DSB)")
    //         if(st_idx == 1){pc_phasor = std::conj(pc_phasor);} //conjugate for remote but not reference station
    //         //retrieve and multiply the appropriate sub view of the visibility array
    //         auto chunk = in->SubView(pp);
    //         chunk *= pc_phasor;
    //     }
    // }
// 
//     return true;
// }


bool
MHO_PolProductSummation::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}


// std::size_t
// MHO_PolProductSummation::DetermineStationIndex(const visibility_type* in)
// {
//     //determine if the p-cal corrections are being applied to the remote or reference station
//     std::string val;
// 
//     if(fMk4ID != "") //selection by mk4 id
//     {
//         in->Retrieve(fRemStationMk4IDKey, val);
//         if(fMk4ID == val){return 1;}
//         in->Retrieve(fRefStationMk4IDKey, val);
//         if(fMk4ID == val){return 0;}
//     }
// 
//     if(fStationCode != "")//seletion by 2-char station code
//     {
//         in->Retrieve(fRemStationKey, val);
//         if(fStationCode == val){return 1;}
//         in->Retrieve(fRefStationKey, val);
//         if(fStationCode == val){return 0;}
//     }
// 
//     msg_warn("calibration", "manual pcal, remote/reference station do not match selection."<< eom );
//     return 2;
// }

// bool
// MHO_PolProductSummation::PolMatch(std::size_t station_idx, std::string& polprod)
// {
//     make_upper(polprod);
//     return (fPol[0] == polprod[station_idx]);
// }


bool
MHO_PolProductSummation::InitializeInPlace(visibility_type* /*in*/){ return true;}

bool
MHO_PolProductSummation::InitializeOutOfPlace(const visibility_type* /*in*/, visibility_type* /*out*/){return true;}


}//end of namespace
