#include <map>
#include <set>
#include <vector>

#include "MHO_BasicFringeInfo.hh"
#include "MHO_BitReversalPermutation.hh"
#include "MHO_ComputePlotData.hh"
#include "MHO_CyclicRotator.hh"
#include "MHO_EndZeroPadder.hh"
#include "MHO_Reducer.hh"
#include "MHO_SelectRepack.hh"
#include "MHO_UniformGridPointsCalculator.hh"

#include "MHO_Constants.hh"

#include "MHO_LinearDParCorrection.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"

namespace hops
{

#ifdef HOPS_USE_FFTW3
using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransformFFTW< visibility_type >;
#else
using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransform< visibility_type >;
#endif

MHO_ComputePlotData::MHO_ComputePlotData()
{
    fMBDAnchor = "model";
    fParamStore = nullptr;
    fContainerStore = nullptr;
    fVisibilities = nullptr;
    fToolbox = nullptr;
    fWeights = nullptr;
    fSBDArray = nullptr;
    fImagUnit = MHO_Constants::imag_unit;
    fValid = false;
    fNChan = 0;
    fNAP = 0;
};

void MHO_ComputePlotData::Initialize()
{

    fTotalSummedWeights = fParamStore->GetAs< double >("/fringe/total_summed_weights");
    fRefFreq = fParamStore->GetAs< double >("/control/config/ref_freq");
    fMBDelay = fParamStore->GetAs< double >("/fringe/mbdelay");
    fDelayRate = fParamStore->GetAs< double >("/fringe/drate");
    fFringeRate = fParamStore->GetAs< double >("/fringe/frate");
    fSBDelay = fParamStore->GetAs< double >("/fringe/sbdelay");
    fSBDMaxBin = fParamStore->GetAs< double >("/fringe/max_sbd_bin");
    fAmp = fParamStore->GetAs< double >("/fringe/famp");

    fVisibilities = fContainerStore->GetObject< visibility_type >(std::string("vis"));
    fWeights = fContainerStore->GetObject< weight_type >(std::string("weight"));
    fSBDArray = fContainerStore->GetObject< visibility_type >(std::string("sbd"));
    fValid = true;

    if(fVisibilities == nullptr)
    {
        msg_error("fringe", "could not find visibility, object with name 'vis'." << eom);
    }

    if(fWeights == nullptr)
    {
        msg_error("fringe", "could not find visibility, object with name 'weight'." << eom);
        fValid = false;
    }

    if(fSBDArray == nullptr)
    {
        msg_error("fringe", "could not find visibility, object with name 'sbd'." << eom);
        fValid = false;
    }
}

void MHO_ComputePlotData::precompute_chan_metadata()
{
    auto chan_ax = &(std::get< CHANNEL_AXIS >(*fSBDArray));
    fNChan = fSBDArray->GetDimension(CHANNEL_AXIS);
    fNAP = fSBDArray->GetDimension(TIME_AXIS);

    fChanFreq.resize(fNChan);
    fChanSideband.resize(fNChan, 0);
    fChanBandwidth.resize(fNChan, 0.0);

    for(std::size_t ch = 0; ch < fNChan; ch++)
    {
        fChanFreq[ch] = (*chan_ax)(ch);

        std::string net_sideband = "?";
        bool key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", net_sideband);
        if(!key_present)
        {
            msg_error("fringe", "missing net_sideband label for channel " << ch << "." << eom);
        }

        int sb = 0; // default: DSB
        if(net_sideband == "U") { sb = 1; }
        if(net_sideband == "L") { sb = -1; }

        int dsb_partner = 0;
        bool dsb_key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "dsb_partner", dsb_partner);
        if(dsb_key_present) { sb = 0; }

        fChanSideband[ch] = sb;

        double bw = 0.0;
        chan_ax->RetrieveIndexLabelKeyValue(ch, "bandwidth", bw);
        fChanBandwidth[ch] = bw;
    }
}

void MHO_ComputePlotData::precompute_vr_tables()
{
    auto ap_ax = &(std::get< TIME_AXIS >(*fSBDArray));
    auto sbd_ax = &(std::get< FREQ_AXIS >(*fSBDArray));
    double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);
    double frt_offset = fParamStore->GetAs< double >("/config/frt_offset");

    fVRTable.resize(fNChan * fNAP);
    fVRMBD0Table.resize(fNChan * fNAP);
    fVRPhaseTable.resize(fNChan * fNAP);

    // Config 1: default SBD params (fNSBDBins=0, fSBDMaxBin=0, fSBDMax=0, fSBDSep=1).
    // This matches the fRot state used by calc_sbd and calc_dr, which are called before
    // calc_phase configures the SBD parameters.
    for(std::size_t ch = 0; ch < fNChan; ch++)
    {
        fRot.SetSideband(fChanSideband[ch]);
        double freq = fChanFreq[ch];
        for(std::size_t ap = 0; ap < fNAP; ap++)
        {
            double tdelta = (ap_ax->at(ap) + ap_delta / 2.0) - frt_offset;
            std::size_t idx = ch * fNAP + ap;
            fVRTable[idx] = fRot.vrot(tdelta, freq, fRefFreq, fDelayRate, fMBDelay);
            fVRMBD0Table[idx] = fRot.vrot(tdelta, freq, fRefFreq, fDelayRate, 0.0);
        }
    }

    // Config 2: proper SBD params (same values that calc_phase sets before its vrot calls).
    // This matches the fRot state used by calc_phase, calc_xpower_spec, calc_segs, and correct_vis.
    fRot.SetSBDSeparation(sbd_delta);
    fRot.SetSBDMaxBin(fSBDMaxBin);
    fRot.SetNSBDBins(sbd_ax->GetSize() / 2);
    fRot.SetSBDMax(fSBDelay);

    for(std::size_t ch = 0; ch < fNChan; ch++)
    {
        fRot.SetSideband(fChanSideband[ch]);
        double freq = fChanFreq[ch];
        for(std::size_t ap = 0; ap < fNAP; ap++)
        {
            double tdelta = (ap_ax->at(ap) + ap_delta / 2.0) - frt_offset;
            fVRPhaseTable[ch * fNAP + ap] = fRot.vrot(tdelta, freq, fRefFreq, fDelayRate, fMBDelay);
        }
    }
    // fRot is now left in Config 2 state, matching what calc_phase would have set.
}

xpower_amp_type MHO_ComputePlotData::calc_mbd()
{
    //calculate the frequency grid for MBD search
    MHO_UniformGridPointsCalculator fGridCalc;
    std::vector< double > in_freq_pts(std::get< CHANNEL_AXIS >(*fSBDArray).GetData(),
                                      std::get< CHANNEL_AXIS >(*fSBDArray).GetData() +
                                          std::get< CHANNEL_AXIS >(*fSBDArray).GetSize());
    std::vector< double > freq_pts;
    std::map< std::size_t, std::size_t > chan_index_map;
    double freq_eps = 1e-4; //tolerance of 0.1kHz
    //dsb channel pairs share a sky_freq so we need combine them at the same location
    //this eliminates non-unique (within the tolerance) adjacent frequencies
    fGridCalc.GetUniquePoints(in_freq_pts, freq_eps, freq_pts, chan_index_map);
    fGridCalc.SetPoints(freq_pts);
    fGridCalc.SetDefaultGridPoints(256);
    fGridCalc.Calculate();

    std::size_t fGridStart = fGridCalc.GetGridStart();
    double fGridSpace = fGridCalc.GetGridSpacing();
    std::size_t fNGridPoints = fGridCalc.GetNGridPoints();
    auto fMBDBinMap = fGridCalc.GetGridIndexMap();
    std::size_t fNSBD = fSBDArray->GetDimension(FREQ_AXIS);
    std::size_t fNDR = fSBDArray->GetDimension(TIME_AXIS);

    //resize workspaces
    fMBDWorkspace.Resize(fNGridPoints);
    fMBDWorkspace.ZeroArray();
    fMBDAmpWorkspace.Resize(fNGridPoints);
    fMBDAmpWorkspace.ZeroArray();

    auto mbd_ax = &(std::get< 0 >(fMBDWorkspace));
    for(std::size_t i = 0; i < fNGridPoints; i++)
    {
        mbd_ax->at(i) = fGridStart + i * fGridSpace;
    }

    //set up FFT and rotator engines
    fFFTEngine.SetArgs(&fMBDWorkspace);
    fFFTEngine.DeselectAllAxes();
    fFFTEngine.SelectAxis(0);
    fFFTEngine.SetForward();
    bool ok = fFFTEngine.Initialize();
    check_step_fatal(ok, "calibration", "MBD search fft engine initialization." << eom);

    fCyclicRotator.SetOffset(0, fNGridPoints / 2);
    fCyclicRotator.SetArgs(&fMBDWorkspace);
    ok = fCyclicRotator.Initialize();
    check_step_fatal(ok, "calibration", "MBD search cyclic rotation initialization." << eom);

    //now we are going to loop over all of the channels/AP
    //and perform the weighted sum of the data at the max-SBD bin
    //with the fitted delay-rate rotation (but mbd=0) applied

    //grab the total summed weights
    double total_summed_weights = 1.0;
    fWeights->Retrieve("total_summed_weights", total_summed_weights);

    std::size_t POLPROD = 0;
    std::size_t nchan = fSBDArray->GetDimension(CHANNEL_AXIS);
    std::size_t nap = fSBDArray->GetDimension(TIME_AXIS);

    // Use fVRMBD0Table: pre-computed w/ default SBD params and MBD=0, matching the original
    // per-call fRot state at the time calc_mbd executes (before calc_phase sets SBD params).
    std::complex< double > sum = 0;
    for(std::size_t ch = 0; ch < nchan; ch++)
    {
        sum = 0;
        for(std::size_t ap = 0; ap < nap; ap++)
        {
            std::complex< double > vis = (*fSBDArray)(POLPROD, ch, ap, fSBDMaxBin);
            double w = (*fWeights)(POLPROD, ch, ap, 0);
            sum += w * vis * fVRMBD0Table[ch * fNAP + ap];
        }
        //slot the summed data in at the appropriate location in the new grid
        std::size_t mbd_bin = fMBDBinMap[chan_index_map[ch]];
        fMBDWorkspace(mbd_bin) += sum; // += when adding to bin (to capture contribution from both halves of DSB channels)
    }

    //now run an FFT along the MBD axis and cyclic rotate
    ok = fFFTEngine.Execute();
    check_step_fatal(ok, "calibration", "MBD search fft engine execution." << eom);
    ok = fCyclicRotator.Execute();
    check_step_fatal(ok, "calibration", "MBD search cyclic rotation execution." << eom);

    for(std::size_t i = 0; i < fNGridPoints; i++)
    {
        fMBDAmpWorkspace[i] = std::abs(fMBDWorkspace[i]) / total_summed_weights;
        std::get< 0 >(fMBDAmpWorkspace).at(i) = std::get< 0 >(fMBDWorkspace).at(i);
    }

    return fMBDAmpWorkspace;
}

