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
        fMBDWorkspace.Resize(fNGridPoints, fNDR);
        fMBDAmpWorkspace.Resize(fNGridPoints, fNDR);

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
        //loop over the single-band delay 'lags', computing the MBD/DR function
        //find the max for each SBD, and globally
        double maxmbd = 0.0;
        for(std::size_t sbd_idx=0; sbd_idx<fNSBD; sbd_idx++)
        {
            fMBDWorkspace.ZeroArray(); //zero out workspace
            fMBDAmpWorkspace.ZeroArray();

            if(sbd_idx == fNSBD-1) //only need to do this once on the last iter
            {
                //set up the mbd delay axis (in frequency space)
                auto mbd_ax = &(std::get<0>(fMBDWorkspace) );
                for(std::size_t i=0; i<fNGridPoints;i++)
                {
                    mbd_ax->at(i) = fGridStart + i*fGridSpace;
                }

                //set up the delay rate axis
                auto dr_ax = &(std::get<1>(fMBDWorkspace) );
                for(std::size_t i=0;i<fNDR;i++)
                {
                    dr_ax->at(i) = std::get<TIME_AXIS>(*in)(i);
                }
            }

            //copy in the data from each channel for this SDB/DR
            std::size_t nch = std::get<CHANNEL_AXIS>(*in).GetSize();
            for(std::size_t ch=0; ch<nch; ch++)
            {
                std::size_t mbd_bin = fMBDBinMap[ch];
                for(std::size_t dr_idx=0; dr_idx < fNDR; dr_idx++)
                {
                     fMBDWorkspace(mbd_bin, dr_idx) = (*in)(0, ch, dr_idx, sbd_idx);
                }
            }

            //now run an FFT along the MBD axis and cyclic rotate
            bool ok = fFFTEngine.Execute();
            check_step_fatal(ok, "calibration", "MBD search fft engine execution." << eom );
            ok = fCyclicRotator.Execute();
            check_step_fatal(ok, "calibration", "MBD search cyclic rotation execution." << eom );

            std::size_t total_mbd_dr_size = fMBDWorkspace.GetSize();
            for(std::size_t i=0; i<total_mbd_dr_size; i++)
            {
                //since we don't care about the actual amplitude (just searching for the max location)
                //this is faster since it doesn't need to take a square root
                // double a = std::real(fMBDWorkspace[i]);
                // double b = std::imag(fMBDWorkspace[i]);
                // fMBDAmpWorkspace[i] = a*a + b*b;
                // std::abs(fMBDWorkspace[i]);
                fMBDAmpWorkspace[i] = std::norm(fMBDWorkspace[i]);
            }

            //search for the peak amplitude in MBD and DR space
            fMaxSearch.SetArgs(&fMBDAmpWorkspace);
            fMaxSearch.Initialize();
            fMaxSearch.Execute();
            std::size_t max_loc = fMaxSearch.GetMaxLocation();
            auto loc_array = fMBDAmpWorkspace.GetIndicesForOffset(max_loc);
            double tmp_max = fMBDAmpWorkspace[max_loc];

            if(tmp_max > maxmbd)
            {
                maxmbd = tmp_max;
                fMBDMaxBin = loc_array[0];
                fSBDMaxBin = sbd_idx;
                fDRMaxBin = loc_array[1];
            }
        }
        return true;
    }

    return false;
};


}
