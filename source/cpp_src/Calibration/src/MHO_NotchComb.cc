#include "MHO_NotchComb.hh"

#include "MHO_MathUtilities.hh"

namespace hops
{

MHO_NotchComb::MHO_NotchComb()
{
    fBandwidthKey = "bandwidth";
    fSidebandLabelKey = "net_sideband";
    fUpperSideband = "U";
    fLowerSideband = "L";

    fNotchOffset = 0.0;
    fNotchPeriod = 0.0;
    fNotchWidth = 0.0;
};

MHO_NotchComb::~MHO_NotchComb(){};

bool MHO_NotchComb::ExecuteInPlace(visibility_type* in)
{
    if(fNotchPeriod <= 0.0 || fNotchWidth <= 0.0)
    {
        //no op
        return true;
    }

    auto chan_ax = &(std::get< CHANNEL_AXIS >(*in));
    auto freq_ax = &(std::get< FREQ_AXIS >(*in));

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

        //figure out the upper/lower frequency limits for this channel
        double lower_freq, upper_freq;
        MHO_MathUtilities::DetermineChannelFrequencyLimits(sky_freq, bandwidth, net_sideband, lower_freq, upper_freq);
        double sb = 1.0;
        if(net_sideband == fLowerSideband)
        {
            sb = -1.0;
        }

        // //loop over notches and spectral points and apply the filter inside this channel
        double count = 0;
        double npts = freq_ax->GetSize();

        //need to determine A <= x+n*P <= B
        //where A is the channel lower bound, B is the channel upper bound
        //x is the the notch offset, P is the notch period
        //so look for integers n, where (A-x)/P <= (B-x)/P
        //we intentionally have reverse the use of floor/ceil here, so we can catch
        //notches which might be outside of the channel, but due to a non-zero notch width
        //might overlap a portion of the channel
        int n_lower = std::floor((lower_freq - fNotchOffset) / fNotchPeriod);
        int n_upper = std::ceil((upper_freq - fNotchOffset) / fNotchPeriod);

        //determine how many notches should be applied in this channel
        int n_notches = n_upper - n_lower;

        for(int m = 0; m < n_notches; m++)
        {
            double notch_center = fNotchOffset + (n_lower + m) * fNotchPeriod;
            double notch_low = notch_center - fNotchWidth / 2.0;
            double notch_high = notch_center + fNotchWidth / 2.0;

            //check if the notch that is to be excluded is within/overlaps this channel
            double overlap[2];
            int n_inter = MHO_MathUtilities::FindIntersection< double >(lower_freq, upper_freq, notch_low, notch_high, overlap);

            if(n_inter)
            {
                for(std::size_t sp = 0; sp < freq_ax->GetSize(); sp++)
                {
                    //calculate the frequency of this point
                    double deltaf = ((*freq_ax)(sp));
                    double sp_freq = sky_freq + sb * deltaf; //TODO FIXME...CHECK THE SIGN PER USB/LSB
                    if(notch_low < sp_freq && sp_freq < notch_high)
                    {
                        count++;
                        //get a slice view for this spectral point across all APs, and zero it out
                        in->SliceView(":", ch, ":", sp) *= 0.0;
                    }
                }
            }
        }

        //re-scale the weights to account for the excised chunk
        //TODO FIXME...this needs to be done properly to get the correct integration-time and SNR
        double frac = (npts - count) / npts;
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

} // namespace hops
