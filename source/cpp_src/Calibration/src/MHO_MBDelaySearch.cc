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
        fMBDBinMap = fGridCalc.GetGridIndexMap();
        fNSBD = in->GetDimension(FREQ_AXIS);
        fNDR = in->GetDimension(TIME_AXIS);

        msg_debug("calibration", "MBD search, N grid points = " << fNGridPoints << eom);

        //resize workspaces (TODO...make conditional on current size -- if already configured)
        fMBDWorkspace.Resize(fNGridPoints);//, fNDR);
        fMBDAmpWorkspace.Resize(fNGridPoints);//, fNDR);

        //set up FFT and rotator engines
        fFFTEngine.SetArgs(&fMBDWorkspace);
        fFFTEngine.DeselectAllAxes();
        fFFTEngine.SelectAxis(0);
        fFFTEngine.SetForward();
        bool ok = fFFTEngine.Initialize();
        check_step_fatal(ok, "calibration", "MBD search fft engine initialization." << eom );

        fCyclicRotator.SetOffset(0, fNGridPoints/2);
        fCyclicRotator.SetArgs(&fMBDWorkspace);
        ok = fCyclicRotator.Initialize();
        check_step_fatal(ok, "calibration", "MBD search cyclic rotation initialization." << eom );

        fInitialized = true;
    }

    return fInitialized;
}


bool
MHO_MBDelaySearch::ExecuteImpl(const XArgType* in)
{

    if(fInitialized)
    {
        std::cout<<"dims = "<<fNSBD<<", "<<fNDR<<std::endl;
        
        //loop over the single-band delay 'lags', computing the MBD/DR function
        //find the max for each SBD, and globally
        double maxmbd = 0.0;
        for(std::size_t sbd_idx=0; sbd_idx<fNSBD; sbd_idx++)
        {
            for(std::size_t dr_idx=0; dr_idx < fNDR; dr_idx++)
            {
                fMBDWorkspace.ZeroArray(); //zero out workspace
                
                //copy in the data from each channel for this SDB/DR
                std::size_t nch = std::get<CHANNEL_AXIS>(*in).GetSize();
                for(std::size_t ch=0; ch<nch; ch++)
                {
                    std::size_t mbd_bin = fMBDBinMap[ch];
                    fMBDWorkspace(mbd_bin) = (*in)(0, ch, dr_idx, sbd_idx);
                }
    
                if(sbd_idx == fNSBD-1 && dr_idx == fNDR-1) 
                {
                    //only need to do this once on the last iter to
                    //set up the mbd delay axis (in frequency space)
                    auto mbd_ax = &(std::get<0>(fMBDWorkspace) );
                    for(std::size_t i=0; i<fNGridPoints;i++)
                    {
                        mbd_ax->at(i) = fGridStart + i*fGridSpace;
                    }
                }

                //now run an FFT along the MBD axis and cyclic rotate
                bool ok = fFFTEngine.Execute();
                check_step_fatal(ok, "calibration", "MBD search fft engine execution." << eom );
                
                std::size_t total_mbd_dr_size = fMBDWorkspace.GetSize();
                for(std::size_t i=0; i<total_mbd_dr_size; i++)
                {
                    //since we don't care about the actual amplitude (just searching for the max location)
                    //this is faster since it doesn't need to take a square root
                    double tmp_max = std::norm(fMBDWorkspace[i]);
                    if(tmp_max > maxmbd)
                    {
                        maxmbd = tmp_max;
                        fMBDMaxBin = (i + fNGridPoints/2) % fNGridPoints;
                        fSBDMaxBin = sbd_idx;
                        fDRMaxBin = dr_idx;
                    }
                }
                
                if(sbd_idx == fNSBD-1 && dr_idx == fNDR-1) 
                {
                    //only need to do this once on the last iter (to properly set-up the MBD axis)
                    ok = fCyclicRotator.Execute();
                    check_step_fatal(ok, "calibration", "MBD search cyclic rotation execution." << eom );
                }
            }
        }
        
        fMBDAxis = std::get<0>(fMBDWorkspace);
        fDRAxis = std::get<TIME_AXIS>(*in);
        
        return true;
    }

    return false;
};


}
