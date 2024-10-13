#include "MHO_MixedPolYShift.hh"

namespace hops
{

MHO_MixedPolYShift::MHO_MixedPolYShift()
{
    fRemStationKey = "remote_station";
    fRefStationKey = "reference_station";
    fImagUnit = MHO_Constants::imag_unit;
    fDegToRad = MHO_Constants::deg_to_rad;
    fYPolPhaseOffset = -90.0;
};

MHO_MixedPolYShift::~MHO_MixedPolYShift(){};

bool MHO_MixedPolYShift::ExecuteInPlace(visibility_type* in)
{
    if(in == nullptr)
    {
        return false;
    }

    auto pp_ax = &(std::get< POLPROD_AXIS >(*in));
    auto chan_ax = &(std::get< CHANNEL_AXIS >(*in));

    //loop over reference (0) and remote (1) stations
    for(std::size_t st_idx = 0; st_idx < 2; st_idx++)
    {
        std::string station;
        if(st_idx == 0)
        {
            in->Retrieve(fRefStationKey, station);
        }
        else
        {
            in->Retrieve(fRemStationKey, station);
        }

        //loop over available pol products
        for(std::size_t pp = 0; pp < pp_ax->GetSize(); pp++)
        {
            std::string polprod = pp_ax->at(pp);
            if(IsApplicable(st_idx, polprod)) //iff we have mixed pol, and this station is the linear pol station
            {
                //see c-code precorrect.c file for original implementation
                visibility_element_type shift_phasor = std::exp(fImagUnit * fYPolPhaseOffset * fDegToRad);

                if(st_idx == 0)
                {
                    shift_phasor = std::conj(shift_phasor);
                } //conjugate for refrence station

                msg_debug("calibration", "adding -90 deg static pc_offset for RY/YR data of station: " << station << eom);

                //TODO...we could attach info about this operator being applied to this entry on the pol-product axis

                //loop over the channels and apply the phase offset
                //we need to do this on a per-channel basis in case we have mixed USB/LSB data (there is a sign flip between the two)
                for(std::size_t ch = 0; ch < chan_ax->GetSize(); ch++)
                {
                    std::string net_sideband = "?";
                    bool nsb_key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", net_sideband);
                    //conjugate phases for LSB data, but not for USB
                    visibility_element_type applied_phasor = shift_phasor;
                    if(net_sideband == "L")
                    {
                        applied_phasor = std::conj(shift_phasor);
                    }

                    //retrieve and multiply the appropriate sub view of the visibility array
                    auto chunk = in->SubView(pp, ch);
                    chunk *= applied_phasor;
                }
            }
        }
    }

    return true;
}

bool MHO_MixedPolYShift::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}

bool MHO_MixedPolYShift::IsApplicable(std::size_t st_idx, std::string polprod)
{
    //determine if this pol-product is a mixed linear/circular polarization product
    if(polprod.size() == 2 && IsMixedLinCirc(polprod))
    {
        //ok, we have a mixed-linear/circular pol-product
        //now we have to check if one of the station pols is 'Y'
        std::string ref_pol = std::string(1, polprod[0]);
        std::string rem_pol = std::string(1, polprod[1]);
        if(ref_pol == "Y" && st_idx == 0)
        {
            return true;
        } //apply to reference station
        if(rem_pol == "Y" && st_idx == 1)
        {
            return true;
        } //apply to remote station
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
    if(polprod.find("R") != std::string::npos)
    {
        has_circ = true;
    }
    if(polprod.find("L") != std::string::npos)
    {
        has_circ = true;
    } //LCP has never been tested!
    //look for linear pol labels...Y/X (H/V have never been tested/used)
    if(polprod.find("Y") != std::string::npos)
    {
        has_lin = true;
    }
    if(polprod.find("X") != std::string::npos)
    {
        has_lin = true;
    }
    //if both are true, we have a mixed linear/circular pol-product
    return (has_circ && has_lin);
}

} // namespace hops
