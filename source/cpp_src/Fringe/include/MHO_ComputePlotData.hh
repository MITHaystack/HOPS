#ifndef MHO_ComputePlotData_HH__
#define MHO_ComputePlotData_HH__

#include "MHO_ContainerDefinitions.hh"
#include "MHO_TableContainer.hh"

#include "MHO_BinaryOperator.hh"
#include "MHO_ComplexConjugator.hh"
#include "MHO_CyclicRotator.hh"
#include "MHO_FringeRotation.hh"
#include "MHO_FunctorBroadcaster.hh"
#include "MHO_MathUtilities.hh"
#include "MHO_NaNMasker.hh"
#include "MHO_SubSample.hh"

#include "MHO_ContainerStore.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_ParameterStore.hh"

#ifdef HOPS_USE_FFTW3
    #include "MHO_FFTWTypes.hh"
    #include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#else
    #include "MHO_FastFourierTransform.hh"
#endif

#include "MHO_MultidimensionalFastFourierTransform.hh"

#include "MHO_JSONHeaderWrapper.hh"

namespace hops
{

/*!
 *@file MHO_ComputePlotData.hh
 *@class MHO_ComputePlotData
 *@author J. Barrett - barrettj@mit.edu
 *@date Mon Apr 24 10:01:21 2023 -0400
 *@brief class for computing fringe plot information
 */

/**
 * @brief Class MHO_ComputePlotData
 */
class MHO_ComputePlotData
{
    public:
        MHO_ComputePlotData();
        virtual ~MHO_ComputePlotData(){};

        /**
         * @brief Sets optimize closure to true in underlying MHO_FringeRotation (fRot) object.
         */
        void EnableOptimizeClosure() { fRot.SetOptimizeClosureTrue(); }

        /**
         * @brief Disables optimize closure by setting underlying MHO_FringeRotation (fRot) optimizeClosure to false.
         */
        void DisableOptimizeClosure() { fRot.SetOptimizeClosureFalse(); }

        /**
         * @brief Setter for mbd anchor (either: 'sbd' or 'model')
         *
         * @param flag Flag indicating the MBD anchor type
         */
        void SetMBDAnchor(std::string flag) { fMBDAnchor = flag; }

        /**
         * @brief Setter for operator toolbox
         *
         * @param toolbox Pointer to MHO_OperatorToolbox object
         */
        void SetOperatorToolbox(MHO_OperatorToolbox* toolbox) { fToolbox = toolbox; }

        /**
         * @brief Setter for container store
         *
         * @param cStore Pointer to MHO_ContainerStore object
         */
        void SetContainerStore(MHO_ContainerStore* cStore) { fContainerStore = cStore; }

        /**
         * @brief Setter for parameter store
         *
         * @param pStore Pointer to MHO_ParameterStore object
         */
        void SetParameterStore(MHO_ParameterStore* pStore) { fParamStore = pStore; }

        /**
         * @brief Setter for vex info
         *
         * @param vex_info Const reference to mho_json object containing VEX information (from root file)
         */
        void SetVexInfo(const mho_json& vex_info) { fVexInfo = vex_info; }

        /**
         * @brief Initializes member variables by retrieving values from parameter and container stores.
         */
        void Initialize();

        /**
         * @brief Dumps fringe plot data into a JSON object for visualization/consumption by plotting routine.
         *
         * @param plot_dict JSON object to store fringe plot data.
         */
        void DumpInfoToJSON(mho_json& plot_dict);

        /**
         * @brief Dumps spectral-line fringe plot data into a JSON object.
         *        Uses the 'spec_dr' container (stored by MHO_SpectralLineFringeFitter) instead of the
         *        broadband 'sbd' container. Populates DLYRATE/DLYRATE_XAXIS (1-D DR spectrum),
         *        SL_FREQ_AMP/SL_FREQ_PHS/SL_FREQ_XAXIS (1-D freq spectrum at peak DR bin),
         *        and SL_2D_AMP/SL_2D_DR_AXIS/SL_2D_FREQ_AXIS (2-D amplitude surface at peak channel).
         *
         * @param plot_dict JSON object to store spectral-line plot data.
         */
        void DumpSpectralLineInfoToJSON(mho_json& plot_dict);

