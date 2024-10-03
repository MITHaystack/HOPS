#include "MHO_MBDelaySearch.hh"

namespace hops
{

MHO_MBDelaySearch::MHO_MBDelaySearch()
{
    fInitialized = false;
    fMBDMaxBin = -1;
    fSBDMaxBin = -1;
    fDRMaxBin = -1;
    fSBDStart = -1;
    fSBDStop = -1;
    fMBDBinMap.clear();

    //the window limits
    fSBDWinSet = false;
    fMBDWinSet = false;
    fDRWinSet = false;
    fSBDWin[0] = 1e30;
    fSBDWin[1] = -1e30;
    fMBDWin[0] = 1e30;
    fMBDWin[1] = -1e30;
    fDRWin[0] = 1e30;
    fDRWin[1] = -1e30;

    fCoarseSBD = 0;
    fCoarseMBD = 0;
    fCoarseDR = 0;
    fSBDBinSep = 0;
    fDRBinSep = 0;
    fMBDBinSep = 0;

    fNPointsSearched = 0;
}

MHO_MBDelaySearch::~MHO_MBDelaySearch(){};

bool MHO_MBDelaySearch::InitializeImpl(const XArgType* in)
{
    fInitialized = false;
    if(in != nullptr)
    {
        //calculate the frequency grid for MBD search
        MHO_UniformGridPointsCalculator fGridCalc;

        //set the channel frequency points...we will use the channel center frequencies
        std::vector< double > freq_pts;
        freq_pts = DetermineFrequencyPoints(in);
        fGridCalc.SetPoints(freq_pts);

        //fGridCalc.SetPoints( std::get<CHANNEL_AXIS>(*in).GetData(), std::get<CHANNEL_AXIS>(*in).GetSize() );
        fGridCalc.Calculate();

        fGridStart = fGridCalc.GetGridStart();
        fGridSpace = fGridCalc.GetGridSpacing();
        fNGridPoints = fGridCalc.GetNGridPoints();
        fAverageFreq = fGridCalc.GetGridAverage();
        fMBDBinMap = fGridCalc.GetGridIndexMap();
        fNSBD = in->GetDimension(FREQ_AXIS);
        fNDR = in->GetDimension(TIME_AXIS);

        if(fSBDStart == -1)
        {
            fSBDStart = 0;
        }
        if(fSBDStop == -1)
        {
            fSBDStop = fNSBD;
        }

        msg_debug("calibration", "MBD search, N grid points = " << fNGridPoints << eom);

        //resize workspaces (TODO...make conditional on current size -- if already configured)
        fMBDWorkspace.Resize(fNGridPoints);    //, fNDR);
        fMBDAmpWorkspace.Resize(fNGridPoints); //, fNDR);

        //copy the tags/axes for the SBD DR workspace
        //copy this slice into local workspace table container
        auto sbd_dims = in->GetDimensionArray();
        sbd_dims[POLPROD_AXIS] = 1;
        sbd_dims[FREQ_AXIS] = 1;
        //auto sbd_dr_dim = fSBDDrWorkspace.GetDimensionArray();

        fSBDDrWorkspace.Resize(&(sbd_dims[0]));
        fSBDDrWorkspace.ZeroArray();
        fSBDDrWorkspace.CopyTags(*in);

        std::get< CHANNEL_AXIS >(fSBDDrWorkspace) = std::get< CHANNEL_AXIS >(*in);
        std::get< TIME_AXIS >(fSBDDrWorkspace) = std::get< TIME_AXIS >(*in);
        std::get< FREQ_AXIS >(fSBDDrWorkspace)(0) = 0.0;

        fDelayRateCalc.SetReferenceFrequency(fRefFreq);
        fDelayRateCalc.SetArgs(&fSBDDrWorkspace, fWeights, &sbd_dr_data);
        bool ok = fDelayRateCalc.Initialize();
        check_step_fatal(ok, "fringe", "Delay rate search fft engine initialization failed." << eom);

        //set up FFT and rotator engines
        fFFTEngine.SetArgs(&fMBDWorkspace);
        fFFTEngine.DeselectAllAxes();
        fFFTEngine.SelectAxis(0);
        fFFTEngine.SetForward();
        ok = fFFTEngine.Initialize();
        check_step_fatal(ok, "fringe", "MBD search fft engine initialization failed." << eom);

        fCyclicRotator.SetOffset(0, fNGridPoints / 2);
        fCyclicRotator.SetArgs(&fMBDWorkspace);
        ok = fCyclicRotator.Initialize();
        check_step_fatal(ok, "fringe", "MBD search cyclic rotation initialization failed." << eom);

        fInitialized = true;
    }

    return fInitialized;
}

bool MHO_MBDelaySearch::ExecuteImpl(const XArgType* in)
{
    bool ok;

    if(fInitialized && fNSBD > 1)
    {
        fSBDAxis = std::get< FREQ_AXIS >(*in);
        fSBDBinSep = fSBDAxis(1) - fSBDAxis(0);
        //loop over the single-band delay 'lags', computing the MBD/DR function
        //find the max for each SBD, and globally
        fMax = -0.0;
        fNPointsSearched = 0;
        bool first = true;
        for(std::size_t sbd_idx = 0; sbd_idx < fNSBD; sbd_idx++)
        {
            double sbd = fSBDAxis(sbd_idx);
            bool do_sbd_search = (fSBDWin[0] <= sbd) && (sbd <= fSBDWin[1]);
            if(!fSBDWinSet || do_sbd_search)
            {
                //first select the slice of visibilities which correspond to this SBD
                //and copy this slice into local workspace table container
                auto sbd_dims = fSBDDrWorkspace.GetDimensionArray();
                std::size_t a = sbd_dims[CHANNEL_AXIS];
                std::size_t b = sbd_dims[TIME_AXIS];
                for(std::size_t i = 0; i < a; i++)
                {
                    std::string net_sideband;
                    int dsb_partner;
                    bool key_present = std::get< CHANNEL_AXIS >(*in).RetrieveIndexLabelKeyValue(a, "net_sideband", net_sideband);
                    bool dsb_key_present = std::get< CHANNEL_AXIS >(*in).RetrieveIndexLabelKeyValue(a, "dsb_partner", dsb_partner);
                    std::size_t remapped_sbd_idx = fNSBD - 1 - sbd_idx;

                    for(std::size_t j = 0; j < b; j++)
                    {
                        if(net_sideband == "L" && dsb_key_present)
                        {
                            fSBDDrWorkspace(0, i, j, 0) = (*in)(0, i, j, remapped_sbd_idx);
                        }
                        else 
                        {
                            fSBDDrWorkspace(0, i, j, 0) = (*in)(0, i, j, sbd_idx);
                        }
                    }
                }

                //run the transformation to delay rate space (this also involves a zero padded FFT)
                ok = fDelayRateCalc.Execute();

                //copy and cache the delay-rate axis just once
                if(first)
                {
                    fDRAxis = std::get< TIME_AXIS >(sbd_dr_data); //upon retrieval this here is fringe-rate
                    fDRAxis *= 1.0 / fRefFreq;                    //now convert to delay rate by dividing by reference frequency
                    fDRBinSep = fDRAxis(1) - fDRAxis(0);
                }

                auto NDRBin = fDRAxis.GetSize(); //sbd_dr_data.GetDimensionArray();
                for(std::size_t dr_idx = 0; dr_idx < NDRBin; dr_idx++)
                {
                    double dr = fDRAxis(dr_idx);
                    bool do_dr_search = (fDRWin[0] <= dr) && (dr <= fDRWin[1]);
                    if(!fDRWinSet || do_dr_search)
                    {
                        //zero out MBD workspace
                        fMBDWorkspace.ZeroArray();

                        //copy in the data from each channel for this SDB/DR
                        std::size_t nch = std::get< CHANNEL_AXIS >(*in).GetSize();
                        for(std::size_t ch = 0; ch < nch; ch++)
                        {
                            std::size_t mbd_bin = fMBDBinMap[ fChannelIndexToFreqPointIndex[ch] ];
                            std::string net_sideband;
                            bool key_present = std::get< CHANNEL_AXIS >(*in).RetrieveIndexLabelKeyValue(ch, "net_sideband", net_sideband);
                            auto val = sbd_dr_data(0, ch, dr_idx, 0);
                            fMBDWorkspace(mbd_bin) += val;
                        }

                        if(first)
                        {
                            //only need to do this once, in order to
                            //set up the mbd delay axis (in frequency space)
                            fFFTEngine.EnableAxisLabelTransformation();
                            auto mbd_ax = &(std::get< 0 >(fMBDWorkspace));
                            for(std::size_t i = 0; i < fNGridPoints; i++)
                            {
                                (*mbd_ax)(i) = fGridStart + i * fGridSpace;
                            }
                        }

                        //now run an FFT along the MBD axis and cyclic rotate
                        bool ok = fFFTEngine.Execute();

                        if(first)
                        {
                            //now grab the transformed (to delay space) mbd axis
                            fMBDAxis = std::get< 0 >(fMBDWorkspace);
                            //turn off for all other iterations
                            fFFTEngine.DisableAxisLabelTransformation();
                            first = false;
                            fMBDBinSep = fMBDAxis(1) - fMBDAxis(0);
                        }

                        check_step_fatal(ok, "fringe", "MBD search fft engine execution." << eom);
                        std::size_t total_mbd_dr_size = fMBDWorkspace.GetSize();

                        for(std::size_t i = 0; i < total_mbd_dr_size; i++)
                        {
                            double mbd = fMBDAxis(i);
                            bool do_mbd_search = (fMBDWin[0] <= mbd) && (mbd <= fMBDWin[1]);
                            if(!fMBDWinSet || do_mbd_search)
                            {
                                //since we don't care about the actual amplitude (just searching for the max location)
                                //this is faster since it doesn't need to take a square root
                                double tmp_max = std::norm(fMBDWorkspace[i]);
                                if(tmp_max > fMax)
                                {
                                    fMax = tmp_max;
                                    //index shift here is because we haven't yet applied the cyclic rotator to the mbd axis
                                    fMBDMaxBin = (i + fNGridPoints / 2) % fNGridPoints;
                                    fSBDMaxBin = sbd_idx;
                                    fDRMaxBin = dr_idx;
                                }
                                fNPointsSearched += 1; //just count each search point we visit
                            }
                        }
                    }
                }
            }
        }

        //only need to do this once after the last iter (to properly set-up the MBD axis)
        ok = fCyclicRotator.Execute();
        check_step_fatal(ok, "fringe", "MBD search cyclic rotation execution." << eom);
        fMBDAxis = std::get< 0 >(fMBDWorkspace);

        if(fCoarseMBD > 0 && fCoarseSBD > 0 && fCoarseDR > 0)
        {
            fCoarseMBD = fMBDAxis(fMBDMaxBin);
            fCoarseSBD = fSBDAxis(fSBDMaxBin);
            fCoarseDR = fDRAxis(fDRMaxBin);
            fMax = std::sqrt(fMax);
            return true;
        }
        else
        {
            msg_error("calibration", "MHO_MBDelaySearch failed to find fringe peak, max = " << fMax << eom);
        }
    }
    else
    {
        msg_error("calibration", "MHO_MBDelaySearch could not execute, intialization failure." << eom);
    }

    return false;
};

double MHO_MBDelaySearch::GetNPointsSearched() const
{
    //factor of 4 is due to the fact that the SBD search space
    //has been zero-padded for interpolation (e.g. all points visited are not independent)
    return fNPointsSearched / 4;
}

//configure the search windows (using floating point limits)
//default is the full range
void MHO_MBDelaySearch::SetSBDWindow(double low, double high)
{
    msg_debug("calibration", "mbd search SBD window set to (" << low << ", " << high << ")" << eom);
    fSBDWinSet = true;
    if(low <= high)
    {
        fSBDWin[0] = low;
        fSBDWin[1] = high;
    }
    else
    {
        fSBDWin[1] = low;
        fSBDWin[0] = high;
    }
}

void MHO_MBDelaySearch::SetMBDWindow(double low, double high)
{
    msg_debug("calibration", "mbd search MBD window set to (" << low << ", " << high << ")" << eom);
    fMBDWinSet = true;
    if(low <= high)
    {
        fMBDWin[0] = low;
        fMBDWin[1] = high;
    }
    else
    {
        fMBDWin[1] = low;
        fMBDWin[0] = high;
    }
}

void MHO_MBDelaySearch::SetDRWindow(double low, double high)
{
    msg_debug("calibration", "mbd search DR window set to (" << low << ", " << high << ")" << eom);
    fDRWinSet = true;
    if(low <= high)
    {
        fDRWin[0] = low;
        fDRWin[1] = high;
    }
    else
    {
        fDRWin[1] = low;
        fDRWin[0] = high;
    }
}

//retrieve the window limits
void MHO_MBDelaySearch::GetSBDWindow(double& low, double& high) const
{
    low = 0.0;
    high = 0.0;
    if(fSBDAxis.GetSize() >= 2)
    {
        low = fSBDAxis.at(0);
        high = fSBDAxis.at(fSBDAxis.GetSize() - 1) + fSBDBinSep;
    }

    if(fSBDWinSet)
    {
        low = std::max(fSBDWin[0], low);
        high = std::min(fSBDWin[1], high);
    }
}

void MHO_MBDelaySearch::GetMBDWindow(double& low, double& high) const
{
    low = 0.0;
    high = 0.0;
    if(fMBDAxis.GetSize() >= 2)
    {
        low = fMBDAxis.at(0);
        high = fMBDAxis.at(fMBDAxis.GetSize() - 1) + fMBDBinSep;
    }
    if(fMBDWinSet)
    {
        low = std::max(fMBDWin[0], low);
        high = std::min(fMBDWin[1], high);
    }
}

void MHO_MBDelaySearch::GetDRWindow(double& low, double& high) const
{
    low = 0.0;
    high = 0.0;
    if(fDRAxis.GetSize() >= 2)
    {
        low = fDRAxis.at(0);
        high = fDRAxis.at(fDRAxis.GetSize() - 1) + fDRBinSep;
    }
    if(fDRWinSet)
    {
        low = std::max(fDRWin[0], low);
        high = std::min(fDRWin[1], high);
    }
}


std::vector<double> 
MHO_MBDelaySearch::DetermineFrequencyPoints(const XArgType* in)
{
    double freq_eps = 1e-4;
    std::vector<double> freq_pts;
    auto chan_ax = &(std::get< CHANNEL_AXIS >(*in));

    //if we have double-sideband channels we want to merge
    std::vector< mho_json > dsb_labels = chan_ax->GetMatchingIntervalLabels("double_sideband");
    std::size_t n_dsb_chan_pair = dsb_labels.size();
    if(n_dsb_chan_pair != 0)
    {
        std::vector<double> tmp;
        for(std::size_t ch = 0; ch < chan_ax->GetSize(); ch++)
        {
            tmp.push_back(chan_ax->at(ch));
        }

        //dsb channel pairs share a sky_freq so we need combine them in the same location
        fChannelIndexToFreqPointIndex.clear();
        std::size_t freq_point_index = 0;
        freq_pts.push_back(tmp[0]);
        fChannelIndexToFreqPointIndex[0] = freq_point_index;
        for(std::size_t ch = 1; ch < tmp.size(); ch++)
        {
            //check if adjacent channels share a frequency within epsilon
            if(std::fabs( tmp[ch] - tmp[ch-1]) > freq_eps )
            {
                freq_pts.push_back(tmp[ch]);
                freq_point_index++;

            }
            fChannelIndexToFreqPointIndex[ch] = freq_point_index;
        }

        //check that the number of frequency points is as expected 
        std::size_t npts = freq_pts.size();
        if(npts == (chan_ax->GetSize() - n_dsb_chan_pair) )
        {
            return freq_pts;
        }
        else 
        {
            //fall through to the mixed LSB/USB case, since the number of channels isn't as expected
            msg_error("calibration", "frequency configuration for double-sideband data is not as expected, assuming mixed LSB/USB" << eom);
        }
    }

    //default behavior is to use the use the mid-points of each channel 
    //(so we have unique freqs for each channel, mainly needed for mixed LSB/USB )
    freq_pts.clear();
    fChannelIndexToFreqPointIndex.clear();
    for(std::size_t ch = 0; ch < chan_ax->GetSize(); ch++)
    {
        fChannelIndexToFreqPointIndex[ch] = ch;
        double sky_freq = (*chan_ax)(ch);
        std::string net_sideband;
        double bandwidth;
        bool key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", net_sideband);
        if(!key_present)
        {
            msg_error("calibration", "mbd search missing net_sideband label for channel " << ch << eom);
        }
        key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "bandwidth", bandwidth);
        if(!key_present)
        {
            msg_error("calibration", "mbd search missing bandwidth label for channel " << ch << eom);
        }
        double center_freq = sky_freq;
        if(net_sideband == "L")
        {
            center_freq -= bandwidth / 2.0;
        }
        if(net_sideband == "U")
        {
            center_freq += bandwidth / 2.0;
        }
        freq_pts.push_back(center_freq);
    }

    return freq_pts;

}


} // namespace hops
