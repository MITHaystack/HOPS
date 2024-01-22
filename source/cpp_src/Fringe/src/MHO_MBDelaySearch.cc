#include "MHO_MBDelaySearch.hh"

namespace hops
{

MHO_MBDelaySearch::MHO_MBDelaySearch()
{
    fInitialized = false;
    fMBDMaxBin = -1;
    fSBDMaxBin = -1;
    fDRMaxBin = -1;
    fMBDBinMap.clear();

    //the window limits 
    fSBDWinSet = false;
    fMBDWinSet = false;
    fDRWinSet = false;

    fSBDWin[0] = 0.0;
    fSBDWin[1] = 0.0;
    fMBDWin[0] = 0.0;
    fMBDWin[1] = 0.0;
    fDRWin[0] = 0.0;
    fDRWin[1] = 0.0;

    fSBDWinBin[0] = -1;
    fSBDWinBin[1] = -1;
    fMBDWinBin[0] = -1;
    fMBDWinBin[1] = -1;
    fDRWinBin[0] = -1;
    fDRWinBin[1] = -1;
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

    if(fInitialized)
    {
        fSBDAxis = std::get<FREQ_AXIS>(*in);

        ConfigureWindows();

        //loop over the single-band delay 'lags', computing the MBD/DR function
        //find the max for each SBD, and globally
        fMax = -0.0;
        bool first = true;
        for(std::size_t sbd_idx=0; sbd_idx<fNSBD; sbd_idx++)
        {
            double sbd = 1e-6*(fSBDAxis.at(sbd_idx));
            bool do_search = (fSBDWin[0] <= sbd) && (sbd <= fSBDWin[1]);
            std::cout<<"win, sbd, win  = "<<fSBDWin[0]<<", "<<sbd<<", "<<fSBDWin[1]<<std::endl;
            if(do_search)
            {
                std::cout<<"SBDIDX = "<<sbd_idx<<std::endl;
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
                if(first){fDRAxis = std::get<TIME_AXIS>(sbd_dr_data);} //copy the axis just once


                auto sbd_dr_dim = sbd_dr_data.GetDimensionArray();
                for(std::size_t dr_idx=0; dr_idx < sbd_dr_dim[TIME_AXIS]; dr_idx++)
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
                        //off for all other iterations
                        fFFTEngine.DisableAxisLabelTransformation();
                        first = false;
                    }

                    check_step_fatal(ok, "fringe", "MBD search fft engine execution." << eom );

                    std::size_t total_mbd_dr_size = fMBDWorkspace.GetSize();
                    for(std::size_t i=0; i<total_mbd_dr_size; i++)
                    {
                        //since we don't care about the actual amplitude (just searching for the max location)
                        //this is faster since it doesn't need to take a square root
                        double tmp_max = std::norm(fMBDWorkspace[i]);
                        if(tmp_max > fMax)
                        {
                            fMax = tmp_max;
                            fMBDMaxBin = (i + fNGridPoints/2) % fNGridPoints;
                            fSBDMaxBin = sbd_idx;
                            fDRMaxBin = dr_idx;
                        }
                    }
                }
            }
        }

        //only need to do this once after the last iter (to properly set-up the MBD axis)
        ok = fCyclicRotator.Execute();
        check_step_fatal(ok, "fringe", "MBD search cyclic rotation execution." << eom );
        fMBDAxis = std::get<0>(fMBDWorkspace);

        fMax = std::sqrt(fMax);
        return true;
    }

    return false;
};


void MHO_MBDelaySearch::ConfigureWindows()
{
    if(!fSBDWinSet) //unset, use full range
    {
        fSBDWin[0] = fSBDAxis[0];
        fSBDWin[1] = fSBDAxis[fNSBD-1];
    }
    std::cout<<"MY SBD WIN = "<<fSBDWin[0]<<", "<<fSBDWin[1]<<std::endl;
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

//configure the search windows using bin numbers (will take precendence over floating point values)
//default is the full range
void 
MHO_MBDelaySearch::SetSBDWindowBins(int low, int high)
{
    if(low <= high){fSBDWinBin[0] = low; fSBDWinBin[1] = high;}
    else{fSBDWinBin[1] = low; fSBDWinBin[0] = high;}
}

void 
MHO_MBDelaySearch::SetMBDWindowBins(int low, int high)
{
    if(low <= high){fMBDWinBin[0] = low; fMBDWinBin[1] = high;}
    else{fMBDWinBin[1] = low; fMBDWinBin[0] = high;}
}

void 
MHO_MBDelaySearch::SetDRWindowBins(int low, int high)
{
    if(low <= high){fDRWinBin[0] = low; fDRWinBin[1] = high;}
    else{fDRWinBin[1] = low; fDRWinBin[0] = high;}
}

//retrieve the window limits 
void 
MHO_MBDelaySearch::GetSBDWindow(double& low, double& high) const
{
    low = fSBDWin[0]; high = fSBDWin[1];
}

void 
MHO_MBDelaySearch::GetMBDWindow(double& low, double& high) const
{
    low = fMBDWin[0]; high = fMBDWin[1];
}

void 
MHO_MBDelaySearch::GetDRWindow(double& low, double& high) const
{
    low = fDRWin[0]; high = fDRWin[1];
}










}
