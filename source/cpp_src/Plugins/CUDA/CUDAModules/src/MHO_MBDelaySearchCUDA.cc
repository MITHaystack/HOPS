#include "MHO_MBDelaySearchCUDA.hh"

namespace hops
{

MHO_MBDelaySearchCUDA::MHO_MBDelaySearchCUDA(): MHO_MBDelaySearch(){}

MHO_MBDelaySearchCUDA::~MHO_MBDelaySearchCUDA()
{
    cufftDestroy(fCUFFTPlan);
    cudaFree(fDeviceBuffer);
};

bool MHO_MBDelaySearchCUDA::InitializeImpl(const XArgType* in)
{
    fInitialized = false;
    if(in != nullptr)
    {
        //calculate the frequency grid for MBD search
        MHO_UniformGridPointsCalculator fGridCalc;
        std::vector< double> in_freq_pts(std::get<CHANNEL_AXIS>(*in).GetData(), 
                                         std::get<CHANNEL_AXIS>(*in).GetData() + std::get<CHANNEL_AXIS>(*in).GetSize() );
        std::vector< double > freq_pts;
        double freq_eps = 1e-4; //tolerance of 0.1kHz
        //dsb channel pairs share a sky_freq so we need combine them at the same location
        //this eliminates non-unique (within the tolerance) adjacent frequencies
        fGridCalc.GetUniquePoints(in_freq_pts, freq_eps, freq_pts, fChannelIndexToFreqPointIndex);
        fGridCalc.SetPoints(freq_pts);
        fGridCalc.Calculate();

        fGridStart = fGridCalc.GetGridStart();
        fGridSpace = fGridCalc.GetGridSpacing();
        fNGridPoints = fGridCalc.GetNGridPoints();
        fAverageFreq = fGridCalc.GetGridAverage();
        fMBDBinMap = fGridCalc.GetGridIndexMap();
        fNSBD = in->GetDimension(FREQ_AXIS);
        fNDR = in->GetDimension(TIME_AXIS);
        fNDRSP = fDelayRateCalc.CalculateSearchSpaceSize(fNDR);

        if(fSBDStart == -1)
        {
            fSBDStart = 0;
        }
        if(fSBDStop == -1)
        {
            fSBDStop = fNSBD;
        }

        msg_debug("fringe", "MBD search, N grid points = " << fNGridPoints << ", N delay-rate points = " << fNDR << eom);

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

        //initialize CUDA elements
        std::size_t nch = std::get< CHANNEL_AXIS >(*in).GetSize();
        fHostBuffer.Resize(fNDRSP, fNGridPoints);
        fHostBuffer.ZeroArray(); //TODO MOVE ME

        // Create a cuFFT fCUFFTPlan
        msg_debug("cuda", "creating cuda FFT plan of size: (" << fNGridPoints << ", " << fNDRSP << ")" << eom);
        cufftPlan1d(&fCUFFTPlan, fNGridPoints, CUFFT_Z2Z, fNDRSP); // 1D complex-to-complex FFT along the second dimension

        //Allocate device memory for the input data
        cudaMalloc((void**)&fDeviceBuffer, sizeof(cufftDoubleComplex) * fHostBuffer.GetSize());

        fInitialized = true;
    }

    return fInitialized;
}

bool MHO_MBDelaySearchCUDA::ExecuteImpl(const XArgType* in)
{
    bool ok;

    if(fInitialized && fNSBD > 1)
    {
        fSBDAxis = std::get< FREQ_AXIS >(*in);
        fSBDBinSep = fSBDAxis.at(1) - fSBDAxis.at(0);
        //loop over the single-band delay 'lags', computing the MBD/DR function
        //find the max for each SBD, and globally
        fMax = -0.0;
        fNPointsSearched = 0;
        bool first = true;
        for(std::size_t sbd_idx = 0; sbd_idx < fNSBD; sbd_idx++)
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
                for(std::size_t i = 0; i < a; i++)
                {
                    for(std::size_t j = 0; j < b; j++)
                    {
                        fSBDDrWorkspace(0, i, j, 0) = (*in)(0, i, j, sbd_idx);
                    }
                }

                //run the transformation to delay rate space (this also involves a zero padded FFT)
                ok = fDelayRateCalc.Execute();

                //copy and cache the delay-rate axis just once
                if(first)
                {
                    fDRAxis = std::get< TIME_AXIS >(sbd_dr_data); //upon retrieval this here is fringe-rate
                    fDRAxis *= 1.0 / fRefFreq;                    //now convert to delay rate by dividing by reference frequency
                    fDRBinSep = fDRAxis.at(1) - fDRAxis.at(0);
                }

                //zero out MBD workspace
                fMBDWorkspace.ZeroArray();

                //calculate the MBD axis
                if(first)
                {
                    //only need to do this once, in order to
                    //set up the mbd delay axis (in frequency space)
                    fFFTEngine.EnableAxisLabelTransformation();
                    auto mbd_ax = &(std::get< 0 >(fMBDWorkspace));
                    for(std::size_t i = 0; i < fNGridPoints; i++)
                    {
                        mbd_ax->at(i) = fGridStart + i * fGridSpace;
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
                    fMBDBinSep = fMBDAxis.at(1) - fMBDAxis.at(0);
                }

                std::size_t nch = std::get< CHANNEL_AXIS >(*in).GetSize();
                fHostBuffer.ZeroArray();

                //fill the 2d workspace
                for(std::size_t dr_idx = 0; dr_idx < fNDRSP; dr_idx++)
                {
                    for(std::size_t ch = 0; ch < nch; ch++)
                    {
                        //std::size_t mbd_bin = fMBDBinMap[ch];
                        std::size_t mbd_bin = fMBDBinMap[fChannelIndexToFreqPointIndex[ch]];
                        fHostBuffer(dr_idx, mbd_bin) = sbd_dr_data(0, ch, dr_idx, 0);
                    }
                }

                //copy to device
                cudaMemcpy(fDeviceBuffer, fHostBuffer.GetData(), sizeof(visibility_element_type) * fHostBuffer.GetSize(),
                           cudaMemcpyHostToDevice);

                //now run a batched FFT along the MBD axis
                cufftExecZ2Z(fCUFFTPlan, fDeviceBuffer, fDeviceBuffer, CUFFT_FORWARD);

                // Copy the result back to host
                cudaMemcpy(fHostBuffer.GetData(), fDeviceBuffer, sizeof(visibility_element_type) * fHostBuffer.GetSize(),
                           cudaMemcpyDeviceToHost);

                //search for the maximum TODO FIXME
                for(std::size_t dr_idx = 0; dr_idx < fNDRSP; dr_idx++)
                {
                    double dr = fDRAxis.at(dr_idx);
                    bool do_dr_search = (fDRWin[0] <= dr) && (dr <= fDRWin[1]);
                    if(!fDRWinSet || do_dr_search)
                    {
                        for(std::size_t mbd_idx = 0; mbd_idx < fNGridPoints; mbd_idx++)
                        {
                            double mbd = (fMBDAxis.at(mbd_idx));
                            bool do_mbd_search = (fMBDWin[0] <= mbd) && (mbd <= fMBDWin[1]);
                            if(!fMBDWinSet || do_mbd_search)
                            {
                                //since we don't care about the actual amplitude (just searching for the max location)
                                //this is faster since it doesn't need to take a square root
                                double tmp_max = std::norm(fHostBuffer.at(dr_idx, mbd_idx));
                                if(tmp_max > fMax)
                                {
                                    fMax = tmp_max;
                                    //index shift here is because we haven't applied the cyclic rotator to the mbd axis
                                    fMBDMaxBin = (mbd_idx + fNGridPoints / 2) % fNGridPoints;
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

        fCoarseMBD = fMBDAxis(fMBDMaxBin);
        fCoarseSBD = fSBDAxis(fSBDMaxBin);
        fCoarseDR = fDRAxis(fDRMaxBin);

        fMax = std::sqrt(fMax);
        return true;
    }
    else
    {
        msg_error("fringe", "MHO_MBDelaySearchCUDA could not execute, intialization failure." << eom);
    }

    return false;
};

} // namespace hops