xpower_amp_type MHO_ComputePlotData::calc_sbd()
{

    //grab the total summed weights
    double total_summed_weights = 1.0;
    fWeights->Retrieve("total_summed_weights", total_summed_weights);

    xpower_type sbd_xpower_in;
    xpower_type sbd_xpower_out;
    xpower_amp_type sbd_amp;

    std::size_t POLPROD = 0;
    std::size_t nchan = fSBDArray->GetDimension(CHANNEL_AXIS);
    std::size_t nap = fSBDArray->GetDimension(TIME_AXIS);
    std::size_t nbins = fSBDArray->GetDimension(FREQ_AXIS);

    auto sbd_ax = &(std::get< FREQ_AXIS >(*fSBDArray));

    sbd_amp.Resize(nbins);
    sbd_xpower_in.Resize(nbins);
    sbd_xpower_out.Resize(4 * nbins); //interpolation

    sbd_xpower_in.ZeroArray();
    sbd_xpower_out.ZeroArray();

    // Use fVRTable: pre-computed w/ default SBD params and fMBDelay, matching the original
    // per-call fRot state at the time calc_sbd executes (before calc_phase sets SBD params).
    // vr depends only on (ch, ap), not on the bin index, so we look it up from the table.
    for(std::size_t i = 0; i < nbins; i++)
    {
        for(std::size_t ch = 0; ch < nchan; ch++)
        {
            std::complex< double > ch_sum = 0;
            for(std::size_t ap = 0; ap < nap; ap++)
            {
                std::complex< double > vis = (*fSBDArray)(POLPROD, ch, ap, i);
                double w = (*fWeights)(POLPROD, ch, ap, 0);
                ch_sum += w * vis * fVRTable[ch * fNAP + ap];
            }
            sbd_xpower_in(i) += ch_sum;
        }

        sbd_amp(i) = std::abs(sbd_xpower_in(i)) / total_summed_weights;
        std::get< 0 >(sbd_amp)(i) = (*sbd_ax)(i);
    }

    return sbd_amp;
}

phasor_type MHO_ComputePlotData::calc_segs()
{
    //grab the SBD max bin
    std::size_t max_sbd_bin = (std::size_t)fParamStore->GetAs< int >("/fringe/max_sbd_bin");

    std::size_t POLPROD = 0;
    std::size_t nchan = fSBDArray->GetDimension(CHANNEL_AXIS);
    std::size_t nap = fSBDArray->GetDimension(TIME_AXIS);

    auto chan_ax = &(std::get< CHANNEL_AXIS >(*fSBDArray));
    auto ap_ax = &(std::get< TIME_AXIS >(*fSBDArray));

    phasor_type phasor_segs;
    phasor_segs.Resize(nchan + 1, nap);
    phasor_segs.ZeroArray();

    //grab the fourfit channel names
    std::string chan_label_key = "channel_label";
    std::vector< std::string > channel_labels;
    for(std::size_t ch = 0; ch < chan_ax->GetSize(); ch++)
    {
        std::string ch_label;
        bool key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, chan_label_key, ch_label);
        if(key_present)
        {
            channel_labels.push_back(ch_label);
        }
        else
        {
            msg_warn("fringe", "unlabeled channel at index: " << ch << ", using '?' " << eom);
            channel_labels.push_back("?");
        }
    }

    // Use fVRPhaseTable: pre-computed w/ proper SBD params and fMBDelay, matching the fRot state
    // after calc_phase has run (calc_segs is called after calc_phase in DumpInfoToJSON).
    for(std::size_t ap = 0; ap < nap; ap++)
    {
        std::complex< double > sum = 0; //sum over all channels
        double sumwt = 0.0;
        for(std::size_t ch = 0; ch < nchan; ch++)
        {
            //make sure this plot gets the channel label:
            (&std::get< 0 >(phasor_segs))->InsertIndexLabelKeyValue(ch, chan_label_key, channel_labels[ch]);
            (&std::get< 0 >(phasor_segs))->at(ch) = fChanFreq[ch]; //set the channel frequency label
            std::complex< double > vis = (*fSBDArray)(POLPROD, ch, ap, max_sbd_bin);
            std::complex< double > z = vis * fVRPhaseTable[ch * fNAP + ap];
            phasor_segs(ch, ap) = z;

            //apply weight and sum (for 'All' channel)
            double w = (*fWeights)(POLPROD, ch, ap, 0);
            std::complex< double > wght_phsr = w * z;
            sum += wght_phsr;
            sumwt += w;
        }
        (&std::get< 1 >(phasor_segs))->at(ap) = ap_ax->at(ap); //set the ap label
        //add the sum over all channels
        phasor_segs(nchan, ap) = sum / sumwt;
        std::string all_chan_name = "All";
        (&std::get< 0 >(phasor_segs))->InsertIndexLabelKeyValue(nchan, chan_label_key, all_chan_name);
    }
    return phasor_segs;
}

void MHO_ComputePlotData::correct_vis()
{
    std::size_t POLPROD = 0;
    std::size_t nchan = fVisibilities->GetDimension(CHANNEL_AXIS);
    std::size_t nap = fVisibilities->GetDimension(TIME_AXIS);
    std::size_t nbins = fVisibilities->GetDimension(FREQ_AXIS);

    auto freq_ax = &(std::get< FREQ_AXIS >(*fVisibilities));

    // Pre-compute conj(sbd_rot)[ch][sp] = conj(exp(-2pi*i * sb_sign * (freq[sp] - bw/2) * fSBDelay)).
    // This depends on (ch, sp) but NOT on ap, eliminating one exp() call per (ap, ch, sp) from
    // the innermost loop (saving nap x nchan x (nbins-1) exp() evaluations).
    std::complex< double > imag_unit(0, 1);
    std::vector< std::complex< double > > sbd_phase_table(nchan * nbins);
    for(std::size_t ch = 0; ch < nchan; ch++)
    {
        double sb_sign = (double)fChanSideband[ch];
        double bw = fChanBandwidth[ch];
        for(std::size_t sp = 0; sp < nbins; sp++)
        {
            double theta = sb_sign * ((*freq_ax)[sp] - bw / 2.0) * fSBDelay;
            sbd_phase_table[ch * nbins + sp] = std::conj(std::exp(-2.0 * M_PI * imag_unit * theta));
        }
    }

    // Main loop: vr is a table lookup per (ap, ch); sbd_rot is a table lookup per (ch, sp).
    // fNChan/fNAP from fSBDArray match fVisibilities channel/AP dimensions.
    for(std::size_t ap = 0; ap < nap; ap++)
    {
        for(std::size_t ch = 0; ch < nchan; ch++)
        {
            std::complex< double > conj_vr = std::conj(fVRPhaseTable[ch * fNAP + ap]);
            for(std::size_t sp = 0; sp < nbins; sp++)
            {
                std::complex< double > vis = (*fVisibilities)(POLPROD, ch, ap, sp);
                (*fVisibilities)(POLPROD, ch, ap, sp) = sbd_phase_table[ch * nbins + sp] * conj_vr * vis;
            }
        }
    }

    //NOTE: for single linear pol-products fourfit applies a sign correction based on delta-parallactic angle
    //we should probably invert this correction here, because it results in a very confusing 180 sign flip
    //if we process the 'corrected' visibilities through fourfit once again.
    //So now -- we retrieve the MHO_LinearDParCorrection operator from the operator toolbox and apply it's inverse here

    if(fToolbox != nullptr)
    {
        std::string op_name = "dpar_corr";
        MHO_Operator* op = fToolbox->GetOperator(op_name);
        if(op != nullptr)
        {
            MHO_LinearDParCorrection* dpar_op = dynamic_cast< MHO_LinearDParCorrection* >(op);
            if(dpar_op != nullptr)
            {
                dpar_op->Execute(); //the nature of this operator is that if we execute it twice, it's effect is inverted
            }
        }
    }

    //Finally, we need to determine if the user asked to have the visibilities summed/reduced along
    //a particular axis (to condense the output), typical use case is either none, or along the time-axis
    int xpower_output = fParamStore->GetAs< int >("/cmdline/xpower_output");
    //do nothing, either we were not asked to attach this data to the output
    //or it does not need to be reduced in any way
    if(xpower_output == -1 || xpower_output == 0)
    {
        return;
    }
    if(0 < xpower_output && xpower_output <= 3)
    {
        MHO_Reducer< visibility_type, MHO_CompoundSum > vis_reducer;
        vis_reducer.SetArgs(fVisibilities);
        vis_reducer.ReduceAxis(xpower_output);
        bool init = vis_reducer.Initialize();
        bool exe = vis_reducer.Execute();

        MHO_Reducer< weight_type, MHO_CompoundSum > wt_reducer;
        wt_reducer.SetArgs(fWeights);
        wt_reducer.ReduceAxis(xpower_output);
        init = wt_reducer.Initialize();
        exe = wt_reducer.Execute();
    }
}

xpower_amp_type MHO_ComputePlotData::calc_dr()
{
    //grab the total summed weights
    double total_summed_weights = 1.0;
    fWeights->Retrieve("total_summed_weights", total_summed_weights);

    std::size_t POLPROD = 0;
    std::size_t nchan = fSBDArray->GetDimension(CHANNEL_AXIS);
    std::size_t nap = fSBDArray->GetDimension(TIME_AXIS);
    std::size_t drsp_size = 2 * MHO_BitReversalPermutation::NextLowestPowerOfTwo(nap); //see MHO_DelayRate.cc
    if(drsp_size < 256)
    {
        drsp_size = 256;
    } //Empirically determined from FRNGE plots (see line 72 of make_plotdata.c)

    ////////////////////////////////////////////////////////////////////////

    //resize workspaces (TODO...make conditional on current size -- if already configured)
    fDRWorkspace.Resize(drsp_size);
    fDRWorkspace.ZeroArray();
    fDRAmpWorkspace.Resize(drsp_size);
    fDRAmpWorkspace.ZeroArray();

    //set up FFT and rotator engines
    fFFTEngine.SetArgs(&fDRWorkspace);
    fFFTEngine.DeselectAllAxes();
    fFFTEngine.SelectAxis(0);
    fFFTEngine.SetForward();
    bool ok = fFFTEngine.Initialize();
    check_step_fatal(ok, "calibration", "MBD search fft engine initialization." << eom);

    fCyclicRotator.SetOffset(0, drsp_size / 2);
    fCyclicRotator.SetArgs(&fDRWorkspace);
    ok = fCyclicRotator.Initialize();
    check_step_fatal(ok, "calibration", "MBD search cyclic rotation initialization." << eom);

    auto ap_ax = &(std::get< TIME_AXIS >(*fSBDArray));
    double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    auto dr_ax = &(std::get< 0 >(fDRWorkspace));

    for(std::size_t i = 0; i < drsp_size; i++)
    {
        dr_ax->at(i) = i * ap_delta;
    }

    // Use fVRTable: pre-computed w/ default SBD params and fMBDelay, matching the original
    // per-call fRot state at the time calc_dr executes (before calc_phase sets SBD params).
    for(std::size_t ch = 0; ch < nchan; ch++)
    {
        for(std::size_t ap = 0; ap < nap; ap++)
        {
            std::complex< double > vis = (*fSBDArray)(POLPROD, ch, ap, fSBDMaxBin);
            double w = (*fWeights)(POLPROD, ch, ap, 0);
            fDRWorkspace(ap) += w * vis * fVRTable[ch * fNAP + ap];
        }
    }

    //now run an FFT along the MBD axis and cyclic rotate
    ok = fFFTEngine.Execute();
    check_step_fatal(ok, "calibration", "MBD search fft engine execution." << eom);
    ok = fCyclicRotator.Execute();
    check_step_fatal(ok, "calibration", "MBD search cyclic rotation execution." << eom);

    for(std::size_t i = 0; i < drsp_size; i++)
    {
        fDRAmpWorkspace[i] = std::abs(fDRWorkspace[i]) / total_summed_weights;
        TODO_FIXME_MSG("TODO FIXME, factor 1/1000 is due to need to plot axis in ns/s")
        std::get< 0 >(fDRAmpWorkspace).at(i) = (std::get< 0 >(fDRWorkspace).at(i)) / (fRefFreq / 1000.0);
    }

    return fDRAmpWorkspace;
}

