#include "MHO_SpectralLineFringeSearch.hh"

namespace hops
{

MHO_SpectralLineFringeSearch::MHO_SpectralLineFringeSearch()
{
    fWeights = nullptr;
    fRefFreq = 1.0; // MHz - caller must set

    fDRWinSet = false;
    fFreqWinSet = false;
    fDRWin[0] = 1e30;
    fDRWin[1] = -1e30;
    fFreqWin[0] = 1e30;
    fFreqWin[1] = -1e30;

    fInitialized = false;

    fNAP = 0;
    fNPaddedAP = 0;
    fNChan = 0;
    fNFreq = 0;
    fNDR = 0;
    fDRBinSep = 0.0;

    fChanMaxBin = -1;
    fDRMaxBin = -1;
    fFreqMaxBin = -1;

    fCoarseDR = 0.0;
    fCoarsePeakSkyFreqMHz = 0.0;
    fMax = 0.0;
}

MHO_SpectralLineFringeSearch::~MHO_SpectralLineFringeSearch() {}

void MHO_SpectralLineFringeSearch::SetWindow(double* win, double low, double high)
{
    if(low <= high)
    {
        win[0] = low;
        win[1] = high;
    }
    else
    {
        win[0] = high;
        win[1] = low;
    }
}

void MHO_SpectralLineFringeSearch::SetDRWindow(double low, double high)
{
    msg_debug("calibration", "spectral line search DR window set to (" << low << ", " << high << ")" << eom);
    fDRWinSet = true;
    SetWindow(fDRWin, low, high);
}

void MHO_SpectralLineFringeSearch::GetDRWindow(double& low, double& high) const
{
    if(fDRWinSet)
    {
        low = fDRWin[0];
        high = fDRWin[1];
    }
    else if(fDRAxis.GetSize() >= 2)
    {
        low = fDRAxis.at(0);
        high = fDRAxis.at(fDRAxis.GetSize() - 1) + fDRBinSep;
    }
    else
    {
        low = 0.0;
        high = 0.0;
    }
}

void MHO_SpectralLineFringeSearch::SetFrequencyWindow(double low_mhz, double high_mhz)
{
    msg_debug("calibration", "spectral line search frequency window set to (" << low_mhz << ", " << high_mhz << ") MHz" << eom);
    fFreqWinSet = true;
    SetWindow(fFreqWin, low_mhz, high_mhz);
}

void MHO_SpectralLineFringeSearch::GetFrequencyWindow(double& low_mhz, double& high_mhz) const
{
    low_mhz = fFreqWin[0];
    high_mhz = fFreqWin[1];
}

bool MHO_SpectralLineFringeSearch::InitializeImpl(const XArgType* in)
{
    fInitialized = false;

    if(in == nullptr)
    {
        msg_error("calibration", "MHO_SpectralLineFringeSearch: input visibility data is null." << eom);
        return false;
    }
    if(fWeights == nullptr)
    {
        msg_error("calibration", "MHO_SpectralLineFringeSearch: weight data has not been set." << eom);
        return false;
    }

    profiler_scope();

    fNChan = in->GetDimension(CHANNEL_AXIS);
    fNAP = in->GetDimension(TIME_AXIS);
    fNFreq = in->GetDimension(FREQ_AXIS);

    if(fNAP < 2)
    {
        msg_error("calibration", "MHO_SpectralLineFringeSearch: need at least 2 accumulation periods, got " << fNAP << eom);
        return false;
    }

    // Zero-pad time axis to the next power of two >= 2 * N_AP for FFT efficiency
    fNPaddedAP = 1;
    while(fNPaddedAP < 2 * fNAP)
    {
        fNPaddedAP *= 2;
    }
    fNDR = fNPaddedAP;

    // Resize workspace: [1][N_chan][N_padded_AP][N_freq]
    std::size_t dims[VIS_NDIM];
    dims[POLPROD_AXIS] = 1;
    dims[CHANNEL_AXIS] = fNChan;
    dims[TIME_AXIS] = fNPaddedAP;
    dims[FREQ_AXIS] = fNFreq;
    fSpecDRWorkspace.Resize(dims);
    fSpecDRWorkspace.ZeroArray();

    // Copy channel and frequency axis labels from input
    std::get< CHANNEL_AXIS >(fSpecDRWorkspace) = std::get< CHANNEL_AXIS >(*in);
    std::get< FREQ_AXIS >(fSpecDRWorkspace) = std::get< FREQ_AXIS >(*in);

    // Build the delay-rate axis.
    // AP period from time axis (uniform sampling assumed).
    auto& time_ax = std::get< TIME_AXIS >(*in);
    double ap_delta = time_ax(1) - time_ax(0);

    // After fftshift, bin k has fringe rate = (k - N/2) / (N * dt).
    // Delay rate = fringe rate / ref_freq  (ref_freq in MHz -> need freq in Hz for delay-rate in s/s).
    double ref_freq_hz = fRefFreq * 1e6;
    fDRAxis.Resize(fNDR);
    for(std::size_t k = 0; k < fNDR; k++)
    {
        double fringe_rate = (static_cast< double >(k) - static_cast< double >(fNDR) / 2.0) /
                             (static_cast< double >(fNDR) * ap_delta);
        fDRAxis(k) = fringe_rate / ref_freq_hz;
    }
    fDRBinSep = (fNDR >= 2) ? (fDRAxis(1) - fDRAxis(0)) : 0.0;

    msg_debug("calibration", "SpectralLineFringeSearch: N_chan=" << fNChan << " N_AP=" << fNAP
                                                                 << " N_padded=" << fNPaddedAP << " N_freq=" << fNFreq
                                                                 << " DR_bin_sep=" << fDRBinSep << " s/s" << eom);

    // Configure in-place FFT engine along the time (delay-rate after FFT) axis.
    fFFTEngine.SetArgs(&fSpecDRWorkspace);
    fFFTEngine.DeselectAllAxes();
    fFFTEngine.SelectAxis(TIME_AXIS);
    fFFTEngine.SetForward();
    bool ok = fFFTEngine.Initialize();
    if(!ok)
    {
        msg_error("calibration", "MHO_SpectralLineFringeSearch: FFT engine initialization failed." << eom);
        return false;
    }

    // Configure cyclic rotator to shift TIME_AXIS by N/2 (fftshift: puts DC at centre).
    fCyclicRotator.SetOffset(TIME_AXIS, static_cast< int64_t >(fNPaddedAP / 2));
    fCyclicRotator.SetArgs(&fSpecDRWorkspace);
    ok = fCyclicRotator.Initialize();
    if(!ok)
    {
        msg_error("calibration", "MHO_SpectralLineFringeSearch: cyclic rotator initialization failed." << eom);
        return false;
    }

    fInitialized = true;
    return true;
}

bool MHO_SpectralLineFringeSearch::ExecuteImpl(const XArgType* in)
{
    if(!fInitialized || in == nullptr || fWeights == nullptr)
    {
        msg_error("calibration", "MHO_SpectralLineFringeSearch: cannot execute, initialization failure." << eom);
        return false;
    }

    profiler_scope();

    // -----------------------------------------------------------------
    // Step 1: Fill workspace with weighted visibilities.
    // Weight dim layout: [polprod][channel][AP][freq], but weights are
    // per (channel, AP) - the freq dimension of the weight array has size 1
    // (a single scalar weight applies to all spectral bins in that channel/AP).
    // -----------------------------------------------------------------
    fSpecDRWorkspace.ZeroArray();
    for(std::size_t ch = 0; ch < fNChan; ch++)
    {
        for(std::size_t t = 0; t < fNAP; t++)
        {
            double wt = (*fWeights)(0, ch, t, 0);
            for(std::size_t f = 0; f < fNFreq; f++)
            {
                fSpecDRWorkspace(0, ch, t, f) = (*in)(0, ch, t, f) * wt;
            }
        }
        // Bins t >= fNAP are already zero (ZeroArray above).
    }

    // -----------------------------------------------------------------
    // Step 2: In-place FFT along TIME_AXIS -> delay-rate spectrum.
    // -----------------------------------------------------------------
    bool ok = fFFTEngine.Execute();
    if(!ok)
    {
        msg_error("calibration", "MHO_SpectralLineFringeSearch: FFT execution failed." << eom);
        return false;
    }

    // -----------------------------------------------------------------
    // Step 3: Cyclic rotate TIME_AXIS by N/2 to centre zero delay-rate.
    // -----------------------------------------------------------------
    ok = fCyclicRotator.Execute();
    if(!ok)
    {
        msg_error("calibration", "MHO_SpectralLineFringeSearch: cyclic rotation failed." << eom);
        return false;
    }

    // -----------------------------------------------------------------
    // Step 4: Peak search over (channel, DR bin, freq bin).
    // -----------------------------------------------------------------
    fMax = -1.0;
    fChanMaxBin = -1;
    fDRMaxBin = -1;
    fFreqMaxBin = -1;

    auto& chan_ax = std::get< CHANNEL_AXIS >(*in);

    for(std::size_t ch = 0; ch < fNChan; ch++)
    {
        double chan_sky_freq = chan_ax(ch); // MHz
        bool do_freq_search = !fFreqWinSet || (fFreqWin[0] <= chan_sky_freq && chan_sky_freq <= fFreqWin[1]);
        if(!do_freq_search)
        {
            continue;
        }

        for(std::size_t dr_idx = 0; dr_idx < fNDR; dr_idx++)
        {
            double dr = fDRAxis(dr_idx);
            bool do_dr_search = !fDRWinSet || (fDRWin[0] <= dr && dr <= fDRWin[1]);
            if(!do_dr_search)
            {
                continue;
            }

            for(std::size_t f = 0; f < fNFreq; f++)
            {
                // Use std::norm (squared magnitude) to avoid sqrt in the hot loop.
                double amp_sq = std::norm(fSpecDRWorkspace(0, ch, dr_idx, f));
                if(amp_sq > fMax)
                {
                    fMax = amp_sq;
                    fChanMaxBin = static_cast< int >(ch);
                    fDRMaxBin = static_cast< int >(dr_idx);
                    fFreqMaxBin = static_cast< int >(f);
                }
            }
        }
    }

    // Convert from squared magnitude to amplitude.
    fMax = std::sqrt(fMax);

    if(fChanMaxBin >= 0 && fDRMaxBin >= 0 && fFreqMaxBin >= 0)
    {
        fCoarseDR = fDRAxis(static_cast< std::size_t >(fDRMaxBin));
        fCoarsePeakSkyFreqMHz = chan_ax(static_cast< std::size_t >(fChanMaxBin));

        msg_debug("calibration",
                  "SpectralLineFringeSearch coarse peak: chan=" << fChanMaxBin << " dr_bin=" << fDRMaxBin
                                                                << " freq_bin=" << fFreqMaxBin
                                                                << " DR=" << fCoarseDR << " s/s"
                                                                << " sky_freq=" << fCoarsePeakSkyFreqMHz << " MHz"
                                                                << " amp=" << fMax << eom);
        return true;
    }

    msg_debug("calibration", "MHO_SpectralLineFringeSearch: failed to locate peak, amp=" << fMax << eom);
    return false;
}

} // namespace hops