        //protected:

        TODO_FIXME_MSG("TODO FIXME, temporary kludge to pass sbd amp data for test")
        /**
         * @brief Calculates Multi-Band Delay (MBD) function (amplitude vs. mbd)
         *
         * @return xpower_amp_type representing the calculated MBD.
         */
        xpower_amp_type calc_mbd();
        /**
         * @brief Calculates Single Band Delay (SBD) function (amplitude vs. sbd)
         *
         * @return xpower_amp_type containing SBD power spectrum
         */
        xpower_amp_type calc_sbd();
        /**
         * @brief Calculates and returns time-averaged phasor segments for each channel
         *
         * @return phasor_type containing calculated phasor segments.
         */
        phasor_type calc_segs();
        /**
         * @brief Calculates and returns the cross-power spectrum for a given SBD array.
         *
         * @return xpower_type containing the calculated cross-power spectrum
         */
        xpower_type calc_xpower_spec();
        /**
         * @brief Calculates delay-rate (dr) function (amplitude vs delay rate)
         *
         * @return xpower_amp_type representing calculated delay-rate
         */
        xpower_amp_type calc_dr();

        // visibility_type* calc_corrected_vis();
        /**
         * @brief Corrects visibility data by applying fringe solution.
         */
        void correct_vis();

        /**
         * @brief Calculates fringe phase using weighted sum and fitted (delay, delay-rate) rotation.
         *
         * @return fringe phase as a double value.
         */
        double calc_phase();

        /**
         * @brief Calculates frequency root mean square (phase and amplitude) for given phasors.
         *
         * @param phasors Reference to phasor_type object containing channel data
         * @param coh_avg_phase Coherent average phase in radians
         * @param fringe_amp Fringe amplitude
         * @param total_summed_weights Total summed weights
         * @param snr Signal-to-noise ratio
         * @param freqrms_phase Output: Frequency RMS phase
         * @param freqrms_amp Output: Frequency RMS amplitude
         * @param inc_avg_amp_freq Output: incoherent average amplitude (over frequency)
         */
        void calc_freqrms(phasor_type& phasors, double coh_avg_phase, double fringe_amp, double total_summed_weights,
                          double snr, double& freqrms_phase, double& freqrms_amp, double& inc_avg_amp_freq);

        /**
         * @brief Calculates time-domain measurements (phase, amplitude, average) from phasors data.
         *
         * @param phasors Input phasor data
         * @param nseg Number of segments in phasors data
         * @param apseg Average phase segment size
         * @param coh_avg_phase Coherent average phase (degrees)
         * @param fringe_amp Fringe amplitude
         * @param total_summed_weights Total summed weights
         * @param snr Signal to Noise Ratio
         * @param timerms_phase Output: Time RMS phase
         * @param timerms_amp Output: Time RMS amplitude
         * @param inc_avg_amp Output: incoherent average amplitude (over time)
         */
        void calc_timerms(phasor_type& phasors, std::size_t nseg, std::size_t apseg, double coh_avg_phase, double fringe_amp,
                          double total_summed_weights, double snr, double& timerms_phase, double& timerms_amp,
                          double& inc_avg_amp);

        //exports the multitone pcal data (if it was applied)
        //to the plot dictionary
        void dump_multitone_pcmodel(mho_json& plot_dict,
                                    int station_flag,        //0 = reference station, 1 = remote station,
                                    std::string pol,         //single char string
                                    std::string key_suffix = "" //appended to PLOT_INFO keys, e.g. "2" for second pol
        );

        //exports the manual pcal data (pc_phases)
        //to the plot dictionary
        void dump_manual_pcmodel(mho_json& plot_dict,
                                 int station_flag,        //0 = reference station, 1 = remote station,
                                 std::string pol,         //single char string
                                 std::string key_suffix = "" //appended to PLOT_INFO keys, e.g. "2" for second pol
        );

        /**
         * @brief calcuates the fringe quality code
         */
        std::string calc_quality_code(); //quality only, not error

        /**
         * @brief Pre-computes per-channel metadata (freq, sideband sign, bandwidth) into flat arrays,
         *        eliminating repeated string-keyed label lookups inside the calc_* functions.
         */
        void precompute_chan_metadata();