double MHO_ComputePlotData::calc_phase()
{
    //grab the total summed weights
    double total_summed_weights = 1.0;
    fWeights->Retrieve("total_summed_weights", total_summed_weights);

    std::size_t POLPROD = 0;
    std::size_t nchan = fSBDArray->GetDimension(CHANNEL_AXIS);
    std::size_t nap = fSBDArray->GetDimension(TIME_AXIS);

    // Use fVRPhaseTable: pre-computed w/ proper SBD params and fMBDelay (the same SBD params
    // that calc_phase would set on fRot before calling vrot). The setter calls below are kept
    // for state consistency so fRot remains properly configured after this function.
    auto sbd_ax = &(std::get< FREQ_AXIS >(*fSBDArray));
    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);
    fRot.SetSBDSeparation(sbd_delta);
    fRot.SetSBDMaxBin(fSBDMaxBin);
    fRot.SetNSBDBins(sbd_ax->GetSize() / 2); //this is effective nlags
    fRot.SetSBDMax(fSBDelay);

    fFringe.Resize(nchan);

    std::complex< double > sum_all = 0.0;
    for(std::size_t ch = 0; ch < nchan; ch++)
    {
        std::get< 0 >(fFringe).at(ch) = fChanFreq[ch]; //set the fringe element freq label

        std::complex< double > fringe_phasor = 0.0;
        double sumwt = 0.0;
        for(std::size_t ap = 0; ap < nap; ap++)
        {
            std::complex< double > vis = (*fSBDArray)(POLPROD, ch, ap, fSBDMaxBin);
            double w = (*fWeights)(POLPROD, ch, ap, 0);
            std::complex< double > wght_phsr = w * vis * fVRPhaseTable[ch * fNAP + ap];
            sumwt += w;
            sum_all += wght_phsr;
            fringe_phasor += wght_phsr;
        }

        //set the fringe phasor
        //NOTE we have NOT applied the correction factor (see make_plotdata.c line 356)
        //c = (sumwt > 0.0) ? status.amp_corr_fact/sumwt : 0.0;
        fFringe[ch] = fringe_phasor / sumwt;
    }

    double coh_avg_phase = std::arg(sum_all);

    // std::cout<<"coh_avg_phase = "<<coh_avg_phase<<std::endl;

    return coh_avg_phase; //not quite the value which is displayed in the fringe plot (see fill type 208)
}

xpower_type MHO_ComputePlotData::calc_xpower_spec()
{
    //kludge version - this code is adapted from a combination of what is found in
    //make_plotdata.c and generate_graphs.c. It is extremely convoluted
    //and we ought to find a cleaner/clearer way to do the same thing

    //grab the total summed weights
    double total_summed_weights = 1.0;
    fWeights->Retrieve("total_summed_weights", total_summed_weights);

    int nlags = fSBDArray->GetDimension(FREQ_AXIS) / 2;
    int nl = nlags;

    xpower_type X;
    xpower_type Y;
    xpower_type cp_spectrum;
    xpower_type cp_spectrum_out;
    X.Resize(2 * nl);
    X.ZeroArray();
    Y.Resize(4 * nl);
    Y.ZeroArray();
    cp_spectrum.Resize(2 * nl);
    cp_spectrum.ZeroArray();

    std::size_t POLPROD = 0;
    std::size_t nchan = fSBDArray->GetDimension(CHANNEL_AXIS);
    std::size_t nap = fSBDArray->GetDimension(TIME_AXIS);
    std::size_t nbins = fSBDArray->GetDimension(FREQ_AXIS);

    //single channel xpower spectrum, need this info for the 'sbdbox' parameters
    //printed out on the fourfit plot
    std::vector< xpower_type > sbxsp;
    std::vector< int > maxlag;
    std::vector< double > maxlag_amp;
    std::vector< double > sbdbox;
    sbxsp.resize(nchan);
    maxlag.resize(nchan, 0);
    maxlag_amp.resize(nchan, 0.0);
    sbdbox.resize(nchan, 0.0);
    for(std::size_t ch = 0; ch < nchan; ch++)
    {
        sbxsp[ch].Resize(2 * nl);
    }

    //TODO FIXME...this is temporary, as it doesn't account for min_weight cuts
    std::vector< int > nusb_ap;
    nusb_ap.resize(nchan, 0);
    std::vector< int > nlsb_ap;
    nlsb_ap.resize(nchan, 0);

    auto sbd_ax = &(std::get< FREQ_AXIS >(*fSBDArray));
    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);

    std::complex< double > sum;
    std::complex< double > Z;
    double bw;
    //count the sidebands encountered
    int nusb = 0;
    int nlsb = 0;

    double total_usb_frac = 0;
    double total_lsb_frac = 0;

    // Use fVRPhaseTable: pre-computed w/ proper SBD params and fMBDelay, matching the fRot state
    // after calc_phase has run (calc_xpower_spec is called after calc_phase in DumpInfoToJSON).
    // nusb/nlsb counts and nusb_ap/nlsb_ap are filled on first lag iteration only.
    bool sideband_counts_done = false;
    for(int lag = 0; lag < 2 * nl; lag++)
    {
        total_usb_frac = 0;
        total_lsb_frac = 0;
        for(int ch = 0; ch < nchan; ch++)
        {
            int sb = fChanSideband[ch];
            bw = fChanBandwidth[ch];

            if(!sideband_counts_done)
            {
                if(sb == 1)
                {
                    nusb += 1;
                    nusb_ap[ch] = nap;
                }
                else if(sb == -1)
                {
                    nlsb += 1;
                    nlsb_ap[ch] = nap;
                }
            }

            sum = 0.0;
            for(int ap = 0; ap < nap; ap++)
            {
                std::complex< double > vis = (*fSBDArray)(POLPROD, ch, ap, lag);
                double w = (*fWeights)(POLPROD, ch, ap, 0);
                Z = vis * fVRPhaseTable[ch * fNAP + ap];

                if(sb == 1)
                {
                    total_usb_frac += w;
                }
                if(sb == -1)
                {
                    total_lsb_frac += w;
                }

                sum += w * Z;
            }
            sbxsp[ch].at(lag) = sum;
            X[lag] = X[lag] + sum;

            if(std::abs(sum) > maxlag_amp[ch])
            {
                maxlag_amp[ch] = std::abs(sum);
                maxlag[ch] = lag;
            }
        }
        sideband_counts_done = true;
        //need to understand this
        int j = lag - nl;
        if(j < 0)
        {
            j += 4 * nl;
        }
        if(lag == 0)
        {
            j = 2 * nl;
        } // pure real lsb/dc channel goes in middle??
        Y[j] = X[lag];
        std::get< 0 >(Y)(j) = j;
    }

    //set up FFT and run
    fFFTEngine.SetArgs(&Y); // &Y);
    fFFTEngine.DeselectAllAxes();
    fFFTEngine.SelectAxis(0);
    fFFTEngine.SetForward();
    bool ok = fFFTEngine.Initialize();
    ok = fFFTEngine.Execute();

    std::complex< double > cmplx_unit_I(0.0, 1.0);
    int s = Y.GetDimension(0) / 2;
    cp_spectrum.Resize(s);

    for(int i = 0; i < 2 * nl; i++)
    {
        int j = nl - i;
        double sbfactor = 0.0;
        if(j <= 0)
        {
            if(total_usb_frac > 0)
            {
                sbfactor = sqrt(0.5) / (M_PI * total_usb_frac);
            }
        }
        else
        {
            if(total_lsb_frac > 0)
            {
                sbfactor = sqrt(0.5) / (M_PI * total_lsb_frac);
            }
        }
        if(j < 0)
        {
            j += 4 * nl;
        }

        cp_spectrum[i] = Y[j] * sbfactor;
        Z = std::exp(-1.0 * cmplx_unit_I * (fSBDelay * (i - nl) * M_PI / (sbd_delta * 2.0 * nl)));
        cp_spectrum[i] = Z * cp_spectrum[i];
    }

    //now have to tweak the section exported to the plot data
    //depending on how many USB/LSB/DSB channels we have encountered
    double xstart;
    double xend;
    int ncp, izero;
    if(nusb > 0 && nlsb > 0) /* DSB */
    {
        xstart = -bw;
        xend = bw;
        ncp = 2 * nl;
        izero = 0;
    }
    else if(nlsb > 0) /* LSB only */
    {
        xstart = -bw;
        xend = 0.0;
        ncp = nl;
        izero = 0;
    }
    else /* USB only */
    {
        xstart = 0.0;
        xend = bw;
        ncp = nl;
        izero = nl;
    }

    cp_spectrum_out.Resize(ncp);

    for(int i = 0; i < ncp; i++)
    {
        double spec_axis_value = xstart + (xend - xstart) * i / ncp;
        std::get< 0 >(cp_spectrum_out)(i) = spec_axis_value;
        cp_spectrum_out[i] = cp_spectrum[i + izero];
    }

    //now compute the sbdbox for each channel
    double yy[3], q[3];
    double peak, maxv;
    for(int ch = 0; ch < nchan; ch++)
    {
        for(int i = 0; i < 3; i++)
        {
            //clamp idx so we cannot exceed [0, 2*nl-1]
            int idx = std::max(maxlag[ch] - 1 + i, 0);
            idx = std::min(idx, 2 * nl - 1);
            yy[i] = std::abs(sbxsp[ch].at(idx));
        }
        MHO_MathUtilities::parabola(yy, -1.0, 1.0, &peak, &maxv, q);
        sbdbox[ch] = maxlag[ch] + peak + 1;
    }
    //'this is the All-channel sbdbox
    sbdbox.push_back(nl + 1 + fSBDelay / sbd_delta);

    //copy this into the accessible arrays
    fSBDBox = sbdbox;
    fNLSBAP = nlsb_ap;
    fNUSBAP = nusb_ap;

    return cp_spectrum_out;
}

