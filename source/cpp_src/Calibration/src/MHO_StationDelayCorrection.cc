#include "MHO_StationDelayCorrection.hh"
#include <math.h>

namespace hops
{

MHO_StationDelayCorrection::MHO_StationDelayCorrection()
{
    fStationKey = "station";
    fRemStationKey = "remote_station";
    fRefStationKey = "reference_station";
    fRemStationMk4IDKey = "remote_station_mk4id";
    fRefStationMk4IDKey = "reference_station_mk4id";

    fSidebandLabelKey = "net_sideband";
    fLowerSideband = "L";
    fUpperSideband = "U";
    fStationIdentity = "";

    fImagUnit = MHO_Constants::imag_unit;
    fNanoSecToSecond = MHO_Constants::nanosec_to_second;
    fMHzToHz = MHO_Constants::MHz_to_Hz;
    fPi = MHO_Constants::pi;

    fRefFreq = 0.0;
    fDelayOffset = 0.0;
};

MHO_StationDelayCorrection::~MHO_StationDelayCorrection(){};

bool MHO_StationDelayCorrection::ExecuteInPlace(visibility_type* in)
{
    //loop over reference (0) and remote (1) stations
    for(std::size_t st_idx = 0; st_idx < 2; st_idx++)
    {
        if(IsApplicable(st_idx, in))
        {
            //loop over pol-products and apply phase rotation to the appropriate pol/channel/freq
            auto pp_ax = &(std::get< POLPROD_AXIS >(*in));
            auto chan_ax = &(std::get< CHANNEL_AXIS >(*in));
            std::string pp_label;
            for(std::size_t pp = 0; pp < pp_ax->GetSize(); pp++)
            {
                double delay = fDelayOffset * fNanoSecToSecond;
                for(std::size_t ch = 0; ch < chan_ax->GetSize(); ch++)
                {
                    // double bw = 0;
                    // bool has_bandwidth = chan_ax->RetrieveIndexLabelKeyValue(ch, "bandwidth", bw);
                    std::string net_sideband;
                    bool has_sideband = chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", net_sideband);

                    double chan_freq = chan_ax->at(ch);
                    //is this strictly correct?...this ignores slope across channel width
                    double deltaf = fMHzToHz * (chan_freq - fRefFreq);
                    //consider this alternative...where the channel mid-point (not edge) is the reference point
                    // double sb_sign = 1.0;
                    // if(net_sideband == "L"){sb_sign = -1.0;}
                    // double deltaf = fMHzToHz * ( (chan_freq + sb_sign*bw/2.0) - fRefFreq);
                    double theta = 2.0 * fPi * deltaf * delay;
                    std::complex< double > pc_phasor = std::exp(fImagUnit * theta);
                    //conjugate phases for LSB data, but not for USB
                    if(net_sideband == fLowerSideband)
                    {
                        pc_phasor = std::conj(pc_phasor);
                    } //conjugate phase for LSB data
                    if(st_idx == 0)
                    {
                        pc_phasor = std::conj(pc_phasor);
                    } //conjugate phase for reference station offset

                    //retrieve and multiply the appropriate sub view of the visibility array
                    auto chunk = in->SubView(pp, ch);
                    chunk *= pc_phasor;
                }
            }
        }
    }

    return true;
}

bool MHO_StationDelayCorrection::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}

bool MHO_StationDelayCorrection::IsApplicable(std::size_t st_idx, const visibility_type* in)
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
            msg_debug("calibration", "applying a station delay correction to station: " << val << eom);
            apply_correction = true;
        }
    }

    if(fStationIdentity.size() == 2) //selection by 2-char station code
    {
        in->Retrieve(station_code_key, val);
        if(fStationIdentity == val || fStationIdentity == "??")
        {
            msg_debug("calibration", "applying a station delay correction to station: " << val << eom);
            apply_correction = true;
        }
    }

    return apply_correction;
}

bool MHO_StationDelayCorrection::InitializeInPlace(visibility_type* /*in*/)
{
    return true;
}

bool MHO_StationDelayCorrection::InitializeOutOfPlace(const visibility_type* /*in*/, visibility_type* /*out*/)
{
    return true;
}

} // namespace hops
