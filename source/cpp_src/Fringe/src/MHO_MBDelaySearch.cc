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


bool
MHO_MBDelaySearch::InitializeImpl(const XArgType* in)
{
    fInitialized = false;
    if(in != nullptr)
    {
        //calculate the frequency grid for MBD search
        MHO_UniformGridPointsCalculator fGridCalc;
        fGridCalc.SetPoints( std::get<CHANNEL_AXIS>(*in).GetData(), std::get<CHANNEL_AXIS>(*in).GetSize() );
        fGridCalc.Calculate();

        fGridStart = fGridCalc.GetGridStart();
        fGridSpace = fGridCalc.GetGridSpacing();
        fNGridPoints = fGridCalc.GetNGridPoints();
        fAverageFreq = fGridCalc.GetGridAverage();
        fMBDBinMap = fGridCalc.GetGridIndexMap();
        fNSBD = in->GetDimension(FREQ_AXIS);
        fNDR = in->GetDimension(TIME_AXIS);

        if(fSBDStart == -1){fSBDStart = 0;}
        if(fSBDStop == -1){fSBDStop = fNSBD;}

        msg_debug("fringe", "MBD search, N grid points = " << fNGridPoints << eom);

        //resize workspaces (TODO...make conditional on current size -- if already configured)
        fMBDWorkspace.Resize(fNGridPoints);//, fNDR);
        fMBDAmpWorkspace.Resize(fNGridPoints);//, fNDR);

        //copy the tags/axes for the SBD DR workspace
        //copy this slice into local workspace table container
        auto sbd_dims = in->GetDimensionArray();
        sbd_dims[POLPROD_AXIS] = 1;
        sbd_dims[FREQ_AXIS] = 1;
        //auto sbd_dr_dim = fSBDDrWorkspace.GetDimensionArray();

        fSBDDrWorkspace.Resize( &(sbd_dims[0]) );
        fSBDDrWorkspace.ZeroArray();
        fSBDDrWorkspace.CopyTags(*in);

        std::get<CHANNEL_AXIS>(fSBDDrWorkspace) = std::get<CHANNEL_AXIS>(*in);
        std::get<TIME_AXIS>(fSBDDrWorkspace) = std::get<TIME_AXIS>(*in);
        std::get<FREQ_AXIS>(fSBDDrWorkspace)(0) = 0.0;

        fDelayRateCalc.SetReferenceFrequency(fRefFreq);
        fDelayRateCalc.SetArgs(&fSBDDrWorkspace, fWeights, &sbd_dr_data);
        bool ok = fDelayRateCalc.Initialize();
        check_step_fatal(ok, "fringe", "Delay rate search fft engine initialization failed." << eom );

        //set up FFT and rotator engines
        fFFTEngine.SetArgs(&fMBDWorkspace);
        fFFTEngine.DeselectAllAxes();
        fFFTEngine.SelectAxis(0);
        fFFTEngine.SetForward();
        ok = fFFTEngine.Initialize();
        check_step_fatal(ok, "fringe", "MBD search fft engine initialization failed." << eom );

        fCyclicRotator.SetOffset(0, fNGridPoints/2);
        fCyclicRotator.SetArgs(&fMBDWorkspace);
        ok = fCyclicRotator.Initialize();
        check_step_fatal(ok, "fringe", "MBD search cyclic rotation initialization failed." << eom );

        fInitialized = true;
    }

    return fInitialized;
}


