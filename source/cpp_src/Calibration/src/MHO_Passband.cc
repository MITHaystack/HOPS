#include "MHO_Passband.hh"

#include "MHO_MathUtilities.hh"


namespace hops
{


MHO_Passband::MHO_Passband()
{
    fIsExclusion = true;
    fLow = 0.0;
    fHigh = 0.0;

    fBandwidthKey = "bandwidth";
    fSidebandLabelKey = "net_sideband";
    fUpperSideband = "U";
    fLowerSideband = "L";
};

MHO_Passband::~MHO_Passband(){};


bool
MHO_Passband::ExecuteInPlace(visibility_type* in)
{
    auto chan_ax = &(std::get<CHANNEL_AXIS>(*in) );
    auto wchan_ax = &(std::get<CHANNEL_AXIS>(*fWeights) );
    auto freq_ax = &(std::get<FREQ_AXIS>(*in) );

    if(fIsExclusion)
    {
        //loop over all channels looking for the chunk to exclude
        for(std::size_t ch=0; ch < chan_ax->GetSize(); ch++) //loop over all channels
        {
            //get channel's frequency info
            double sky_freq = (*chan_ax)(ch); //get the sky frequency of this channel
            double bandwidth = 0;
            std::string net_sideband;

            bool key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, fSidebandLabelKey, net_sideband);
            if(!key_present){msg_error("calibration", "missing net_sideband label for channel "<< ch << ", with sky_freq: "<<sky_freq << eom); }
            key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, fBandwidthKey, bandwidth);
            if(!key_present){msg_error("calibration", "missing bandwidth label for channel "<< ch << ", with sky_freq: "<<sky_freq << eom);}

            //figure out the upper/lower frequency limits for this channel
            double lower_freq, upper_freq;
            MHO_MathUtilities::DetermineChannelFrequencyLimits(sky_freq, bandwidth, net_sideband, lower_freq, upper_freq);
            double sb = 1.0;
            if(net_sideband == fLowerSideband){sb = -1.0;}

            //check if the passband that is to be excluded is within/overlaps this channel
            double overlap[2];
            int n_inter = MHO_MathUtilities::FindIntersection<double>(lower_freq, upper_freq, fLow, fHigh, overlap);

            if(n_inter)
            {
                //loop over spectral points and apply the pass-band inside this channel
                double count = 0;
                double npts = freq_ax->GetSize();
                for(std::size_t sp=0; sp < freq_ax->GetSize(); sp++)
                {
                    //calculate the frequency of this point
                    double deltaf = ( (*freq_ax)(sp) );
                    double sp_freq = sky_freq + sb*deltaf; //TODO FIXME...CHECK THE SIGN PER USB/LSB
                    if(fLow < sp_freq && sp_freq < fHigh)
                    {
                        count++;
                        //get a slice view for this spectral point across all APs, and zero it out
                        in->SliceView(":", ch, ":", sp) *= 0.0;
                    }
                }
                
                //re-scale the weights to account for the excised chunk 
                //TODO FIXME...this needs to be done properly to get the correct integration-time and SNR
                double frac = (npts-count)/npts;
                double factor = 0.0;
                if(frac > 0.0){factor = 1.0/frac;}
                fWeights->SliceView(":", ch, ":", 0) *= factor;
                
                std::string ubf_key = "used_bandwidth_fraction";
                std::string rf_key = "rescaling_factor";
                chan_ax->InsertIndexLabelKeyValue(ch, ubf_key, frac);
                wchan_ax->InsertIndexLabelKeyValue(ch, rf_key, factor);
            }
        }
    }
    else
    {
        //loop over all channels, cutting everything that is not in the 'inclusion'
        for(std::size_t ch=0; ch < chan_ax->GetSize(); ch++) //loop over all channels
        {
            //get channel's frequency info
            double sky_freq = (*chan_ax)(ch); //get the sky frequency of this channel
            double bandwidth = 0;
            std::string net_sideband;

            bool key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, fSidebandLabelKey, net_sideband);
            if(!key_present){msg_error("calibration", "missing net_sideband label for channel "<< ch << ", with sky_freq: "<<sky_freq << eom); }
            key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, fBandwidthKey, bandwidth);
            if(!key_present){msg_error("calibration", "missing bandwidth label for channel "<< ch << ", with sky_freq: "<<sky_freq << eom);}

            //figure out the upper/lower frequency limits for this channel
            double lower_freq, upper_freq;
            MHO_MathUtilities::DetermineChannelFrequencyLimits(sky_freq, bandwidth, net_sideband, lower_freq, upper_freq);
            double sb = 1.0;
            if(net_sideband == fLowerSideband){sb = -1.0;}
            
            //check if the passband that is to be excluded is within/overlaps this channel
            double overlap[2];
            int n_inter = MHO_MathUtilities::FindIntersection<double>(lower_freq, upper_freq, fLow, fHigh, overlap);

            if(n_inter)
            {
                //loop over spectral points and determine what is excluded inside this channel
                double count = 0;
                double npts = freq_ax->GetSize();
                for(std::size_t sp=0; sp < freq_ax->GetSize(); sp++)
                {
                    //calculate the frequency of this point
                    double deltaf = ( (*freq_ax)(sp) );
                    double sp_freq = sky_freq + sb*deltaf; //TODO FIXME...CHECK THE SIGN PER USB/LSB
                    if(sp_freq < fLow || sp_freq > fHigh)
                    {
                        count++;
                        //get a slice view for this spectral point across all APs, and zero it out
                        in->SliceView(":", ch, ":", sp) *= 0.0;
                    }
                }
                //re-scale the weights to account for the excised chunk 
                double frac = (npts-count)/npts;
                //TODO FIXME...this needs to be done properly to get the correct integration-time and SNR
                double factor = 0.0;
                if(frac > 0.0){factor = 1.0/frac;}
                fWeights->SliceView(":", ch, ":", 0) *= factor;
                
                std::string ubf_key = "used_bandwidth_fraction";
                std::string rf_key = "rescaling_factor";
                chan_ax->InsertIndexLabelKeyValue(ch, ubf_key, frac);
                wchan_ax->InsertIndexLabelKeyValue(ch, rf_key, factor);
            }
            else 
            {
                //no intersection with the 'inclusion', so zero out this whole channel 
                in->SliceView(":", ch, ":", ":") *= 0.0;
                
                //rescale the weights by zero, since this was cut entirely
                fWeights->SliceView(":", ch, ":", ":") *= 0.0;
                
                std::string ubf_key = "used_bandwidth_fraction";
                std::string rf_key = "rescaling_factor";
                chan_ax->InsertIndexLabelKeyValue(ch, ubf_key, 0.0);
                wchan_ax->InsertIndexLabelKeyValue(ch, rf_key, 0.0);
            }
        }
    }

    return true;
}


bool
MHO_Passband::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}


bool
MHO_Passband::InitializeInPlace(visibility_type* /*in*/){ return true;}

bool
MHO_Passband::InitializeOutOfPlace(const visibility_type* /*in*/, visibility_type* /*out*/){return true;}

}//end of namespace
