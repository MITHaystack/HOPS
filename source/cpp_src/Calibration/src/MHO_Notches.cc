#include "MHO_Notches.hh"

#include "MHO_MathUtilities.hh"

namespace hops
{

MHO_Notches::MHO_Notches()
{
    fBandwidthKey = "bandwidth";
    fSidebandLabelKey = "net_sideband";
    fUpperSideband = "U";
    fLowerSideband = "L";
};

MHO_Notches::~MHO_Notches(){};

bool MHO_Notches::ExecuteInPlace(visibility_type* in)
{
    auto chan_ax = &(std::get< CHANNEL_AXIS >(*in));
    auto freq_ax = &(std::get< FREQ_AXIS >(*in));
    std::size_t npts = freq_ax->GetSize();

    //loop over all channels looking for the chunk to exclude
    for(std::size_t ch = 0; ch < chan_ax->GetSize(); ch++) //loop over all channels
    {
        //get channel's frequency info
        double sky_freq = (*chan_ax)(ch); //get the sky frequency of this channel
        double bandwidth = 0;
        std::string net_sideband;

        bool key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, fSidebandLabelKey, net_sideband);
        if(!key_present)
        {
            msg_error("calibration", "missing net_sideband label for channel " << ch << ", with sky_freq: " << sky_freq << eom);
        }
        key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, fBandwidthKey, bandwidth);
        if(!key_present)
        {
            msg_error("calibration", "missing bandwidth label for channel " << ch << ", with sky_freq: " << sky_freq << eom);
        }

        //flag the spectral points covered by any notch (overlapping notches are only counted once)
        std::vector< bool > flags(npts, false);
        for(std::size_t m = 0; m < fNotches.size(); m++)
        {
            double notch_low = fNotches[m].first;
            double notch_high = fNotches[m].second;
            if(notch_high <= notch_low)
            {
                msg_warn("calibration", "ignoring illegal notch: (" << notch_low << ", " << notch_high << ")" << eom);
                continue;
            }
            if(net_sideband == fLowerSideband)
            {
                FlagLSBNotch(notch_low, notch_high, sky_freq, bandwidth, flags);
            }
            else
            {
                FlagUSBNotch(notch_low, notch_high, sky_freq, bandwidth, flags);
            }
        }

        double count = 0;
        for(std::size_t sp = 0; sp < npts; sp++)
        {
            if(flags[sp])
            {
                count++;
                //get a slice view for this spectral point across all APs, and zero it out
                in->SliceView(":", ch, ":", sp) *= 0.0;
            }
        }

        //re-scale the weights to account for the excised chunk
        //TODO FIXME...this needs to be done properly to get the correct integration-time and SNR
        double frac = (npts - count) / (double)npts;
        double factor = 0.0;
        if(frac > 0.0)
        {
            factor = 1.0 / frac;
        }
        fWeights->SliceView(":", ch, ":", 0) *= factor;

        std::string ubf_key = "used_bandwidth_fraction";
        std::string rf_key = "rescaling_factor";
        chan_ax->InsertIndexLabelKeyValue(ch, ubf_key, frac);
        chan_ax->InsertIndexLabelKeyValue(ch, rf_key, factor);
    }

    return true;
}

//note: spectral point index 0 is at the DC edge
void MHO_Notches::FlagUSBNotch(double notch_low, double notch_high, double sky_freq, double bandwidth, std::vector< bool >& flags) const
{
    int npts = flags.size();
    double bottom = sky_freq;
    double top = sky_freq + bandwidth;
    int idx_bottom, idx_top;

    if(notch_low < bottom)
    {
        idx_bottom = -1;
    }
    else if(notch_low < top)
    {
        idx_bottom = (notch_low - bottom) / bandwidth * npts + 0.5;
    }
    else
    {
        idx_bottom = npts + 1;
    }

    if(notch_high < bottom)
    {
        idx_top = -1;
    }
    else if(notch_high < top)
    {
        idx_top = (notch_high - bottom) / bandwidth * npts + 0.5;
    }
    else
    {
        idx_top = npts + 1;
    }

    FlagIndexRange(idx_bottom, idx_top, flags);
}

void MHO_Notches::FlagLSBNotch(double notch_low, double notch_high, double sky_freq, double bandwidth, std::vector< bool >& flags) const
{
    int npts = flags.size();
    double top = sky_freq;
    double bottom = sky_freq - bandwidth;
    int idx_bottom, idx_top;

    if(notch_high < bottom)
    {
        idx_bottom = npts + 1;
    }
    else if(notch_high < top)
    {
        idx_bottom = (1.0 - (notch_high - bottom) / bandwidth) * npts + 0.5;
    }
    else
    {
        idx_bottom = -1;
    }

    if(notch_low < bottom)
    {
        idx_top = npts + 1;
    }
    else if(notch_low < top)
    {
        idx_top = (1.0 - (notch_low - bottom) / bandwidth) * npts + 0.5;
    }
    else
    {
        idx_top = -1;
    }

    FlagIndexRange(idx_bottom, idx_top, flags);
}

void MHO_Notches::FlagIndexRange(int idx_bottom, int idx_top, std::vector< bool >& flags) const
{
    int npts = flags.size();
    for(int i = 0; i < npts; i++)
    {
        if(i > idx_bottom && i < idx_top)
        {
            flags[i] = true;
        }
    }
}

} // namespace hops
