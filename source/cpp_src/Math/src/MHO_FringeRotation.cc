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
    double theta = freq * dr * time_delta;
    theta += mbd * (freq - ref_freq);
    
    //TODO, what is the origin of this correction?
    //Why is it used when computing the fringe phase, but not in vrot when locating the peak in MBD/DR space?
    //vrot.c says:
    //effect of non-integral sbd iff SSB
    //correct phase to dc edge, based on sb delay */
    
    // Effect due to offset of lag where max lies (huh??)
    theta += (fNSBDBins - fSBDMaxBin)* 0.125 * fSideband;
    
    if(fOptimizeClosure){theta += 0.125 * mbd * fSideband / fSBDSep;}
    else{theta += (0.125 * fSBDMax * fSideband) / fSBDSep;}

    std::cout<<"adding: "<<(0.125 * fSBDMax * fSideband) / fSBDSep<<std::endl;

    theta *= (-2.0 * M_PI);             // convert to radians
    return std::exp(fImagUnit*theta);





}


}