void MHO_ComputePlotData::calc_sbd_and_xpower_spec(xpower_amp_type& sbd_amp, xpower_type& cp_spectrum_out)
{
    double total_summed_weights = 1.0;
    fWeights->Retrieve("total_summed_weights", total_summed_weights);

    int nl = fSBDArray->GetDimension(FREQ_AXIS) / 2; // nlags; nbins = 2*nl
    std::size_t nbins = (std::size_t)(2 * nl);

    std::size_t POLPROD = 0;
    std::size_t nchan = fSBDArray->GetDimension(CHANNEL_AXIS);
    std::size_t nap = fSBDArray->GetDimension(TIME_AXIS);

    auto sbd_ax = &(std::get< FREQ_AXIS >(*fSBDArray));
    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);

    // - Initialize accumulators -
    xpower_type sbd_xpower_in, X, Y, cp_spectrum;
    sbd_xpower_in.Resize(nbins);  sbd_xpower_in.ZeroArray();
    X.Resize(2 * nl);             X.ZeroArray();
    Y.Resize(4 * nl);             Y.ZeroArray();
    cp_spectrum.Resize(2 * nl);   cp_spectrum.ZeroArray();
    sbd_amp.Resize(nbins);

    std::vector< xpower_type > sbxsp(nchan);
    std::vector< int > maxlag(nchan, 0);
    std::vector< double > maxlag_amp(nchan, 0.0);
    std::vector< double > sbdbox(nchan, 0.0);
    for(std::size_t ch = 0; ch < nchan; ch++)
    {
        sbxsp[ch].Resize(2 * nl);
        sbxsp[ch].ZeroArray();
    }

    // Count sidebands (used for output range and sbdbox)
    int nusb = 0, nlsb = 0;
    std::vector< int > nusb_ap(nchan, 0);
    std::vector< int > nlsb_ap(nchan, 0);
    for(std::size_t ch = 0; ch < nchan; ch++)
    {
        if(fChanSideband[ch] == 1)       { nusb++; nusb_ap[ch] = (int)nap; }
        else if(fChanSideband[ch] == -1) { nlsb++; nlsb_ap[ch] = (int)nap; }
    }

    // Pre-compute total_usb_frac and total_lsb_frac.
    // In the original calc_xpower_spec these were reset and re-summed for every lag, but the
    // weights do not depend on lag, so they are constants - compute once.
    double total_usb_frac = 0.0, total_lsb_frac = 0.0;
    for(std::size_t ch = 0; ch < nchan; ch++)
    {
        if(fChanSideband[ch] == 1)
        {
            for(std::size_t ap = 0; ap < nap; ap++) { total_usb_frac += (*fWeights)(POLPROD, ch, ap, 0); }
        }
        else if(fChanSideband[ch] == -1)
        {
            for(std::size_t ap = 0; ap < nap; ap++) { total_lsb_frac += (*fWeights)(POLPROD, ch, ap, 0); }
        }
    }

    // - Main merged loop: ch -> ap -> bin -
    // Inner bin loop accesses contiguous memory in fSBDArray (FREQ axis is innermost).
    // calc_sbd uses fVRTable (default SBD params); calc_xpower uses fVRPhaseTable (proper SBD params).
    for(std::size_t ch = 0; ch < nchan; ch++)
    {
        for(std::size_t ap = 0; ap < nap; ap++)
        {
            double w = (*fWeights)(POLPROD, ch, ap, 0);
            std::complex< double > wvr_sbd = w * fVRTable[ch * fNAP + ap];
            std::complex< double > wvr_xp  = w * fVRPhaseTable[ch * fNAP + ap];

            for(std::size_t i = 0; i < nbins; i++)
            {
                std::complex< double > vis = (*fSBDArray)(POLPROD, ch, ap, i);
                sbd_xpower_in(i) += wvr_sbd * vis; // for calc_sbd
                sbxsp[ch].at(i)  += wvr_xp  * vis; // for calc_xpower_spec (per-channel)
            }
        }

        // Accumulate into all-channel sum X and track per-channel maxlag
        for(std::size_t i = 0; i < nbins; i++)
        {
            X(i) += sbxsp[ch].at(i);
            if(std::abs(sbxsp[ch].at(i)) > maxlag_amp[ch])
            {
                maxlag_amp[ch] = std::abs(sbxsp[ch].at(i));
                maxlag[ch] = (int)i;
            }
        }
    }

    // - Finalize SBD amplitude -
    for(std::size_t i = 0; i < nbins; i++)
    {
        sbd_amp(i) = std::abs(sbd_xpower_in(i)) / total_summed_weights;
        std::get< 0 >(sbd_amp)(i) = (*sbd_ax)(i);
    }

    // - Construct Y from X (lag -> j rearrangement) -
    for(int lag = 0; lag < 2 * nl; lag++)
    {
        int j = lag - nl;
        if(j < 0) { j += 4 * nl; }
        if(lag == 0) { j = 2 * nl; } // pure real lsb/dc channel goes in middle
        Y(j) = X(lag);
        std::get< 0 >(Y)(j) = j;
    }

    // - FFT on Y -
    fFFTEngine.SetArgs(&Y);
    fFFTEngine.DeselectAllAxes();
    fFFTEngine.SelectAxis(0);
    fFFTEngine.SetForward();
    bool ok = fFFTEngine.Initialize();
    ok = fFFTEngine.Execute();

    // - Post-processing: sbfactor and SBD phase correction -
    std::complex< double > cmplx_unit_I(0.0, 1.0);
    int s = Y.GetDimension(0) / 2;
    cp_spectrum.Resize(s);

    for(int i = 0; i < 2 * nl; i++)
    {
        int j = nl - i;
        double sbfactor = 0.0;
        if(j <= 0)
        {
            if(total_usb_frac > 0) { sbfactor = sqrt(0.5) / (M_PI * total_usb_frac); }
        }
        else
        {
            if(total_lsb_frac > 0) { sbfactor = sqrt(0.5) / (M_PI * total_lsb_frac); }
        }
        if(j < 0) { j += 4 * nl; }
        cp_spectrum(i) = Y(j) * sbfactor;
        std::complex< double > Z = std::exp(-1.0 * cmplx_unit_I * (fSBDelay * (i - nl) * M_PI / (sbd_delta * 2.0 * nl)));
        cp_spectrum(i) = Z * cp_spectrum(i);
    }

    // - Determine USB/LSB/DSB output range -
    double bw = fChanBandwidth[nchan - 1]; // last channel bw, same as original (ch loop ends there)
    double xstart, xend;
    int ncp, izero;
    if(nusb > 0 && nlsb > 0)      { xstart = -bw; xend = bw;  ncp = 2 * nl; izero = 0; }
    else if(nlsb > 0)              { xstart = -bw; xend = 0.0; ncp = nl;     izero = 0; }
    else /* USB only or neither */ { xstart = 0.0; xend = bw;  ncp = nl;     izero = nl; }

    cp_spectrum_out.Resize(ncp);
    for(int i = 0; i < ncp; i++)
    {
        std::get< 0 >(cp_spectrum_out)(i) = xstart + (xend - xstart) * i / ncp;
        cp_spectrum_out(i) = cp_spectrum(i + izero);
    }

    // - Compute per-channel sbdbox (parabolic interpolation of peak lag) -
    double yy[3], q[3];
    double peak, maxv;
    for(int ch = 0; ch < (int)nchan; ch++)
    {
        for(int i = 0; i < 3; i++)
        {
            int idx = std::max(maxlag[ch] - 1 + i, 0);
            idx = std::min(idx, 2 * nl - 1);
            yy[i] = std::abs(sbxsp[ch].at(idx));
        }
        MHO_MathUtilities::parabola(yy, -1.0, 1.0, &peak, &maxv, q);
        sbdbox[ch] = maxlag[ch] + peak + 1;
    }
    sbdbox.push_back(nl + 1 + fSBDelay / sbd_delta); // 'All' channel sbdbox

    fSBDBox = sbdbox;
    fNLSBAP = nlsb_ap;
    fNUSBAP = nusb_ap;
}

xpower_amp_type MHO_ComputePlotData::calc_dr_segs_phase(double& coh_avg_phase, phasor_type& phasor_segs)
{
    double total_summed_weights = 1.0;
    fWeights->Retrieve("total_summed_weights", total_summed_weights);

    std::size_t POLPROD = 0;
    std::size_t nchan = fSBDArray->GetDimension(CHANNEL_AXIS);
    std::size_t nap = fSBDArray->GetDimension(TIME_AXIS);

    // - Setup for delay-rate spectrum (from calc_dr) -
    std::size_t drsp_size = 2 * MHO_BitReversalPermutation::NextLowestPowerOfTwo(nap);
    if(drsp_size < 256) { drsp_size = 256; }
    fDRWorkspace.Resize(drsp_size);
    fDRWorkspace.ZeroArray();
    fDRAmpWorkspace.Resize(drsp_size);
    fDRAmpWorkspace.ZeroArray();

    fFFTEngine.SetArgs(&fDRWorkspace);
    fFFTEngine.DeselectAllAxes();
    fFFTEngine.SelectAxis(0);
    fFFTEngine.SetForward();
    bool ok = fFFTEngine.Initialize();
    check_step_fatal(ok, "calibration", "MBD search fft engine initialization." << eom);

    fCyclicRotator.SetOffset(0, drsp_size / 2);
    fCyclicRotator.SetArgs(&fDRWorkspace);
    ok = fCyclicRotator.Initialize();
    check_step_fatal(ok, "calibration", "MBD search cyclic rotation initialization." << eom);

    auto ap_ax = &(std::get< TIME_AXIS >(*fSBDArray));
    double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    auto dr_ax = &(std::get< 0 >(fDRWorkspace));
    for(std::size_t i = 0; i < drsp_size; i++) { dr_ax->at(i) = i * ap_delta; }

    // - Setup for phasor segments (from calc_segs) -
    auto chan_ax = &(std::get< CHANNEL_AXIS >(*fSBDArray));
    std::string chan_label_key = "channel_label";
    std::vector< std::string > channel_labels;
    for(std::size_t ch = 0; ch < chan_ax->GetSize(); ch++)
    {
        std::string ch_label;
        bool key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, chan_label_key, ch_label);
        if(key_present) { channel_labels.push_back(ch_label); }
        else
        {
            msg_warn("fringe", "unlabeled channel at index: " << ch << ", using '?' " << eom);
            channel_labels.push_back("?");
        }
    }
    phasor_segs.Resize(nchan + 1, nap);
    phasor_segs.ZeroArray();

    // - Setup for fringe phase (from calc_phase) -
    auto sbd_ax = &(std::get< FREQ_AXIS >(*fSBDArray));
    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);
    // Keep setter calls for fRot state consistency (same values already set by precompute_vr_tables)
    fRot.SetSBDSeparation(sbd_delta);
    fRot.SetSBDMaxBin(fSBDMaxBin);
    fRot.SetNSBDBins(sbd_ax->GetSize() / 2);
    fRot.SetSBDMax(fSBDelay);
    fFringe.Resize(nchan);

    // Per-AP accumulators for the 'All' channel phasor
    std::vector< std::complex< double > > sum_per_ap(nap, 0.0);
    std::vector< double > sumwt_per_ap(nap, 0.0);

    // All-channel coherent sum (for calc_phase output)
    std::complex< double > sum_all = 0.0;

    // - Main merged loop: ch -> ap -
    // For each (ch, ap) at fSBDMaxBin, simultaneously fills:
    //   fDRWorkspace[ap]   (DR, uses fVRTable - default SBD params)
    //   phasor_segs[ch,ap] (segs, uses fVRPhaseTable - proper SBD params)
    //   fFringe[ch]        (phase, uses fVRPhaseTable)
    for(std::size_t ch = 0; ch < nchan; ch++)
    {
        // Set channel metadata in output arrays
        (&std::get< 0 >(phasor_segs))->InsertIndexLabelKeyValue(ch, chan_label_key, channel_labels[ch]);
        (&std::get< 0 >(phasor_segs))->at(ch) = fChanFreq[ch];
        std::get< 0 >(fFringe).at(ch) = fChanFreq[ch];

        std::complex< double > fringe_phasor = 0.0;
        double sumwt_ch = 0.0;

        for(std::size_t ap = 0; ap < nap; ap++)
        {
            std::complex< double > vis = (*fSBDArray)(POLPROD, ch, ap, fSBDMaxBin);
            double w = (*fWeights)(POLPROD, ch, ap, 0);

            // DR accumulation (fVRTable - default SBD params, same as original calc_dr)
            fDRWorkspace(ap) += w * vis * fVRTable[ch * fNAP + ap];

            // Segs and phase accumulation (fVRPhaseTable - proper SBD params)
            std::complex< double > z = vis * fVRPhaseTable[ch * fNAP + ap];
            phasor_segs(ch, ap) = z;

            std::complex< double > wz = w * z;
            sum_per_ap[ap]  += wz;
            sumwt_per_ap[ap] += w;
            fringe_phasor    += wz;
            sumwt_ch         += w;
            sum_all          += wz;
        }

        fFringe[ch] = fringe_phasor / sumwt_ch;
    }

    // - Finalize phasor_segs: AP axis labels and 'All' channel -
    std::string all_chan_name = "All";
    (&std::get< 0 >(phasor_segs))->InsertIndexLabelKeyValue(nchan, chan_label_key, all_chan_name);
    for(std::size_t ap = 0; ap < nap; ap++)
    {
        (&std::get< 1 >(phasor_segs))->at(ap) = ap_ax->at(ap);
        phasor_segs(nchan, ap) = (sumwt_per_ap[ap] > 0.0) ? sum_per_ap[ap] / sumwt_per_ap[ap] : 0.0;
    }

    // - Coherent average phase (calc_phase output) -
    coh_avg_phase = std::arg(sum_all);

    // - FFT and normalization for DR spectrum -
    ok = fFFTEngine.Execute();
    check_step_fatal(ok, "calibration", "MBD search fft engine execution." << eom);
    ok = fCyclicRotator.Execute();
    check_step_fatal(ok, "calibration", "MBD search cyclic rotation execution." << eom);

    for(std::size_t i = 0; i < drsp_size; i++)
    {
        fDRAmpWorkspace[i] = std::abs(fDRWorkspace[i]) / total_summed_weights;
        TODO_FIXME_MSG("TODO FIXME, factor 1/1000 is due to need to plot axis in ns/s")
        std::get< 0 >(fDRAmpWorkspace).at(i) = (std::get< 0 >(fDRWorkspace).at(i)) / (fRefFreq / 1000.0);
    }

    return fDRAmpWorkspace;
}

