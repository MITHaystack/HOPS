#include "MHO_InterpolateSpectralLinePeak.hh"

namespace hops
{

MHO_InterpolateSpectralLinePeak::MHO_InterpolateSpectralLinePeak()
{
    fSpecDRData = nullptr;
    fWeights = nullptr;
    fRefFreqMHz = 1.0;
    fTotalSummedWeights = 1.0;

    fPeakChan = 0;
    fPeakDRBin = 0;
    fPeakFreqBin = 0;

    fDRAxis.Resize(1);

    fPeakSkyFreqMHz = 0.0;
    fDelayRate = 0.0;
    fFringeRate = 0.0;
    fFringeAmp = 0.0;
    fFringePhase = 0.0;
    fPhaseDelay = 0.0;
}

void MHO_InterpolateSpectralLinePeak::SetMaxBins(int peak_chan, int peak_dr_bin, int peak_freq_bin)
{
    fPeakChan = peak_chan;
    fPeakDRBin = peak_dr_bin;
    fPeakFreqBin = peak_freq_bin;
}

bool MHO_InterpolateSpectralLinePeak::Initialize()
{
    if(fSpecDRData == nullptr)
    {
        msg_error("calibration", "MHO_InterpolateSpectralLinePeak: spec_dr data not set." << eom);
        return false;
    }
    if(fWeights == nullptr)
    {
        msg_error("calibration", "MHO_InterpolateSpectralLinePeak: weight data not set." << eom);
        return false;
    }
    if(fDRAxis.GetSize() < 3)
    {
        msg_error("calibration", "MHO_InterpolateSpectralLinePeak: DR axis too small (need >= 3 bins)." << eom);
        return false;
    }

    bool ok = fWeights->Retrieve("total_summed_weights", fTotalSummedWeights);
    if(!ok)
    {
        msg_warn("calibration", "MHO_InterpolateSpectralLinePeak: missing 'total_summed_weights' tag, using 1." << eom);
        fTotalSummedWeights = 1.0;
    }
    if(fTotalSummedWeights <= 0.0)
    {
        msg_warn("calibration", "MHO_InterpolateSpectralLinePeak: total_summed_weights <= 0, using 1." << eom);
        fTotalSummedWeights = 1.0;
    }

    return true;
}

bool MHO_InterpolateSpectralLinePeak::Execute()
{
    profiler_scope();

    std::size_t N_dr = fSpecDRData->GetDimension(TIME_AXIS);
    std::size_t N_freq = fSpecDRData->GetDimension(FREQ_AXIS);

    // -----------------------------------------------------------------
    // Delay-rate: parabolic interpolation in the DR dimension.
    // -----------------------------------------------------------------
    int dr_m1 = ((fPeakDRBin - 1) % static_cast< int >(N_dr) + static_cast< int >(N_dr)) % static_cast< int >(N_dr);
    int dr_p1 = (fPeakDRBin + 1) % static_cast< int >(N_dr);

    double amp_dr_m1 = std::abs((*fSpecDRData)(0, fPeakChan, dr_m1, fPeakFreqBin));
    double amp_dr_0  = std::abs((*fSpecDRData)(0, fPeakChan, fPeakDRBin, fPeakFreqBin));
    double amp_dr_p1 = std::abs((*fSpecDRData)(0, fPeakChan, dr_p1, fPeakFreqBin));

    double dr_sub = parabolic_offset(amp_dr_m1, amp_dr_0, amp_dr_p1);
    double dr_bin_width = (fDRAxis.GetSize() >= 2) ? (fDRAxis(1) - fDRAxis(0)) : 0.0;
    double fine_dr = fDRAxis(static_cast< std::size_t >(fPeakDRBin)) + dr_sub * dr_bin_width;

    // -----------------------------------------------------------------
    // Intra-channel frequency: parabolic interpolation in the freq dimension.
    // -----------------------------------------------------------------
    int f_m1 = ((fPeakFreqBin - 1) % static_cast< int >(N_freq) + static_cast< int >(N_freq)) % static_cast< int >(N_freq);
    int f_p1 = (fPeakFreqBin + 1) % static_cast< int >(N_freq);

    double amp_f_m1 = std::abs((*fSpecDRData)(0, fPeakChan, fPeakDRBin, f_m1));
    double amp_f_0  = std::abs((*fSpecDRData)(0, fPeakChan, fPeakDRBin, fPeakFreqBin));
    double amp_f_p1 = std::abs((*fSpecDRData)(0, fPeakChan, fPeakDRBin, f_p1));

    double freq_sub = parabolic_offset(amp_f_m1, amp_f_0, amp_f_p1);

    auto& freq_ax = std::get< FREQ_AXIS >(*fSpecDRData);
    double freq_bin_width = (N_freq >= 2) ? (freq_ax(1) - freq_ax(0)) : 0.0;

    // Sky frequency: channel centre + sub-bin refinement using the FREQ_AXIS values.
    auto& chan_ax = std::get< CHANNEL_AXIS >(*fSpecDRData);
    double chan_sky_freq_MHz = chan_ax(static_cast< std::size_t >(fPeakChan));
    double freq_offset_MHz = freq_ax(static_cast< std::size_t >(fPeakFreqBin)) + freq_sub * freq_bin_width;
    fPeakSkyFreqMHz = chan_sky_freq_MHz + freq_offset_MHz;
    std::cout<<"peak freq bin = "<<fPeakFreqBin<<std::endl;
    std::cout<<"freq bin width = "<<freq_bin_width<<std::endl;
    std::cout<<"peak freq = "<<fPeakSkyFreqMHz<<" chan freq = "<<chan_sky_freq_MHz<<" offset = "<<freq_offset_MHz<<std::endl;

    // -----------------------------------------------------------------
    // Fringe phase and amplitude at the coarse peak bin.
    // -----------------------------------------------------------------
    auto peak_vis = (*fSpecDRData)(0, fPeakChan, fPeakDRBin, fPeakFreqBin);
    fFringeAmp = std::abs(peak_vis) / fTotalSummedWeights;
    fFringePhase = std::arg(peak_vis); // radians, in [-pi, pi]

    // -----------------------------------------------------------------
    // Delay rate and fringe rate.
    // -----------------------------------------------------------------
    fDelayRate = fine_dr;                          // sec/sec
    fFringeRate = fine_dr * (fRefFreqMHz * 1e6);   // Hz

    // -----------------------------------------------------------------
    // Phase delay at the spectral-line frequency.
    // tau_phase = phi / (2pi x nu_peak)
    // NOTE: This is NOT the group delay; the group delay is undefined
    //       for a spectrally narrow source.
    // -----------------------------------------------------------------
    double peak_sky_freq_hz = fPeakSkyFreqMHz * 1e6;
    if(peak_sky_freq_hz > 0.0)
    {
        fPhaseDelay = fFringePhase / (2.0 * M_PI * peak_sky_freq_hz);
    }
    else
    {
        fPhaseDelay = 0.0;
    }

    msg_info("calibration", "SpectralLine peak interpolation: sky_freq=" << fPeakSkyFreqMHz
                                                                          << " MHz  drate=" << fDelayRate
                                                                          << " s/s  frate=" << fFringeRate
                                                                          << " Hz  amp=" << fFringeAmp
                                                                          << "  phase=" << fFringePhase << " rad"
                                                                          << "  phase_delay=" << fPhaseDelay * 1e6
                                                                          << " us" << eom);
    return true;
}

double MHO_InterpolateSpectralLinePeak::parabolic_offset(double fm1, double f0, double fp1) const
{
    // Denominator of the parabolic interpolation formula:
    //   offset = (fm1 - fp1) / (2*(fm1 - 2*f0 + fp1))
    double denom = fm1 - 2.0 * f0 + fp1;
    if(std::abs(denom) < 1e-30)
    {
        return 0.0; // flat region - no sub-bin shift
    }
    double offset = 0.5 * (fm1 - fp1) / denom;
    // Clamp to +/-0.5 bin.
    if(offset > 0.5)  { offset = 0.5; }
    if(offset < -0.5) { offset = -0.5; }
    return offset;
}

} // namespace hops
