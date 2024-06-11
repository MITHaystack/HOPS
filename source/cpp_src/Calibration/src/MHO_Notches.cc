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


bool
MHO_Notches::ExecuteInPlace(visibility_type* in)
{
    auto chan_ax = &(std::get<CHANNEL_AXIS>(*in) );
    auto freq_ax = &(std::get<FREQ_AXIS>(*in) );

    //loop over all channels looking for the chunk to exclude
    for(std::size_t ch=0; ch < chan_ax->GetSize(); ch++) //loop over all channels
    {
        //loop over notches and spectral points and apply the filter inside this channel
        double count = 0;
        double npts = freq_ax->GetSize();
        
        for(std::size_t m=0; m<fNotches.size(); m++)
        {
            double notch_low = fNotches[m].first;
            double notch_high = fNotches[m].second;

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
            DetermineChannelFrequencyLimits(sky_freq, bandwidth, net_sideband, lower_freq, upper_freq);
            double sb = 1.0;
            if(net_sideband == fLowerSideband){sb = -1.0;}

            //check if the notch that is to be excluded is within/overlaps this channel
            double overlap[2];
            int n_inter = MHO_MathUtilities::FindIntersection(lower_freq, upper_freq, notch_low, notch_high, overlap);

            if(n_inter)
            {
                for(std::size_t sp=0; sp < freq_ax->GetSize(); sp++)
                {
                    //calculate the frequency of this point
                    double deltaf = ( (*freq_ax)(sp) );
                    double sp_freq = sky_freq + sb*deltaf; //TODO FIXME...CHECK THE SIGN PER USB/LSB
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
        double frac = (npts-count)/npts;
        double factor = 0.0;
        if(frac != 0.0){factor = 1.0/frac;}
        fWeights->SliceView(":",ch,":",0) *= factor;
        //fWeights->SliceView(pp,ch,":",0) *= frac;
    }

    return true;
}


bool
MHO_Notches::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}


bool
MHO_Notches::InitializeInPlace(visibility_type* /*in*/){ return true;}

bool
MHO_Notches::InitializeOutOfPlace(const visibility_type* /*in*/, visibility_type* /*out*/){return true;}



void
MHO_Notches::DetermineChannelFrequencyLimits(double sky_freq, double bandwidth, std::string net_sideband, double& lower_freq, double& upper_freq)
{
    if(net_sideband == fUpperSideband)
    {
        lower_freq = sky_freq;
        upper_freq = sky_freq + bandwidth;
    }
    else //lower sideband
    {
        upper_freq = sky_freq;
        lower_freq = sky_freq - bandwidth;
    }
}


}//end of namespace