void MHO_ComputePlotData::DumpInfoToJSON(mho_json& plot_dict)
{
    if(!fValid)
    {
        msg_error("fringe", "cannot calculate plot data, data invalid" << eom);
        return;
    }

    precompute_chan_metadata();
    precompute_vr_tables();

    // Merged passes replace six separate bin/AP scans with two consolidated ones.
    xpower_amp_type sbd_amp;
    xpower_type sbd_xpower;
    calc_sbd_and_xpower_spec(sbd_amp, sbd_xpower); // single ch->ap->bin pass

    auto mbd_amp = calc_mbd();

    double coh_avg_phase;
    phasor_type phasors;
    auto dr_amp = calc_dr_segs_phase(coh_avg_phase, phasors); // single ch->ap pass at SBD max bin

    phasors.CopyTags(*fSBDArray);
    phasors.Insert("name", "phasors");
    std::get< 0 >(phasors).Insert("name", "channel");
    std::get< 0 >(phasors).Insert("units", "MHz");
    std::get< 1 >(phasors).Insert("name", "time");
    std::get< 1 >(phasors).Insert("units", "s");

    //have to clone the phasors data (because this copy will go out of scope at the end of this function)
    auto phasors_clone = phasors.Clone();
    bool ok = fContainerStore->AddObject(phasors_clone);
    ok = fContainerStore->SetShortName(phasors_clone->GetObjectUUID(), "phasors");

    // auto cvis = calc_corrected_vis();
    // cvis->CopyTags(*fSBDArray);
    // cvis->Insert("name", "corrected_visibilities");
    // ok = fContainerStore->AddObject(cvis);
    // ok = fContainerStore->SetShortName(cvis->GetObjectUUID(), "cvis");

    double coh_avg_phase_deg = std::fmod(coh_avg_phase * (180.0 / M_PI), 360.0);
    // std::cout<<"coh avg phase deg = "<<coh_avg_phase_deg<<std::endl;

    fParamStore->Set("/fringe/raw_resid_phase", coh_avg_phase_deg);

    //calculate AP period
    double ap_delta = std::get< TIME_AXIS >(*fVisibilities)(1) - std::get< TIME_AXIS >(*fVisibilities)(0);

    std::size_t npts = sbd_amp.GetSize();
    for(std::size_t i = 0; i < npts; i++)
    {
        plot_dict["SBD_AMP"].push_back(sbd_amp(i));
        plot_dict["SBD_AMP_XAXIS"].push_back(std::get< 0 >(sbd_amp)(i));
    }

    npts = mbd_amp.GetSize();
    for(std::size_t i = 0; i < npts; i++)
    {
        plot_dict["MBD_AMP"].push_back(mbd_amp(i));
        plot_dict["MBD_AMP_XAXIS"].push_back(std::get< 0 >(mbd_amp)(i));
    }

    npts = dr_amp.GetSize();
    for(std::size_t i = 0; i < npts; i++)
    {
        plot_dict["DLYRATE"].push_back(dr_amp(i));
        plot_dict["DLYRATE_XAXIS"].push_back(std::get< 0 >(dr_amp)(i));
    }

    npts = sbd_xpower.GetSize();
    for(std::size_t i = 0; i < npts; i++)
    {
        plot_dict["XPSPEC-ABS"].push_back(std::abs(sbd_xpower(i)));
        plot_dict["XPSPEC-ARG"].push_back(std::arg(sbd_xpower(i)) * (180.0 / M_PI));
        plot_dict["XPSPEC_XAXIS"].push_back(std::get< 0 >(sbd_xpower)(i));
    }

    //grab the fourfit channel labels
    std::string chan_label_key = "channel_label";
    std::vector< std::string > channel_labels;
    for(std::size_t ch = 0; ch < (&std::get< 0 >(phasors))->GetSize(); ch++)
    {
        std::string ch_label;
        bool key_present = (&std::get< 0 >(phasors))->RetrieveIndexLabelKeyValue(ch, chan_label_key, ch_label);
        if(key_present)
        {
            channel_labels.push_back(ch_label);
        }
    }

    plot_dict["ChannelsPlotted"] = channel_labels;

    //grab the per-channel/AP phasors, and average down if necessary
    std::size_t nplot = phasors.GetDimension(0);
    std::size_t naps = phasors.GetDimension(1);
    std::size_t nseg = naps;
    std::vector< std::vector< double > > seg_amp;
    seg_amp.resize(nplot);
    std::vector< std::vector< double > > seg_arg;
    seg_arg.resize(nplot);

    std::vector< double > ch_amp;
    std::vector< double > ch_arg;

    //use fourfit default method to determine how many APs to average together (see calc_rms.c)
    int ap_per_seg = fParamStore->GetAs< int >("/cmdline/ap_per_seg");
    std::size_t apseg;
    //if(nplot == 2){nplot = 1;}
    if(ap_per_seg == 0)
    {
        nseg = 200 / nplot; //max of 200 points across plot
        if(nseg > naps)
        {
            nseg = naps;
        }
        apseg = naps / nseg;
    }
    else
    {
        if(ap_per_seg > naps)
        {
            apseg = naps;
        }
        else
        {
            apseg = ap_per_seg;
        }
    }

    // Number of segments, starting at AP 0
    // and using integer apseg per segment
    nseg = naps / apseg;
    //Remainder goes into last segment (???)
    if((naps % apseg) != 0)
    {
        nseg += 1;
    }

    for(std::size_t i = 0; i < nplot; i++)
    {
        std::complex< double > ph = 0;
        std::complex< double > phsum = 0;
        for(std::size_t j = 0; j < naps; j++)
        {
            ph += phasors(i, j);
            phsum += phasors(i, j);
            if(j % apseg == apseg - 1 || j == naps - 1) //push the last one back
            {
                ph *= 1.0 / (double)apseg; //average
                seg_amp[i].push_back(std::abs(ph));
                seg_arg[i].push_back(std::arg(ph));
                ph = 0.0;
            }
        }
        phsum *= 1.0 / (double)naps;
        ch_amp.push_back(std::abs(phsum));
        ch_arg.push_back(std::arg(phsum));
    }

    //need to flatten and transpose this data to match the plot_data_dir format
    std::vector< double > transposed_flatted_seg_amp;
    std::vector< double > transposed_flatted_seg_arg;

    std::size_t xsize = seg_amp.size();
    std::size_t ysize = seg_amp[0].size();

    for(std::size_t j = 0; j < ysize; j++)
    {
        for(std::size_t i = 0; i < xsize; i++)
        {
            double amp = seg_amp[i][j];
            double arg = seg_arg[i][j];
            transposed_flatted_seg_amp.push_back(amp);
            transposed_flatted_seg_arg.push_back(arg);
        }
    }

    plot_dict["SEG_AMP"] = transposed_flatted_seg_amp;
    plot_dict["SEG_PHS"] = transposed_flatted_seg_arg;

    plot_dict["NSeg"] = nseg;
    plot_dict["NPlots"] = nplot; //nchan+1
    plot_dict["StartPlot"] = 0;

    //determine whether this is a pseudo-Stokes I fringe
    std::string polprod = std::get< POLPROD_AXIS >(*fVisibilities).at(0);
    bool is_pseudo_stokes_I = (polprod == "I");

    //for pseudo-Stokes I, derive the individual polarization characters at each station
    std::vector< std::string > ref_pols_vec, rem_pols_vec;
    if(is_pseudo_stokes_I)
    {
        std::vector< std::string > polprod_set;
        fParamStore->Get("/config/polprod_set", polprod_set);
        std::set< std::string > ref_set, rem_set;
        for(auto& pp : polprod_set)
        {
            ref_set.insert(std::string(1, pp[0]));
            rem_set.insert(std::string(1, pp[1]));
        }
        ref_pols_vec.assign(ref_set.begin(), ref_set.end()); //e.g. {"X","Y"}
        rem_pols_vec.assign(rem_set.begin(), rem_set.end()); //e.g. {"X","Y"}
        plot_dict["extra"]["ref_pols"] = ref_pols_vec;
        plot_dict["extra"]["rem_pols"] = rem_pols_vec;
    }

    //add the 'PLOT_INFO' section
    std::vector< std::string > pltheader;
    if(is_pseudo_stokes_I)
    {
        //pseudo-Stokes I: include second-pol keys, omit Tracks and Chan ids
        pltheader = {"#Ch",      "Freq(MHz)", "Phase",    "Ampl",     "SbdBox",   "APsRf",    "APsRm",
                     "PCdlyRf",  "PCdlyRf2",  "PCdlyRm",  "PCdlyRm2",
                     "PCPhsRf",  "PCPhsRm",   "PCPhsRf2", "PCPhsRm2",
                     "PCOffRf",  "PCOffRm",   "PCOffRf2", "PCOffRm2",
                     "PCAmpRf",  "PCAmpRf2",  "PCAmpRm",  "PCAmpRm2"};
    }
    else
    {
        pltheader = {"#Ch",     "Freq(MHz)", "Phase",   "Ampl",    "SbdBox",  "APsRf",   "APsRm",
                     "PCdlyRf", "PCdlyRm",   "PCPhsRf", "PCPhsRm", "PCOffRf", "PCOffRm", "PCAmpRf",
                     "PCAmpRm", "ChIdRf",    "TrkRf",   "ChIdRm",  "TrkRm"};
    }
    plot_dict["PLOT_INFO"]["header"] = pltheader;

    //includes the 'All' channel
    for(std::size_t i = 0; i < nplot; i++)
    {
        double freq = std::get< 0 >(phasors).at(i);
        plot_dict["PLOT_INFO"]["#Ch"].push_back(channel_labels[i]);
        plot_dict["PLOT_INFO"]["Freq(MHz)"].push_back(freq);
        plot_dict["PLOT_INFO"]["Phase"].push_back(ch_arg[i] * (180.0 / M_PI));
        plot_dict["PLOT_INFO"]["Ampl"].push_back(ch_amp[i]);
        plot_dict["PLOT_INFO"]["SbdBox"].push_back(fSBDBox[i]);
    }

    //just the normal channels (no 'All')
    for(std::size_t i = 0; i < nplot - 1; i++)
    {
        //the following two quanties are mis-named in the plot_data_dir file
        //but we'll make due for now, since we don't really compute these correctly yet...we ignore the min_weight parameter.
        //this is not AP's used by Ref station, but rather APs with USB data used
        plot_dict["PLOT_INFO"]["APsRf"].push_back(fNUSBAP[i]);
        //this is not AP's used by Rem station, but rather APs with LSB data used
        plot_dict["PLOT_INFO"]["APsRm"].push_back(fNLSBAP[i]);

        plot_dict["PLOT_INFO"]["PCdlyRf"].push_back(0.0);
        plot_dict["PLOT_INFO"]["PCdlyRm"].push_back(0.0);
        plot_dict["PLOT_INFO"]["PCPhsRf"].push_back(0.0);
        plot_dict["PLOT_INFO"]["PCPhsRm"].push_back(0.0);
        plot_dict["PLOT_INFO"]["PCOffRf"].push_back(0.0);
        plot_dict["PLOT_INFO"]["PCOffRm"].push_back(0.0);
        plot_dict["PLOT_INFO"]["PCAmpRf"].push_back(0.0);
        plot_dict["PLOT_INFO"]["PCAmpRm"].push_back(0.0);

        if(is_pseudo_stokes_I)
        {
            //initialize second-pol arrays
            plot_dict["PLOT_INFO"]["PCdlyRf2"].push_back(0.0);
            plot_dict["PLOT_INFO"]["PCdlyRm2"].push_back(0.0);
            plot_dict["PLOT_INFO"]["PCPhsRf2"].push_back(0.0);
            plot_dict["PLOT_INFO"]["PCPhsRm2"].push_back(0.0);
            plot_dict["PLOT_INFO"]["PCOffRf2"].push_back(0.0);
            plot_dict["PLOT_INFO"]["PCOffRm2"].push_back(0.0);
            plot_dict["PLOT_INFO"]["PCAmpRf2"].push_back(0.0);
            plot_dict["PLOT_INFO"]["PCAmpRm2"].push_back(0.0);
        }
        else
        {
            plot_dict["PLOT_INFO"]["ChIdRf"].push_back("-");
            plot_dict["PLOT_INFO"]["TrkRf"].push_back("-");
            plot_dict["PLOT_INFO"]["ChIdRm"].push_back("-");
            plot_dict["PLOT_INFO"]["TrkRm"].push_back("-");
        }
    }

    if(is_pseudo_stokes_I)
    {
        //export pcal data for both polarizations at each station
        //ref station, first pol (slot ""), second pol (slot "2")
        dump_multitone_pcmodel(plot_dict, 0, ref_pols_vec[0], "");
        dump_manual_pcmodel(plot_dict, 0, ref_pols_vec[0], "");
        dump_multitone_pcmodel(plot_dict, 0, ref_pols_vec[1], "2");
        dump_manual_pcmodel(plot_dict, 0, ref_pols_vec[1], "2");
        //remote station, first pol (slot ""), second pol (slot "2")
        dump_multitone_pcmodel(plot_dict, 1, rem_pols_vec[0], "");
        dump_manual_pcmodel(plot_dict, 1, rem_pols_vec[0], "");
        dump_multitone_pcmodel(plot_dict, 1, rem_pols_vec[1], "2");
        dump_manual_pcmodel(plot_dict, 1, rem_pols_vec[1], "2");
    }
    else
    {
        int station_flag;
        std::string pol;

        //export reference pcal stuff
        station_flag = 0;
        pol = polprod;
        if(polprod.size() == 2)
        {
            pol = std::string(1, polprod[station_flag]);
        }
        dump_multitone_pcmodel(plot_dict, station_flag, pol);
        dump_manual_pcmodel(plot_dict, station_flag, pol);

        //export remote pcal stuff
        station_flag = 1;
        pol = polprod;
        if(polprod.size() == 2)
        {
            pol = std::string(1, polprod[station_flag]);
        }
        dump_multitone_pcmodel(plot_dict, station_flag, pol);
        dump_manual_pcmodel(plot_dict, station_flag, pol);
    }

    double fringe_amp = fParamStore->GetAs< double >("/fringe/famp");
    double tsum_weights = fParamStore->GetAs< double >("/fringe/total_summed_weights");

    //we use the raw phase resid, because it is unaffected by the 'mbd_anchor' parameter
    double resid_phase = fParamStore->GetAs< double >("/fringe/raw_resid_phase_rad");

    double snr = fParamStore->GetAs< double >("/fringe/snr");
    double freqrms_phase, freqrms_amp, timerms_phase, timerms_amp, inc_avg_amp, inc_avg_amp_freq;
    calc_freqrms(phasors, resid_phase, fringe_amp, tsum_weights, snr, freqrms_phase, freqrms_amp, inc_avg_amp_freq);
    calc_timerms(phasors, nseg, apseg, resid_phase, fringe_amp, tsum_weights, snr, timerms_phase, timerms_amp, inc_avg_amp);

    //now export for the 'validity segment lines' info
    //need to flatten this data to match the plot_data_dir format
    std::vector< double > transposed_flatted_seg_usbfrac;
    std::vector< double > transposed_flatted_seg_lsbfrac;
    std::size_t asize = seg_frac_usb.size();
    std::size_t bsize = seg_frac_usb[0].size();
    for(std::size_t j = 0; j < asize; j++)
    {
        for(std::size_t i = 0; i < bsize; i++)
        {
            double usb = seg_frac_usb[j][i];
            double lsb = seg_frac_lsb[j][i];
            transposed_flatted_seg_usbfrac.push_back(usb);
            transposed_flatted_seg_lsbfrac.push_back(lsb);
        }
    }
    plot_dict["SEG_FRAC_USB"] = transposed_flatted_seg_usbfrac;
    plot_dict["SEG_FRAC_LSB"] = transposed_flatted_seg_lsbfrac;

    plot_dict["extra"]["freqrms_phase"] = freqrms_phase;
    plot_dict["extra"]["freqrms_amp"] = freqrms_amp;
    plot_dict["extra"]["timerms_phase"] = timerms_phase;
    plot_dict["extra"]["timerms_amp"] = timerms_amp;
    plot_dict["extra"]["inc_avg_amp"] = inc_avg_amp;
    plot_dict["extra"]["inc_avg_amp_freq"] = inc_avg_amp_freq;

    TODO_FIXME_MSG("TODO FIXME -- store the rest of these incoherent/theory parameters in the store")
    fParamStore->Set("/fringe/inc_avg_amp_freq", inc_avg_amp_freq);

    //stuff this in the parameter store too (reorganize this later)
    fParamStore->Set("/fringe/freqrms_phase", freqrms_phase);
    fParamStore->Set("/fringe/freqrms_amp", freqrms_amp);
    fParamStore->Set("/fringe/timerms_phase", timerms_phase);
    fParamStore->Set("/fringe/timerms_amp", timerms_amp);

    //calculate the theory values
    int nchan = fParamStore->GetAs< double >("/config/nchannels");
    double th_timerms_phase = MHO_BasicFringeInfo::calculate_theory_timerms_phase(nseg, snr);
    double th_timerms_amp = MHO_BasicFringeInfo::calculate_theory_timerms_amp(nseg, snr);
    double th_freqrms_phase = MHO_BasicFringeInfo::calculate_theory_freqrms_phase(nchan, snr);
    double th_freqrms_amp = MHO_BasicFringeInfo::calculate_theory_freqrms_amp(nchan, snr);

    plot_dict["extra"]["theory_timerms_phase"] = th_timerms_phase;
    plot_dict["extra"]["theory_timerms_amp"] = th_timerms_amp;
    plot_dict["extra"]["theory_freqrms_phase"] = th_freqrms_phase;
    plot_dict["extra"]["theory_freqrms_amp"] = th_freqrms_amp;

    //stuff theory values in the parameter store too
    fParamStore->Set("/fringe/theory_timerms_phase", th_timerms_phase);
    fParamStore->Set("/fringe/theory_timerms_amp", th_timerms_amp);
    fParamStore->Set("/fringe/theory_freqrms_phase", th_freqrms_phase);
    fParamStore->Set("/fringe/theory_freqrms_amp", th_freqrms_amp);

    //calculate the quality code
    std::string qc = calc_quality_code();
    fParamStore->Set("/fringe/quality_code", qc);
    plot_dict["Quality"] = qc;

    plot_dict["extra"]["nlags"] = fParamStore->GetAs< int >("/config/nlags");

    std::string errcode = calc_error_code(plot_dict);
    plot_dict["extra"]["error_code"] = errcode;
    fParamStore->Set("/fringe/error_code", errcode);

    //last thing is apply residual delay/delay rate correction to visibilities
    //in case we export them to the fringe file (-X option)
    correct_vis();
}

