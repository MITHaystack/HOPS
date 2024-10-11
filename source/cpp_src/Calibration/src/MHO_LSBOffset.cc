#include "MHO_LSBOffset.hh"

namespace hops
{

MHO_LSBOffset::MHO_LSBOffset()
{
    fStationKey = "station";
    fRemStationKey = "remote_station";
    fRefStationKey = "reference_station";
    fRemStationMk4IDKey = "remote_station_mk4id";
    fRefStationMk4IDKey = "reference_station_mk4id";
    fStationIdentity = "";

    fImagUnit = MHO_Constants::imag_unit;
    fDegToRad = MHO_Constants::deg_to_rad;

    fLSBPhaseOffset = 0.0;
};

MHO_LSBOffset::~MHO_LSBOffset(){};

bool MHO_LSBOffset::ExecuteInPlace(visibility_type* in)
{

    for(std::size_t st_idx = 0; st_idx < 2; st_idx++)
    {
        //loop over reference (0) and remote (1) stations
        visibility_element_type lsb_phasor = std::exp(fImagUnit * fLSBPhaseOffset * fDegToRad);
        if(st_idx == 1)
        {
            lsb_phasor = std::conj(lsb_phasor);
        } //legacy behavior is to conjugate for remote station

        if(IsApplicable(st_idx, in))
        {
            //loop over pol-products and apply pc-phases to the appropriate pol/channel
            auto pp_ax = &(std::get< POLPROD_AXIS >(*in));
            auto chan_ax = &(std::get< CHANNEL_AXIS >(*in));
            std::string pp_label;
            for(std::size_t pp = 0; pp < pp_ax->GetSize(); pp++)
            {
                //loop over the channels, locate LSB channels which are members of a double-sideband pair and apply phase offset
                for(std::size_t ch = 0; ch < chan_ax->GetSize(); ch++)
                {
                    std::string net_sideband;
                    int dsb_partner;
                    bool nsb_key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", net_sideband);
                    bool dsb_key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "dsb_partner", dsb_partner);
                    if(dsb_key_present && net_sideband == "L")
                    {
                        //retrieve and multiply the appropriate sub view of the visibility array
                        auto chunk = in->SubView(pp, ch);
                        chunk *= lsb_phasor;
                    }
                }
            }
        }
    }

    return true;
}

bool MHO_LSBOffset::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}

bool MHO_LSBOffset::IsApplicable(std::size_t st_idx, const visibility_type* in)
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

    if(fStationIdentity.size() > 2)
    {
        msg_error("calibration",
                  "station identiy: " << fStationIdentity << " is not a recognizable mark4 or 2-character code" << eom);
    }

    if(fStationIdentity.size() == 1) //selection by mk4 id
    {
        in->Retrieve(mk4id_key, val);
        if(fStationIdentity == val || fStationIdentity == "?")
        {
            apply_correction = true;
        }
    }

    if(fStationIdentity.size() == 2) //selection by 2-char station code
    {
        in->Retrieve(station_code_key, val);
        if(fStationIdentity == val || fStationIdentity == "??")
        {
            apply_correction = true;
        }
    }

    return apply_correction;
}

bool MHO_LSBOffset::InitializeInPlace(visibility_type* /*in*/)
{
    return true;
}

bool MHO_LSBOffset::InitializeOutOfPlace(const visibility_type* /*in*/, visibility_type* /*out*/)
{
    return true;
}

} // namespace hops