bool
MHO_MBDelaySearch::ExecuteImpl(const XArgType* in)
{
    bool ok;

    if(fInitialized && fNSBD > 1)
    {
        fSBDAxis = std::get<FREQ_AXIS>(*in);
        fSBDBinSep = fSBDAxis.at(1) - fSBDAxis.at(0);
        //loop over the single-band delay 'lags', computing the MBD/DR function
        //find the max for each SBD, and globally
        fMax = -0.0;
        fNPointsSearched = 0;
        bool first = true;
        for(std::size_t sbd_idx=0; sbd_idx<fNSBD; sbd_idx++)
        {
            double sbd = (fSBDAxis.at(sbd_idx));
            bool do_sbd_search = (fSBDWin[0] <= sbd) && (sbd <= fSBDWin[1]);
            if(!fSBDWinSet || do_sbd_search)
            {
                //first select the slice of visibilities which correspond to this SBD
                //and copy this slice into local workspace table container
                auto sbd_dims = fSBDDrWorkspace.GetDimensionArray();
                std::size_t a = sbd_dims[CHANNEL_AXIS];
                std::size_t b = sbd_dims[TIME_AXIS];
                for(std::size_t i=0;i<a;i++)
                {
                    for(std::size_t j=0;j<b;j++)
                    {
                        fSBDDrWorkspace(0,i,j,0) = (*in)(0,i,j,sbd_idx);
                    }
                }

                //run the transformation to delay rate space (this also involves a zero padded FFT)
                ok = fDelayRateCalc.Execute();

                //copy and cache the delay-rate axis just once
                if(first)
                {
                    fDRAxis = std::get<TIME_AXIS>(sbd_dr_data); //upon retrieval this here is fringe-rate
                    fDRAxis *= 1.0/fRefFreq; //now convert to delay rate by dividing by reference frequency
                    fDRBinSep = fDRAxis.at(1) - fDRAxis.at(0);
                }

                auto NDRBin = fDRAxis.GetSize() ;//sbd_dr_data.GetDimensionArray();
                for(std::size_t dr_idx=0; dr_idx < NDRBin; dr_idx++)
                {
                    double dr = fDRAxis.at(dr_idx);
                    bool do_dr_search = (fDRWin[0] <= dr) && (dr <= fDRWin[1]);
                    if(!fDRWinSet || do_dr_search)
                    {
                        //zero out MBD workspace
                        fMBDWorkspace.ZeroArray();

                        //copy in the data from each channel for this SDB/DR
                        std::size_t nch = std::get<CHANNEL_AXIS>(*in).GetSize();
                        for(std::size_t ch=0; ch<nch; ch++)
                        {
                            std::size_t mbd_bin = fMBDBinMap[ch];
                            fMBDWorkspace(mbd_bin) = sbd_dr_data(0, ch, dr_idx, 0);
                        }

                        if(first)
                        {
                            //only need to do this once, in order to
                            //set up the mbd delay axis (in frequency space)
                            fFFTEngine.EnableAxisLabelTransformation();
                            auto mbd_ax = &(std::get<0>(fMBDWorkspace) );
                            for(std::size_t i=0; i<fNGridPoints;i++)
                            {
                                mbd_ax->at(i) = fGridStart + i*fGridSpace;
                            }
                        }

                        //now run an FFT along the MBD axis and cyclic rotate
                        bool ok = fFFTEngine.Execute();

                        if(first)
                        {
                            //now grab the transformed (to delay space) mbd axis
                            fMBDAxis = std::get<0>(fMBDWorkspace);
                            //turn off for all other iterations
                            fFFTEngine.DisableAxisLabelTransformation();
                            first = false;
                            fMBDBinSep = fMBDAxis.at(1) - fMBDAxis.at(0);
                        }

                        check_step_fatal(ok, "fringe", "MBD search fft engine execution." << eom );
                        std::size_t total_mbd_dr_size = fMBDWorkspace.GetSize();

                        for(std::size_t i=0; i<total_mbd_dr_size; i++)
                        {
                            double mbd = (fMBDAxis.at(i));
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
                                    fMBDMaxBin = (i + fNGridPoints/2) % fNGridPoints;
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
        check_step_fatal(ok, "fringe", "MBD search cyclic rotation execution." << eom );
        fMBDAxis = std::get<0>(fMBDWorkspace);


        fCoarseMBD = fMBDAxis(fMBDMaxBin);
        fCoarseSBD = fSBDAxis(fSBDMaxBin);
        fCoarseDR = fDRAxis(fDRMaxBin);

        fMax = std::sqrt(fMax);
        return true;
    }
    else
    {
        msg_error("fringe", "MHO_MBDelaySearch could not execute, intialization failure." << eom);
    }

    return false;
};


double
MHO_MBDelaySearch::GetNPointsSearched() const
{
    //factor of 4 is due to the fact that the SBD search space
    //has been zero-padded for interpolation (e.g. all points visited are not independent)
    return fNPointsSearched/4;
}

//configure the search windows (using floating point limits)
//default is the full range
void
MHO_MBDelaySearch::SetSBDWindow(double low, double high)
{
    fSBDWinSet = true;
    if(low <= high){fSBDWin[0] = low; fSBDWin[1] = high;}
    else{fSBDWin[1] = low; fSBDWin[0] = high;}
}

void
MHO_MBDelaySearch::SetMBDWindow(double low, double high)
{
    fMBDWinSet = true;
    if(low <= high){fMBDWin[0] = low; fMBDWin[1] = high;}
    else{fMBDWin[1] = low; fMBDWin[0] = high;}
}

void
MHO_MBDelaySearch::SetDRWindow(double low, double high)
{
    fDRWinSet = true;
    if(low <= high){fDRWin[0] = low; fDRWin[1] = high;}
    else{fDRWin[1] = low; fDRWin[0] = high;}
}

//retrieve the window limits
void
MHO_MBDelaySearch::GetSBDWindow(double& low, double& high) const
{
    low = 0.0;
    high = 0.0;
    if(fSBDAxis.GetSize() >= 2)
    {
        low = fSBDAxis.at(0);
        high = fSBDAxis.at(fSBDAxis.GetSize()-1) + fSBDBinSep;
    }

    if(fSBDWinSet)
    {
        low = std::max(fSBDWin[0], low);
        high = std::min(fSBDWin[1], high);
    }
}

void
MHO_MBDelaySearch::GetMBDWindow(double& low, double& high) const
{
    low = 0.0;
    high = 0.0;
    if(fMBDAxis.GetSize() >= 2)
    {
        low = fMBDAxis.at(0);
        high = fMBDAxis.at(fMBDAxis.GetSize()-1) + fMBDBinSep;
    }
    if(fMBDWinSet)
    {
        low = std::max( fMBDWin[0], low);
        high = std::min(fMBDWin[1], high);
    }
}

void
MHO_MBDelaySearch::GetDRWindow(double& low, double& high) const
{
    low = 0.0;
    high = 0.0;
    if(fDRAxis.GetSize() >= 2)
    {
        low = fDRAxis.at(0);
        high = fDRAxis.at(fDRAxis.GetSize()-1) + fDRBinSep;
    }
    if(fDRWinSet)
    {
        low = std::max( fDRWin[0], low);
        high = std::min(fDRWin[1], high);
    }
}


}
