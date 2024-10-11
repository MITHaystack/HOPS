#include "MHO_MixedPolYShift.hh"

namespace hops
{

MHO_MixedPolYShift::MHO_MixedPolYShift()
{
    fStationKey = "station";
    fRemStationKey = "remote_station";
    fRefStationKey = "reference_station";
    fRemStationMk4IDKey = "remote_station_mk4id";
    fRefStationMk4IDKey = "reference_station_mk4id";
    fStationIdentity = "";

    fImagUnit = MHO_Constants::imag_unit;
    fDegToRad = MHO_Constants::deg_to_rad;

    fYPolPhaseOffset = 90.0;
};

MHO_MixedPolYShift::~MHO_MixedPolYShift(){};

bool MHO_MixedPolYShift::ExecuteInPlace(visibility_type* in)
{
    for(std::size_t st_idx = 0; st_idx < 2; st_idx++)
    {
        //loop over reference (0) and remote (1) stations
        visibility_element_type shift_phasor = std::exp(fImagUnit * fYPolPhaseOffset * fDegToRad);
        if(st_idx == 1)
        {
            shift_phasor = std::conj(shift_phasor);
        } //legacy behavior is to conjugate for remote station

        if(IsApplicable(st_idx, in))
        {
            // //loop over pol-products and apply pc-phases to the appropriate pol/channel
            // auto pp_ax = &(std::get< POLPROD_AXIS >(*in));
            // auto chan_ax = &(std::get< CHANNEL_AXIS >(*in));
            // std::string pp_label;
            // for(std::size_t pp = 0; pp < pp_ax->GetSize(); pp++)
            // {
            //     //loop over the channel and apply phase offset (USB/LSB dependent)
            //     for(std::size_t ch = 0; ch < chan_ax->GetSize(); ch++)
            //     {
            //         std::string net_sideband;
            //         int dsb_partner;
            //         bool nsb_key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", net_sideband);
            //         bool dsb_key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "dsb_partner", dsb_partner);
            //         if(dsb_key_present && net_sideband == "L")
            //         {
            //             //retrieve and multiply the appropriate sub view of the visibility array
            //             auto chunk = in->SubView(pp, ch);
            //             chunk *= shift_phasor;
            //         }
            //     }
            // }
        }
    }

    return true;
}

bool MHO_MixedPolYShift::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}

bool MHO_MixedPolYShift::IsApplicable(std::size_t st_idx, const visibility_type* in)
{
    //from the c-code (precorrect.c)
    // if(param.mixed_mode_rot) //is mixed-mode extra 90 deg rotation turned on?
    // {
    //     //msg ("mixed mot rot feature is enabled", 1);
    //     //this is a mixed mode (circular pol. x linear pol.) experiment?
    //     if( pass->linpol[0] != pass->linpol[1] )
    //     {
    //         //msg ("mixed mot rot feature is active for this scan: linpol? %d and pol index? %d", 1, pass->linpol[stn], i);
    //         if(pass->linpol[stn] == 1 && i == 1 ) //apply the rotation to the lin-pol station only
    //         {
    //             msg ("adding -90 deg to static pc_offset for RY/YR data of station %c", 1, ovex->st[n].mk4_site_id);
    //             static_pc_off += -90.0;
    //         }
    //     }
    // }

    //first determine if the visibilities contain a mixed linear/circular polarization product
    auto pp_ax = &(std::get<POLPROD_AXIS>(*in));
    for(std::size_t pp=0; pp < pp_ax->GetSize(); pp++)
    {
        std::string polprod = pp_ax->at(pp);
        if(polprod.size() == 2 && IsMixedLinCirc(polprod) )
        {
            //ok, we have a mixed-linear/circular pol-product
            //now we have to check if one of the station pols is 'Y'
            std::string ref_pol = std::string(polprod[0],1);
            std::string rem_pol = std::string(polprod[1],1);
            if( ref_pol == "Y" ){return true;}
            if( rem_pol == "Y" ){return true;}
        }
    }

    return false;
}

bool MHO_MixedPolYShift::InitializeInPlace(visibility_type* /*in*/)
{
    return true;
}

bool MHO_MixedPolYShift::InitializeOutOfPlace(const visibility_type* /*in*/, visibility_type* /*out*/)
{
    return true;
}

bool MHO_MixedPolYShift::IsMixedLinCirc(std::string polprod) const
{
    bool has_circ = false;
    bool has_lin = false;
    //look for circular pol labels
    if(polprod.find("R") != std::string::npos){has_circ = true;}
    if(polprod.find("L") != std::string::npos){has_circ = true;}
    //look for linear pol labels...Y/X (H/V have never been test/used)
    if(polprod.find("Y") != std::string::npos){has_lin = true;}
    if(polprod.find("X") != std::string::npos){has_lin = true;}
    //if both are true, we have a mixed linear/circular pol-product
    return (has_circ && has_lin);
}



} // namespace hops
