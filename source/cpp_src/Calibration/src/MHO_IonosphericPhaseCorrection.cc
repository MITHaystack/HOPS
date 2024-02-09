#include "MHO_IonosphericPhaseCorrection.hh"
#include "MHO_Constants.hh"

namespace hops
{


MHO_IonosphericPhaseCorrection::MHO_IonosphericPhaseCorrection()
{
    fSidebandLabelKey = "net_sideband";
    fBandwidthKey = "bandwidth";
    fLowerSideband = "L";
    fUpperSideband = "U";

    fdTEC = 0.0;

    fIonoK = MHO_Constants::ion_k;
    fImagUnit = MHO_Constants::imag_unit;
    fDegToRad = MHO_Constants::deg_to_rad;

};

MHO_IonosphericPhaseCorrection::~MHO_IonosphericPhaseCorrection(){};


bool
MHO_IonosphericPhaseCorrection::ExecuteInPlace(visibility_type* in)
{
    auto pp_ax = &(std::get<POLPROD_AXIS>(*in) );
    auto chan_ax = &(std::get<CHANNEL_AXIS>(*in) );
    
    //loop over pol-products
    for(std::size_t pp=0; pp < pp_ax->GetSize(); pp++)
    {
        //now loop over the channels
        for(std::size_t ch=0; ch < chan_ax->GetSize(); ch++)
        {
            //get channel's frequency info
            double sky_freq = (*chan_ax)(ch); //get the sky frequency of this channel
            double bandwidth = 0;
            std::string net_sideband;

            bool key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, fSidebandLabelKey, net_sideband);
            if(!key_present){msg_error("calibration", "missing net_sideband label for channel "<< ch << "." << eom); }
            key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, fBandwidthKey, bandwidth);
            if(!key_present){msg_error("calibration", "missing bandwidth label for channel "<< ch << "." << eom);}

            //figure out the upper/lower frequency limits for this channel
            //std::cout<<"working on channel: "<<ch<<" with sky freq: "<<sky_freq<<" sideband: "<<net_sideband<< std::endl;
            double lower_freq, upper_freq;
            DetermineChannelFrequencyLimits(sky_freq, bandwidth, net_sideband, lower_freq, upper_freq);
            double chan_center_freq = 0.5*(lower_freq+upper_freq);
            
            //calculate the differential ionospheric phase rotation (assume constant over length of scan)
            double ion_theta = fIonoK * fdTEC / (1e6 * chan_center_freq);
            std::complex<double> ion_phasor = std::exp(fImagUnit*ion_theta);
            
            //retrieve and multiply against appropriate sub-view of the visibility array
            auto chunk = in->SubView(pp, ch);
            chunk *= ion_phasor;
        }
    }

    return true;
}


bool
MHO_IonosphericPhaseCorrection::ExecuteOutOfPlace(const visibility_type* in, visibility_type* out)
{
    out->Copy(*in);
    return ExecuteInPlace(out);
}


void
MHO_IonosphericPhaseCorrection::DetermineChannelFrequencyLimits(double sky_freq, double bandwidth, std::string net_sideband, double& lower_freq, double& upper_freq)
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



bool
MHO_IonosphericPhaseCorrection::InitializeInPlace(visibility_type* /*in*/){ return true;}


bool
MHO_IonosphericPhaseCorrection::InitializeOutOfPlace(const visibility_type* /*in*/, visibility_type* /*out*/){return true;}


}//end of namespace
