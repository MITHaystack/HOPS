#include "MHO_FringeRotation.hh"

namespace hops
{

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
    //printf("theta = %f \n", theta);
    theta += mbd * (freq - ref_freq);
    theta *= (-2.0 * M_PI);             // convert to radians
    return std::exp(fImagUnit*theta);


    #pragma message("fix optimize_closure and non-integral sbd phase resid")
    //NEED TO ADD THIS WHEN CALCULATING THE FRINGE RESIDUAL PHASE
    // if (pass->control.optimize_closure) // sacrifice mbd fit for less-noisy closure?
    //     theta += 0.125 * mbd * sb / status.sbd_sep;
    //
    // else                                /* effect of non-integral sbd iff SSB
    //                                      * correct phase to dc edge, based on sb delay */
    //     theta += 0.125 * status.sbd_max * sb / status.sbd_sep;


}


}
