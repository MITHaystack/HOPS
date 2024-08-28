#include "MHO_FringeRotation.hh"
#include <iostream>
namespace hops
{


MHO_FringeRotation::MHO_FringeRotation()
{
    fSideband = 0;
    fNSBDBins = 0;
    fSBDMaxBin = 0;
    fSBDMax = 0.0;
    fSBDSep = 1.0;
    fOptimizeClosure = false;
};

const std::complex<double> MHO_FringeRotation::fImagUnit = std::complex<double>(0.0, 1.0);

std::complex<double>
MHO_FringeRotation::vrot(double time_delta, double freq, double ref_freq, double dr, double mbd) const
{
    return vrot_v1(time_delta, freq, ref_freq, dr, mbd);
}


std::complex<double>
MHO_FringeRotation::vrot_v1(double time_delta, double freq, double ref_freq, double dr, double mbd) const
{
    double theta = freq * dr * time_delta; //fringe rotation due to delay_rate * time_delta
    theta += mbd * (freq - ref_freq); //fringe rotation due to delay * freq_delta
    theta += calc_sideband_correction(mbd); //TODO, what is the origin of this correction?
    return std::exp(-2.0 * M_PI * fImagUnit * theta); //return the phasor
}



double
MHO_FringeRotation::calc_sideband_correction(double mbd) const
{
    double theta_corr = 0.0;
    //Why is none of the following used in vrot when locating the peak in MBD/DR space, but used when computing the fringe phase?
    theta_corr += (fNSBDBins - fSBDMaxBin)* 0.125 * fSideband; // Effect due to offset of lag where max lies?
    //vrot.c says:
    //effect of non-integral sbd iff SSB
    //correct phase to dc edge, based on sb delay */
    double oc_corr = 0.125 * mbd * fSideband / fSBDSep;
    if(fOptimizeClosure){theta_corr += oc_corr;} //std::cout<<"oc correction: "<<oc_corr<<std::endl; std::cout<<mbd<<", "<<fSideband<<", "<<fSBDSep<<std::endl;}
    else{theta_corr += (0.125 * fSBDMax * fSideband) / fSBDSep;}
    return theta_corr;
}

}