void MHO_ComputePlotData::calc_freqrms(phasor_type& phasors, double coh_avg_phase, double fringe_amp,
                                       double total_summed_weights, double snr, double& freqrms_phase, double& freqrms_amp,
                                       double& inc_avg_amp_freq)
{
    std::size_t nchan = phasors.GetDimension(0) - 1; //-1 is for the 'all' channel tacked on the end
    std::size_t nap = phasors.GetDimension(1);

    freqrms_phase = 0;
    freqrms_amp = 0;
    inc_avg_amp_freq = 0.0;

    TODO_FIXME_MSG("TODO FIXME -- implement amp_corr_fact.");
    double amp_corr_fact = 1.0;

    for(std::size_t ch = 0; ch < nchan; ch++)
    {
        std::complex< double > sum = 0;
        double sumwt = 0.0;
        for(std::size_t ap = 0; ap < nap; ap++)
        {
            double wt = (*fWeights)(0, ch, ap, 0);
            sum += wt * phasors(ch, ap);
            sumwt += wt;
        }
        inc_avg_amp_freq += std::abs(sum) * amp_corr_fact;

        if(sumwt == 0)
        {
            sum = 0.0;
        }
        else
        {
            sum /= sumwt;
        }

        double c = std::arg(sum) - coh_avg_phase;
        // condition to lie in [-pi,pi] interval
        //TODO FIXME -- this is the original implementation, but it is incorrect!
        TODO_FIXME_MSG("TODO FIXME, this way of computing an average phase angle is incorrect, should compute the average "
                       "vector first, then take the angle of that.")
        c = std::fmod(c, 2.0 * M_PI);
        if(c > M_PI)
        {
            c -= 2.0 * M_PI;
        }
        else if(c < -M_PI)
        {
            c += 2.0 * M_PI;
        }
        freqrms_phase += c * c;
        c = std::abs(sum) - fringe_amp;
        freqrms_amp += c * c;
    }

    if(nchan > 2)
    {
        // avoid 0/0 singularity
        freqrms_phase = std::sqrt(freqrms_phase / (nchan - 2)) * 180. / M_PI;
    }
    else
    {
        freqrms_phase = 0.0;
    }
    freqrms_amp = std::sqrt(freqrms_amp / nchan) * 100. / fringe_amp;

    inc_avg_amp_freq /= total_summed_weights;
    /* Noise bias correction ? */
    inc_avg_amp_freq /= ((1.0 + 1.0 / (2.0 * snr * snr / (double)nchan)));
}

void MHO_ComputePlotData::calc_timerms(phasor_type& phasors, std::size_t nseg, std::size_t apseg, double coh_avg_phase,
                                       double fringe_amp, double total_summed_weights, double snr, double& timerms_phase,
                                       double& timerms_amp, double& inc_avg_amp)
{
    TODO_FIXME_MSG("TODO FIXME -- implement amp_corr_fact.");
    double amp_corr_fact = 1.0;

    timerms_phase = 0;
    timerms_amp = 0;
    inc_avg_amp = 0.0;

    std::size_t nplot = phasors.GetDimension(0);
    std::size_t nchan = nplot - 1; //-1 is for the 'all' channel tacked on the end
    std::size_t naps = phasors.GetDimension(1);
    double totwt, totap, wt, wtf, wtf_dsb, wt_dsb, apwt, ap_in_seg, usbfrac, lsbfrac, c;
    totwt = 0.0;
    totap = 0.0;

    seg_frac_usb.resize(nseg);
    seg_frac_lsb.resize(nseg);
    std::complex< double > vsum, vsumf, wght_phsr;

    for(std::size_t seg = 0; seg < nseg; seg++)
    {
        seg_frac_usb[seg].resize(nchan,0);
        seg_frac_lsb[seg].resize(nchan,0);

        vsum = 0.0;
        wt = 0.0;     /* Loop over freqs, and ap's in segment */
        wt_dsb = 0.0; /* Loop over freqs, and ap's in segment */
                      /* forming vector sum */
        for(std::size_t fr = 0; fr < nchan; fr++)
        {
            vsumf = 0.0;
            wtf = 0.0;
            wtf_dsb = 0.0;
            ap_in_seg = 0.0;
            usbfrac = lsbfrac = 0.0;

            int sb = fChanSideband[fr]; // +1=USB, 0=DSB, -1=LSB

            for(std::size_t ap = seg * apseg; ap < (seg + 1) * apseg; ap++)
            {
                if(ap >= naps)
                {
                    break;
                }
                apwt = fabs((*fWeights)(0, fr, ap, 0));
                wt += apwt;
                wtf += apwt;

                if(sb == 1)
                {
                    usbfrac += apwt;
                    wtf_dsb += apwt;
                    wt_dsb += apwt;
                }
                else if(sb == -1)
                {
                    lsbfrac += apwt;
                    wtf_dsb += apwt;
                    wt_dsb += apwt;
                }

                if(apwt > 0.0)
                {
                    totap += 1.0;
                }
                wght_phsr = phasors(fr, ap) * apwt;
                vsum = vsum + wght_phsr;
                vsumf = vsumf + wght_phsr;
            }

            double usb_result =  (usbfrac >= 0.0) ? usbfrac / (double)apseg : 0.0;
            double lsb_result =  (lsbfrac >= 0.0) ? lsbfrac / (double)apseg : 0.0;
            seg_frac_usb[seg][fr] = usb_result; 
            seg_frac_lsb[seg][fr] = lsb_result;
        }

        c = std::arg(vsum) - coh_avg_phase;
        // condition to lie in [-pi,pi] interval
        c = std::fmod(c, 2.0 * M_PI);
        if(c > M_PI)
        {
            c -= 2.0 * M_PI;
        }
        else if(c < -M_PI)
        {
            c += 2.0 * M_PI;
        }
        timerms_phase += wt_dsb * c * c;
        /* Performs scalar sum over segments */
        /* of vector sums within segments and */
        /* over all freqs */
        c = std::abs(vsum);
        inc_avg_amp += c * amp_corr_fact;
        if(wt_dsb == 0)
        {
            c = 0.0;
        }
        else
        {
            c = c / wt_dsb;
        }
        /* delres_max is amplitude at peak */
        c = c * amp_corr_fact - fringe_amp;
        timerms_amp += wt_dsb * c * c;
        totwt += wt_dsb;
    }

    /* This removes noise bias based on */
    /* SNR of each segment/freq */
    inc_avg_amp /= ((1.0 + (float)nseg / (2.0 * snr * snr)));
    inc_avg_amp /= totwt;

    /* Correct rms values for fringe segmenting */
    timerms_phase = std::sqrt(timerms_phase / totwt) * 180. / M_PI;
    timerms_amp = sqrt(timerms_amp / totwt) * 100. / fringe_amp;
}