        /**
         * @brief Pre-computes vrot lookup tables indexed as [ch * fNAP + ap].
         *        Three tables are built to match the fRot state used by each calc_* function:
         *          fVRTable      - default SBD params + fMBDelay  (calc_sbd, calc_dr)
         *          fVRMBD0Table  - default SBD params + MBD=0     (calc_mbd)
         *          fVRPhaseTable - proper SBD params  + fMBDelay  (calc_phase, calc_xpower_spec,
         *                                                           calc_segs, correct_vis)
         */
        void precompute_vr_tables();

        /**
         * @brief Merged computation of SBD amplitude spectrum and cross-power spectrum.
         *        Single ch->ap->bin pass replaces two separate bin->ch->ap passes.
         *        calc_sbd uses fVRTable; calc_xpower_spec uses fVRPhaseTable.
         *
         * @param sbd_amp     Output: SBD amplitude vs delay (was calc_sbd return value)
         * @param cp_spectrum Output: cross-power spectrum (was calc_xpower_spec return value)
         */
        void calc_sbd_and_xpower_spec(xpower_amp_type& sbd_amp, xpower_type& cp_spectrum);

        /**
         * @brief Merged computation of delay-rate spectrum, phasor segments, and fringe phase.
         *        Single ch->ap pass over fSBDMaxBin replaces three separate passes.
         *        DR uses fVRTable; segs and phase use fVRPhaseTable.
         *
         * @param coh_avg_phase Output: coherent average phase (was calc_phase return value)
         * @param phasor_segs   Output: phasor segment data (was calc_segs return value)
         * @return xpower_amp_type delay-rate amplitude spectrum (was calc_dr return value)
         */
        xpower_amp_type calc_dr_segs_phase(double& coh_avg_phase, phasor_type& phasor_segs);

        /**
         * @brief calcuates the fringe error code
         */
        std::string calc_error_code(const mho_json& plot_dict);

        double fRefFreq;
        double fTotalSummedWeights;
        double fMBDelay;
        double fDelayRate;
        double fFringeRate;
        double fSBDelay;
        double fAmp;

        std::size_t fSBDMaxBin;

        bool fValid;
        visibility_type* fVisibilities;
        visibility_type* fSBDArray;
        weight_type* fWeights;

        xpower_type fDRWorkspace;
        xpower_amp_type fDRAmpWorkspace;

        xpower_type fMBDWorkspace;
        xpower_amp_type fMBDAmpWorkspace;

        xpower_type fFringe;

        MHO_ContainerStore* fContainerStore;
        MHO_ParameterStore* fParamStore;
        MHO_OperatorToolbox* fToolbox;
        mho_json fVexInfo;

        MHO_MultidimensionalFastFourierTransform< xpower_type > fFFTEngine;
        MHO_CyclicRotator< xpower_type > fCyclicRotator;

        //class which implements vrot
        MHO_FringeRotation fRot;

        //flag for mbd anchor
        std::string fMBDAnchor;

        //space for xpower spectrum and sbdbox
        std::vector< double > fSBDBox;
        std::vector< int > fNUSBAP;
        std::vector< int > fNLSBAP;

        //constants
        std::complex< double > fImagUnit;

        // Pre-computed per-channel metadata (populated by precompute_chan_metadata)
        std::size_t fNChan;
        std::size_t fNAP;
        std::vector< double > fChanFreq;      // sky frequency per channel
        std::vector< int > fChanSideband;     // sideband sign per channel: +1=USB, 0=DSB, -1=LSB
        std::vector< double > fChanBandwidth; // bandwidth per channel

        // Pre-computed vrot tables indexed as [ch * fNAP + ap] (populated by precompute_vr_tables)
        std::vector< std::complex< double > > fVRTable;      // default SBD params + fMBDelay
        std::vector< std::complex< double > > fVRMBD0Table;  // default SBD params + MBD=0
        std::vector< std::complex< double > > fVRPhaseTable; // proper SBD params + fMBDelay

        //lsb/usb validity segments
        std::vector< std::vector< double > > seg_frac_usb;
        std::vector< std::vector< double > > seg_frac_lsb;

};

} // namespace hops

#endif /*! end of include guard: MHO_ComputePlotData_HH__ */
