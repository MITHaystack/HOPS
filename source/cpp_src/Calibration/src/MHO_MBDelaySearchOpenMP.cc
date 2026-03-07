#include "MHO_MBDelaySearchOpenMP.hh"

namespace hops
{

MHO_MBDelaySearchOpenMP::MHO_MBDelaySearchOpenMP(): MHO_MBDelaySearch()
{
    fNThreads = 1;
}

MHO_MBDelaySearchOpenMP::~MHO_MBDelaySearchOpenMP(){};

bool MHO_MBDelaySearchOpenMP::InitializeImpl(const XArgType* in)
{
    fInitialized = false;
    if(in != nullptr)
    {
        //calculate the frequency grid for MBD search
        MHO_UniformGridPointsCalculator fGridCalc;

        std::vector< double > in_freq_pts(std::get< CHANNEL_AXIS >(*in).GetData(),
                                          std::get< CHANNEL_AXIS >(*in).GetData() + std::get< CHANNEL_AXIS >(*in).GetSize());
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

        //precompute flat channel -> MBD-bin lookup to avoid two std::map traversals per hot-loop iteration
        std::size_t nch_init = in_freq_pts.size();
        fMBDBinForChannel.resize(nch_init);
        for(std::size_t ch = 0; ch < nch_init; ch++)
        {
            fMBDBinForChannel[ch] = fMBDBinMap[fChannelIndexToFreqPointIndex[ch]];
        }

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

        fNDRSP = fDelayRateCalc.GetDelayRateSearchSpaceSize();
        fSearchBuffer.Resize(fNDRSP, fNGridPoints);
        fBatchedFFTEngine.SetArgs(&fSearchBuffer);
        fBatchedFFTEngine.DeselectAllAxes();
        fBatchedFFTEngine.SelectAxis(1); //FFT along MBD axis (axis 1); axis 0 is DR - runs as a batch
        fBatchedFFTEngine.SetForward();
        ok = fBatchedFFTEngine.Initialize();
        check_step_fatal(ok, "fringe", "MBD search batched fft engine initialization failed." << eom);

        //allocate and initialize per-thread workspaces for the OpenMP SBD loop.
        //all reserves happen before SetArgs so that element addresses are stable.
        fNThreads = omp_get_max_threads();
        msg_debug("calibration", "MHO_MBDelaySearchOpenMP will use: "<<fNThreads<<" OpenMP threads for the coarse search" << eom );
        fThreadMaxima.resize(fNThreads);
        fPerThreadSBDWorkspace.reserve(fNThreads);
        fPerThreadSBDDrData.reserve(fNThreads);
        fPerThreadDelayRateCalc.reserve(fNThreads);
        fPerThreadSearchBuffer.reserve(fNThreads);
        fPerThreadBatchedFFTEngine.reserve(fNThreads);

        for(int t = 0; t < fNThreads; t++)
        {
            fPerThreadSBDWorkspace.emplace_back();
            fPerThreadSBDDrData.emplace_back();
            fPerThreadDelayRateCalc.emplace_back();
            fPerThreadSearchBuffer.emplace_back();
            fPerThreadBatchedFFTEngine.emplace_back();
        }

        for(int t = 0; t < fNThreads; t++)
        {
            //SBD workspace - same dimensions and metadata as fSBDDrWorkspace
            fPerThreadSBDWorkspace[t].Resize(&(sbd_dims[0]));
            fPerThreadSBDWorkspace[t].ZeroArray();
            fPerThreadSBDWorkspace[t].CopyTags(*in);
            std::get< CHANNEL_AXIS >(fPerThreadSBDWorkspace[t]) = std::get< CHANNEL_AXIS >(*in);
            std::get< TIME_AXIS >(fPerThreadSBDWorkspace[t])    = std::get< TIME_AXIS >(*in);
            std::get< FREQ_AXIS >(fPerThreadSBDWorkspace[t])(0) = 0.0;

            //delay rate calculator bound to this thread's input/output buffers
            fPerThreadDelayRateCalc[t].SetReferenceFrequency(fRefFreq);
            fPerThreadDelayRateCalc[t].SetArgs(&fPerThreadSBDWorkspace[t], fWeights,
                                               &fPerThreadSBDDrData[t]);
            ok = fPerThreadDelayRateCalc[t].Initialize();
            check_step_fatal(ok, "fringe", "Per-thread delay rate search initialization failed." << eom);

            //per-thread [DR x MBD] search buffer and its batched FFT engine
            fPerThreadSearchBuffer[t].Resize(fNDRSP, fNGridPoints);
            fPerThreadBatchedFFTEngine[t].SetArgs(&fPerThreadSearchBuffer[t]);
            fPerThreadBatchedFFTEngine[t].DeselectAllAxes();
            fPerThreadBatchedFFTEngine[t].SelectAxis(1);
            fPerThreadBatchedFFTEngine[t].SetForward();
            ok = fPerThreadBatchedFFTEngine[t].Initialize();
            check_step_fatal(ok, "fringe", "Per-thread batched FFT initialization failed." << eom);
        }

        fInitialized = true;
    }

    return fInitialized;
}

bool MHO_MBDelaySearchOpenMP::ExecuteImpl(const XArgType* in)
{
    profiler_scope();
    bool ok;
    fMax = -0.0;
    if(fInitialized && fNSBD > 1)
    {
        fSBDAxis = std::get< FREQ_AXIS >(*in);
        fSBDBinSep = fSBDAxis(1) - fSBDAxis(0);
        fNPointsSearched = 0;
        std::size_t nch = std::get< CHANNEL_AXIS >(*in).GetSize();

        // Serial axis setup before the parallel region 

        //MBD axis: run the 1D FFT on a zeroed workspace with axis label transformation
        //to obtain the delay-space axis labels (data-independent, only grid geometry needed)
        fMBDWorkspace.ZeroArray();
        fFFTEngine.EnableAxisLabelTransformation();
        auto* mbd_ax = &(std::get< 0 >(fMBDWorkspace));
        for(std::size_t i = 0; i < fNGridPoints; i++) { (*mbd_ax)(i) = fGridStart + i * fGridSpace; }
        ok = fFFTEngine.Execute();
        check_step_fatal(ok, "fringe", "MBD search fft engine execution." << eom);
        fMBDAxis = std::get< 0 >(fMBDWorkspace);
        fFFTEngine.DisableAxisLabelTransformation();
        fMBDBinSep = fMBDAxis(1) - fMBDAxis(0);

        //DR axis: run the first SBD through the serial fDelayRateCalc to get fDRAxis
        {
            auto pre_dims = fSBDDrWorkspace.GetDimensionArray();
            for(std::size_t i = 0; i < pre_dims[CHANNEL_AXIS]; i++)
                for(std::size_t j = 0; j < pre_dims[TIME_AXIS]; j++)
                    fSBDDrWorkspace(0, i, j, 0) = (*in)(0, i, j, 0);
            ok = fDelayRateCalc.Execute();
            check_step_fatal(ok, "fringe", "Delay rate pre-pass execution failed." << eom);
            fDRAxis = std::get< TIME_AXIS >(sbd_dr_data);
            fDRAxis *= 1.0 / fRefFreq;
            fDRBinSep = fDRAxis(1) - fDRAxis(0);
        }

        // Reset per-thread argmax accumulators 
        for(int t = 0; t < fNThreads; t++)
        {
            fThreadMaxima[t].val      = -0.0;
            fThreadMaxima[t].mbd_bin  = -1;
            fThreadMaxima[t].sbd_bin  = -1;
            fThreadMaxima[t].dr_bin   = -1;
            fThreadMaxima[t].n_points = 0.0;
        }

        // Parallel loop over single-band delay lags 
        //#pragma omp parallel for schedule(dynamic) num_threads(fNThreads)
        //#pragma omp parallel for num_threads(fNThreads)
        #pragma omp parallel num_threads(fNThreads)
        {
            int tid = omp_get_thread_num();

            #pragma omp for schedule(static)
            for(std::size_t sbd_idx = 0; sbd_idx < fNSBD; sbd_idx++)
            {
                double sbd = fSBDAxis(sbd_idx);
                if(fSBDWinSet && !((fSBDWin[0] <= sbd) && (sbd <= fSBDWin[1]))) { continue; }

                //int tid = omp_get_thread_num();
                visibility_type&    local_ws  = fPerThreadSBDWorkspace[tid];
                visibility_type&    local_dr  = fPerThreadSBDDrData[tid];
                MHO_DelayRate&      local_drc = fPerThreadDelayRateCalc[tid];
                mbd_dr_type&        local_sb  = fPerThreadSearchBuffer[tid];
                FFT_2D_ENGINE_TYPE& local_fft = fPerThreadBatchedFFTEngine[tid];
                LocalMax&           lm        = fThreadMaxima[tid];

                //copy the SBD slice into the thread-local workspace
                auto dims = local_ws.GetDimensionArray();
                for(std::size_t i = 0; i < dims[CHANNEL_AXIS]; i++)
                    for(std::size_t j = 0; j < dims[TIME_AXIS]; j++)
                        local_ws(0, i, j, 0) = (*in)(0, i, j, sbd_idx);

                //transform to delay rate space
                local_drc.Execute();

                //zero the 2D [DR x MBD] buffer then scatter-accumulate all channels
                local_sb.ZeroArray();
                for(std::size_t dr_idx = 0; dr_idx < fNDRSP; dr_idx++)
                    for(std::size_t ch = 0; ch < nch; ch++)
                        local_sb(dr_idx, fMBDBinForChannel[ch]) += local_dr(0, ch, dr_idx, 0);

                //batched FFT over all DR slices at once
                local_fft.Execute();

                //search the 2D result - write only to thread-local lm, no synchronization needed
                for(std::size_t dr_idx = 0; dr_idx < fNDRSP; dr_idx++)
                {
                    double dr = fDRAxis(dr_idx);
                    if(fDRWinSet && !((fDRWin[0] <= dr) && (dr <= fDRWin[1]))) { continue; }
                    for(std::size_t mbd_idx = 0; mbd_idx < fNGridPoints; mbd_idx++)
                    {
                        double mbd = fMBDAxis(mbd_idx);
                        if(fMBDWinSet && !((fMBDWin[0] <= mbd) && (mbd <= fMBDWin[1]))) { continue; }
                        //std::norm avoids sqrt - searching for max location only
                        double tmp_max = std::norm(local_sb(dr_idx, mbd_idx));
                        if(tmp_max > lm.val)
                        {
                            lm.val     = tmp_max;
                            //index shift: cyclic rotator has not yet been applied to the MBD axis
                            lm.mbd_bin = (mbd_idx + fNGridPoints / 2) % fNGridPoints;
                            lm.sbd_bin = sbd_idx;
                            lm.dr_bin  = dr_idx;
                        }
                        lm.n_points += 1;
                    }
                }
            } // end omp parallel for
        } //end omp parallel nthreads

        // Serial argmax reduction across threads 
        for(int t = 0; t < fNThreads; t++)
        {
            if(fThreadMaxima[t].val > fMax)
            {
                fMax       = fThreadMaxima[t].val;
                fMBDMaxBin = fThreadMaxima[t].mbd_bin;
                fSBDMaxBin = fThreadMaxima[t].sbd_bin;
                fDRMaxBin  = fThreadMaxima[t].dr_bin;
            }
            fNPointsSearched += fThreadMaxima[t].n_points;
        }

        //apply cyclic rotation to finalise the MBD axis
        ok = fCyclicRotator.Execute();
        check_step_fatal(ok, "fringe", "MBD search cyclic rotation execution." << eom);
        fMBDAxis = std::get< 0 >(fMBDWorkspace);

        fMax = std::sqrt(fMax);
        if(fCoarseMBD >= 0 && fCoarseSBD >= 0 && fCoarseDR >= 0)
        {
            fCoarseMBD = fMBDAxis(fMBDMaxBin);
            fCoarseSBD = fSBDAxis(fSBDMaxBin);
            fCoarseDR  = fDRAxis(fDRMaxBin);
            
            return true;
        }
        else
        {
            msg_debug("calibration", "MHO_MBDelaySearchOpenMP failed to find fringe peak on this pass, max = " << fMax << eom);
        }
    }
    else
    {
        msg_error("calibration", "MHO_MBDelaySearchOpenMP could not execute, intialization failure." << eom);
    }
    
    return false;
};




} // namespace hops