std::string MHO_ComputePlotData::calc_quality_code()
{
    //retrieve from param store
    double freqrms_phase = fParamStore->GetAs< double >("/fringe/freqrms_phase");
    double freqrms_amp = fParamStore->GetAs< double >("/fringe/freqrms_amp");
    double timerms_phase = fParamStore->GetAs< double >("/fringe/timerms_phase");
    double timerms_amp = fParamStore->GetAs< double >("/fringe/timerms_amp");

    double th_timerms_phase = fParamStore->GetAs< double >("/fringe/theory_timerms_phase");
    double th_timerms_amp = fParamStore->GetAs< double >("/fringe/theory_timerms_amp");
    double th_freqrms_phase = fParamStore->GetAs< double >("/fringe/theory_freqrms_phase");
    double th_freqrms_amp = fParamStore->GetAs< double >("/fringe/theory_freqrms_amp");

    char qcode[1];
    qcode[0] = '9';
    if(timerms_phase > 11.46 && th_timerms_phase < 5.73)
    {
        (*qcode)--;
    }
    if(timerms_phase > 22.92 && th_timerms_phase < 11.46)
    {
        (*qcode)--;
    }
    if(freqrms_phase > 11.46 && th_freqrms_phase < 5.73)
    {
        (*qcode)--;
    }
    if(freqrms_phase > 22.92 && th_freqrms_phase < 11.46)
    {
        (*qcode)--;
    }
    if(timerms_amp > 20.0 && th_timerms_amp < 10.0)
    {
        (*qcode)--;
    }
    if(timerms_amp > 40.0 && th_timerms_amp < 20.0)
    {
        (*qcode)--;
    }
    if(freqrms_amp > 20.0 && th_freqrms_amp < 10.0)
    {
        (*qcode)--;
    }
    if(freqrms_amp > 40.0 && th_freqrms_amp < 20.0)
    {
        (*qcode)--;
    }
    if(*qcode < '1')
        *qcode = '1';

    double prob_false = fParamStore->GetAs< double >("/fringe/prob_false_detect");
    // No fringes, 0-code
    if(prob_false > 1.E-4)
        *qcode = '0';
    return std::string(1, qcode[0]);
}

std::string MHO_ComputePlotData::calc_error_code(const mho_json& plot_dict)
{

    TODO_FIXME_MSG("TODO FIXME -- implement error codes other than G and H, also move all param retrieval outside of this "
                   "function and pass in data")

    std::string errcode = " "; //default

    double weak_channel;
    bool ok = fParamStore->Get("/control/fit/weak_channel", weak_channel);
    if(!ok)
    {
        weak_channel = 0.5;
    } //default weak_channel threshold is 0.5

    double pc_amp_hcode;
    ok = fParamStore->Get("/control/fit/pc_amp_hcode", pc_amp_hcode);
    if(!ok)
    {
        pc_amp_hcode = 0.005;
    } //default pc_amp_hcode is 0.005

    double snr;
    ok = fParamStore->Get("/fringe/snr", snr);

    double inc_avg_amp_freq;
    ok = fParamStore->Get("/fringe/inc_avg_amp_freq", inc_avg_amp_freq);

    std::string ref_mk4id = fParamStore->GetAs< std::string >("/ref_station/mk4id");
    std::string rem_mk4id = fParamStore->GetAs< std::string >("/rem_station/mk4id");

    //pass the pcal mode that was used
    //check pc_mode values to see if this operator should be built at all (defaults to true)
    //first we check if there is a 'pc_mode' defined under '/control/station/pc_mode'
    std::string generic_pc_mode = "manual";
    if(fParamStore->IsPresent("/control/station/pc_mode"))
    {
        //load possible generic setting
        generic_pc_mode = fParamStore->GetAs< std::string >("/control/station/pc_mode");
    }
    std::string ref_pc_mode = generic_pc_mode;
    std::string rem_pc_mode = generic_pc_mode;

    //override with any station specific parameters
    std::string ref_station_pcmode_path = std::string("/control/station/") + ref_mk4id + "/pc_mode";
    if(fParamStore->IsPresent(ref_station_pcmode_path))
    {
        ref_pc_mode = fParamStore->GetAs< std::string >(ref_station_pcmode_path);
    }

    //override with any station specific parameters
    std::string rem_station_pcmode_path = std::string("/control/station/") + rem_mk4id + "/pc_mode";
    if(fParamStore->IsPresent(ref_station_pcmode_path))
    {
        rem_pc_mode = fParamStore->GetAs< std::string >(rem_station_pcmode_path);
    }

    //Channel < half of mean for G-code
    bool low_chan = false;

    //need fringe phasor data and pc_amp data
    std::size_t nchan = fFringe.GetDimension(0);
    for(std::size_t i = 0; i < nchan; i++)
    {
        //std::cout<<"comparing fringe amp @"<<i<<": "<<fFringe[i]<< " ? " <<(weak_channel * inc_avg_amp_freq)<<std::endl;
        if(std::abs(fFringe[i]) < (weak_channel * inc_avg_amp_freq))
        {
            low_chan = true;
            break;
        }
    }

    /* G-code means a weak channel when SNR>20 */
    if(low_chan && snr > 20.0)
    {
        errcode = "G";
    }

    //get the collection of pol-products used in this fringe
    std::vector< std::string > polprod_set;
    fParamStore->Get("/config/polprod_set", polprod_set);

    //figure out the reference and remote station pol sets
    std::set< std::string > ref_pols;
    std::set< std::string > rem_pols;
    for(std::size_t i = 0; i < polprod_set.size(); i++)
    {
        ref_pols.insert(std::string(1, polprod_set[i][0]));
        rem_pols.insert(std::string(1, polprod_set[i][1]));
    }

    auto chan_ax = &(std::get< CHANNEL_AXIS >(*fSBDArray));

    bool ref_low_pcal = false;
    bool rem_low_pcal = false;
    for(std::size_t ch = 0; ch < nchan; ch++)
    {
        bool chan_ref_pcal_low = true;
        bool chan_rem_pcal_low = true;
        //0 = ref, 1 = rem
        for(auto pol_iter = ref_pols.begin(); pol_iter != ref_pols.end(); pol_iter++)
        {
            std::string pol = *pol_iter;
            //workspace for segment retrieval
            std::vector< double > pc_mag_segs;
            std::string pc_mag_key = "ref_mtpc_mag_" + pol;

            bool b1 = chan_ax->RetrieveIndexLabelKeyValue(ch, pc_mag_key, pc_mag_segs);
            if(b1)
            {
                double pc_amp = MHO_MathUtilities::average(pc_mag_segs);
                if(pc_amp < pc_amp_hcode || pc_amp > 0.500)
                {
                    chan_ref_pcal_low &= true;
                }
                else
                {
                    chan_ref_pcal_low &= false;
                }
            }
        }

        for(auto pol_iter = rem_pols.begin(); pol_iter != rem_pols.end(); pol_iter++)
        {
            std::string pol = *pol_iter;
            //workspace for segment retrieval
            std::vector< double > pc_mag_segs;
            std::string pc_mag_key = "rem_mtpc_mag_" + pol;
            bool b1 = chan_ax->RetrieveIndexLabelKeyValue(ch, pc_mag_key, pc_mag_segs);
            if(b1)
            {
                double pc_amp = MHO_MathUtilities::average(pc_mag_segs);
                if(pc_amp < pc_amp_hcode || pc_amp > 0.500)
                {
                    chan_rem_pcal_low &= true;
                }
                else
                {
                    chan_rem_pcal_low &= false;
                }
            }
        }
        if(chan_ref_pcal_low)
        {
            ref_low_pcal = true;
        }
        if(chan_rem_pcal_low)
        {
            rem_low_pcal = true;
        }
    }

    //only care about 'multitone', so far 'normal' pc_mode has not been implemented
    if(ref_low_pcal && ref_pc_mode == "multitone")
    {
        errcode = "H";
    }
    if(rem_low_pcal && rem_pc_mode == "multitone")
    {
        errcode = "H";
    }

    if(errcode != " ")
    {
        msg_info("fringe", "fringe condition assigned error code: " << errcode << eom);
    }

    return errcode;
}

