#include "MHO_MBDelaySearch2.hh"

namespace hops
{

MHO_MBDelaySearch2::MHO_MBDelaySearch2()
{
    fInitialized = false;
    fMBDMaxBin = -1;
    fSBDMaxBin = -1;
    fDRMaxBin = -1;
    fMBDBinMap.clear();
}

MHO_MBDelaySearch2::~MHO_MBDelaySearch2(){};


bool
MHO_MBDelaySearch2::InitializeImpl(const XArgType* in)
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
MHO_MBDelaySearch2::ExecuteImpl(const XArgType* in)
{
    bool ok;
    if(fInitialized)
    {
        //loop over the single-band delay 'lags', computing the MBD/DR function
        //find the max for each SBD, and globally
        double maxmbd = 0.0;
        for(std::size_t sbd_idx=0; sbd_idx<fNSBD; sbd_idx++)
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
            if(sbd_idx == 0){fDRAxis = std::get<TIME_AXIS>(sbd_dr_data);} //copy the axis just once

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
                check_step_fatal(ok, "fringe", "MBD search fft engine execution." << eom );

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
                    check_step_fatal(ok, "fringe", "MBD search cyclic rotation execution." << eom );
                    fMBDAxis = std::get<0>(fMBDWorkspace);
                }
            }
        }
        return true;
    }

    return false;
};


}