void MHO_ComputePlotData::dump_multitone_pcmodel(mho_json& plot_dict,
                                                 int station_flag,        //0 = reference station, 1 = remote station
                                                 std::string pol,         //single char string
                                                 std::string key_suffix   //appended to PLOT_INFO keys, e.g. "2" for second pol
)
{
    //workspace for segment retrieval
    std::vector< double > pc_mag_segs;
    std::vector< double > pc_phase_segs;
    std::vector< double > pc_delay_segs;
    double man_pc_phase;

    auto chan_ax = &(std::get< CHANNEL_AXIS >(*fSBDArray));

    std::string pc_mag_key;
    std::string pc_phase_key;
    std::string pc_delay_key;

    std::string manual_pc_phase_key;

    if(station_flag == 0)
    {
        pc_mag_key = "ref_mtpc_mag_";
        pc_phase_key = "ref_mtpc_phase_";
        pc_delay_key = "ref_mtpc_delays_";
        manual_pc_phase_key = "ref_pcphase_";
    }

    if(station_flag == 1)
    {
        pc_mag_key = "rem_mtpc_mag_";
        pc_phase_key = "rem_mtpc_phase_";
        pc_delay_key = "rem_mtpc_delays_";
        manual_pc_phase_key = "rem_pcphase_";
    }

    pc_mag_key += pol;
    pc_phase_key += pol;
    pc_delay_key += pol;
    manual_pc_phase_key += pol;

    double sgn = 1.0; //does this need to flip depending on LSB/USB?

    //fourfit applies the ion dTEC to the multitone pc-phase cal
    //so we need to rotate them all by an appropriate amount
    //otherwise the fringe plots won't match
    //TODO FIXME -- decide if this is the behavior that we actually want
    double ion_diff = 0;
    double ion_k = MHO_Constants::ion_k;
    bool bion = fParamStore->Get("/fringe/ion_diff", ion_diff);
    if(!bion)
    {
        ion_diff = 0.0;
    }

    // //split ion differential phase between both ref and rem stations
    // //note the sign is opposite the typical convention (see pcalibrate.c line 144)
    if(station_flag == 0)
    {
        ion_diff = -0.5 * ion_diff;
    }
    if(station_flag == 1)
    {
        ion_diff = 0.5 * ion_diff;
    }

    //extract the multitone pcal model attached to the visibilities
    for(std::size_t ch = 0; ch < chan_ax->GetSize(); ch++)
    {
        //get channel's frequency info
        double sky_freq = chan_ax->at(ch); //not quite right, we want channel center freq!!
        double bandwidth = 0;
        std::string net_sideband;

        bool key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", net_sideband);
        if(!key_present)
        {
            msg_error("fringe", "missing net_sideband label for channel " << ch << "." << eom);
        }
        key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "bandwidth", bandwidth);
        if(!key_present)
        {
            msg_error("fringe", "missing bandwidth label for channel " << ch << "." << eom);
        }

        double lower_freq;
        double upper_freq;
        if(net_sideband == "U")
        {
            lower_freq = sky_freq;
            upper_freq = sky_freq + bandwidth;
        }
        else //lower sideband
        {
            upper_freq = sky_freq;
            lower_freq = sky_freq - bandwidth;
        }
        double center_freq = 0.5 * (upper_freq + lower_freq);

        bool b1 = chan_ax->RetrieveIndexLabelKeyValue(ch, pc_mag_key, pc_mag_segs);
        bool b2 = chan_ax->RetrieveIndexLabelKeyValue(ch, pc_phase_key, pc_phase_segs);
        bool b3 = chan_ax->RetrieveIndexLabelKeyValue(ch, pc_delay_key, pc_delay_segs);
        bool b4 = chan_ax->RetrieveIndexLabelKeyValue(ch, manual_pc_phase_key, man_pc_phase);

        if(b1)
        {
            double ave_pc_mag = MHO_MathUtilities::average(pc_mag_segs);
            if(station_flag == 0)
            {
                plot_dict["PLOT_INFO"]["PCAmpRf" + key_suffix][ch] = ave_pc_mag * 1000.0; //convert to fourfit units (?)
            }
            if(station_flag == 1)
            {
                plot_dict["PLOT_INFO"]["PCAmpRm" + key_suffix][ch] = ave_pc_mag * 1000.0;
            }
        }

        if(b2)
        {
            double ave_pc_phase = MHO_MathUtilities::angular_average(pc_phase_segs);

            if(b4) //rotate all of the multitone phases by the manual applied phase
            {
                std::complex< double > man_phasor = std::exp(sgn * fImagUnit * man_pc_phase);
                double theta_ion = theta_ion = ion_k * ion_diff / (1e6 * center_freq);
                std::complex< double > ion_phasor = std::exp(fImagUnit * theta_ion);

                // std::cout<<"ch:"<<ch<<" phase = "<<manual_pc_phase_key<<" = "<<man_pc_phase<<std::endl;
                for(std::size_t j = 0; j < pc_phase_segs.size(); j++)
                {
                    std::complex< double > phasor = std::exp(fImagUnit * pc_phase_segs[j]);
                    phasor *= man_phasor;
                    //phasor *= ion_phasor;
                    pc_phase_segs[j] = std::arg(phasor);
                }
            }

            if(station_flag == 0)
            {
                //convert to degrees
                plot_dict["PLOT_INFO"]["PCPhsRf" + key_suffix][ch] = ave_pc_phase * (180. / M_PI);
                for(std::size_t j = 0; j < pc_phase_segs.size(); j++)
                {
                    pc_phase_segs[j] *= (180. / M_PI);
                }
                plot_dict["extra"]["ref_mtpc_phase_segs" + key_suffix].push_back(pc_phase_segs);
            }
            if(station_flag == 1)
            {
                plot_dict["PLOT_INFO"]["PCPhsRm" + key_suffix][ch] = ave_pc_phase * (180. / M_PI);
                for(std::size_t j = 0; j < pc_phase_segs.size(); j++)
                {
                    pc_phase_segs[j] *= (180. / M_PI);
                }
                plot_dict["extra"]["rem_mtpc_phase_segs" + key_suffix].push_back(pc_phase_segs);
            }
        }

        if(b3)
        {
            double ave_pc_delay = MHO_MathUtilities::average(pc_delay_segs);
            if(station_flag == 0)
            {
                plot_dict["PLOT_INFO"]["PCdlyRf" + key_suffix][ch] = ave_pc_delay * 1e9; //convert to ns
            }
            if(station_flag == 1)
            {
                plot_dict["PLOT_INFO"]["PCdlyRm" + key_suffix][ch] = ave_pc_delay * 1e9;
            }
        }
    }
}

void MHO_ComputePlotData::dump_manual_pcmodel(mho_json& plot_dict,
                                              int station_flag,        //0 = reference station, 1 = remote station
                                              std::string pol,         //single char string
                                              std::string key_suffix   //appended to PLOT_INFO keys, e.g. "2" for second pol
)
{
    //workspace for segment retrieval
    auto chan_ax = &(std::get< CHANNEL_AXIS >(*fSBDArray));

    std::string pc_mag_key;
    std::string pc_phase_key;
    std::string pc_delay_key;

    if(station_flag == 0)
    {
        pc_phase_key = "ref_pcphase_";
    }
    if(station_flag == 1)
    {
        pc_phase_key = "rem_pcphase_";
    }
    pc_phase_key += pol;

    //extract the manual pcphases pcal model attached to the visibilities
    for(std::size_t ch = 0; ch < chan_ax->GetSize(); ch++)
    {
        double pc_phase;
        bool b1 = chan_ax->RetrieveIndexLabelKeyValue(ch, pc_phase_key, pc_phase);
        if(b1)
        {
            if(station_flag == 0)
            {
                plot_dict["PLOT_INFO"]["PCOffRf" + key_suffix][ch] = pc_phase * (180. / M_PI); //convert to fourfit units (?)
            }
            if(station_flag == 1)
            {
                plot_dict["PLOT_INFO"]["PCOffRm" + key_suffix][ch] = pc_phase * (180. / M_PI);
            }
        }
    }
}

void MHO_ComputePlotData::DumpSpectralLineInfoToJSON(mho_json& plot_dict)
{
    // Fetch the spec-DR workspace stored by MHO_SpectralLineFringeFitter::Finalize().
    visibility_type* spec_dr = fContainerStore->GetObject< visibility_type >(std::string("spec_dr"));
    if(spec_dr == nullptr)
    {
        msg_error("fringe", "DumpSpectralLineInfoToJSON: 'spec_dr' container not found in store." << eom);
        return;
    }

    if(fVisibilities == nullptr)
    {
        msg_error("fringe", "DumpSpectralLineInfoToJSON: 'vis' container not found." << eom);
        return;
    }

    int peak_chan = fParamStore->GetAs< int >("/fringe/peak_channel_idx");
    int peak_dr   = fParamStore->GetAs< int >("/fringe/max_dr_bin");
    int peak_freq = fParamStore->GetAs< int >("/fringe/peak_freq_bin");

    std::size_t n_dr   = spec_dr->GetDimension(TIME_AXIS);
    std::size_t n_freq = spec_dr->GetDimension(FREQ_AXIS);
    std::size_t n_chan  = spec_dr->GetDimension(CHANNEL_AXIS);

    // -----------------------------------------------------------------------
    // Build the DR axis (ns/s).  Use the pre-stored axis when available,
    // otherwise reconstruct it from the data dimensions.
    // -----------------------------------------------------------------------
    std::vector< double > dr_axis_ns;
    bool ok = fParamStore->Get("/fringe/sl_dr_axis_ns_per_s", dr_axis_ns);
    if(!ok || dr_axis_ns.size() != n_dr)
    {
        auto& time_ax_vis = std::get< TIME_AXIS >(*fVisibilities);
        double ap_delta = time_ax_vis(1) - time_ax_vis(0);
        double ref_freq_hz = fRefFreq * 1e6;
        dr_axis_ns.resize(n_dr);
        for(std::size_t k = 0; k < n_dr; k++)
        {
            double fr = (static_cast< double >(k) - static_cast< double >(n_dr) / 2.0) /
                        (static_cast< double >(n_dr) * ap_delta);
            dr_axis_ns[k] = fr / ref_freq_hz * 1e9;
        }
    }

    // Freq axis: copied from vis into spec_dr during search initialisation.
    auto& freq_ax = std::get< FREQ_AXIS >(*spec_dr);

    // -----------------------------------------------------------------------
    // 1-D delay-rate spectrum at (peak_chan, peak_freq_bin).
    // -----------------------------------------------------------------------
    std::size_t pc = static_cast< std::size_t >(peak_chan);
    std::size_t pf = static_cast< std::size_t >(peak_freq);
    std::size_t pd = static_cast< std::size_t >(peak_dr);

    for(std::size_t dr = 0; dr < n_dr; dr++)
    {
        double amp = std::abs((*spec_dr)(0, pc, dr, pf));
        plot_dict["DLYRATE"].push_back(amp);
        plot_dict["DLYRATE_XAXIS"].push_back(dr_axis_ns[dr]);
    }

    // -----------------------------------------------------------------------
    // 1-D frequency spectrum at (peak_chan, peak_DR_bin) - amplitude and phase.
    // -----------------------------------------------------------------------
    for(std::size_t f = 0; f < n_freq; f++)
    {
        std::complex< double > v = (*spec_dr)(0, pc, pd, f);
        double freq_val = (freq_ax.GetSize() > f) ? freq_ax(f) : static_cast< double >(f);
        plot_dict["SL_FREQ_AMP"].push_back(std::abs(v));
        plot_dict["SL_FREQ_PHS"].push_back(std::arg(v) * (180.0 / M_PI));
        plot_dict["SL_FREQ_XAXIS"].push_back(freq_val);
    }

    // -----------------------------------------------------------------------
    // 2-D amplitude surface at peak_chan: rows = DR bins, cols = freq bins.
    // -----------------------------------------------------------------------
    for(std::size_t dr = 0; dr < n_dr; dr++)
    {
        std::vector< double > row;
        row.reserve(n_freq);
        for(std::size_t f = 0; f < n_freq; f++)
        {
            row.push_back(std::abs((*spec_dr)(0, pc, dr, f)));
        }
        plot_dict["SL_2D_AMP"].push_back(row);
    }
    for(std::size_t dr = 0; dr < n_dr; dr++)
    {
        plot_dict["SL_2D_DR_AXIS"].push_back(dr_axis_ns[dr]);
    }
    for(std::size_t f = 0; f < n_freq; f++)
    {
        double freq_val = (freq_ax.GetSize() > f) ? freq_ax(f) : static_cast< double >(f);
        plot_dict["SL_2D_FREQ_AXIS"].push_back(freq_val);
    }

    // -----------------------------------------------------------------------
    // Peak-location metadata for plot overlays.
    // -----------------------------------------------------------------------
    plot_dict["extra"]["sl_peak_chan"]        = peak_chan;
    plot_dict["extra"]["sl_peak_dr_bin"]      = peak_dr;
    plot_dict["extra"]["sl_peak_freq_bin"]    = peak_freq;
    plot_dict["extra"]["sl_peak_dr_ns_per_s"] = dr_axis_ns[pd];
    double peak_freq_axis_val = (freq_ax.GetSize() > pf) ? freq_ax(pf) : static_cast< double >(pf);
    plot_dict["extra"]["sl_peak_freq_axis_val"] = peak_freq_axis_val;
    plot_dict["extra"]["sl_peak_sky_freq_mhz"]  = fParamStore->GetAs< double >("/fringe/peak_spectral_freq");
    plot_dict["extra"]["is_spectral_line"]       = true;

    // Minimal fields required by channel/segment machinery (not used in spectral-line plot, but
    // prevents crashes if generic code downstream iterates over them).
    plot_dict["NSeg"]      = 1;
    plot_dict["NPlots"]    = 1;
    plot_dict["StartPlot"] = 0;

    // Quality code and error code - same path as DumpInfoToJSON.
    std::string qc = calc_quality_code();
    fParamStore->Set("/fringe/quality_code", qc);
    plot_dict["Quality"] = qc;
    plot_dict["extra"]["nlags"]      = fParamStore->GetAs< int >("/config/nlags");
    std::string errcode = calc_error_code(plot_dict);
    plot_dict["extra"]["error_code"] = errcode;
    fParamStore->Set("/fringe/error_code", errcode);
}

} // namespace hops
