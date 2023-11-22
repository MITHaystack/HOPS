#ifndef MHO_ComputePlotData_HH__
#define MHO_ComputePlotData_HH__

#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"

#include "MHO_BinaryOperator.hh"
#include "MHO_NaNMasker.hh"
#include "MHO_FunctorBroadcaster.hh"
#include "MHO_ComplexConjugator.hh"
#include "MHO_CyclicRotator.hh"
#include "MHO_SubSample.hh"
#include "MHO_FringeRotation.hh"

#include "MHO_ParameterStore.hh"
#include "MHO_ContainerStore.hh"

#ifdef HOPS_USE_FFTW3
    #include "MHO_FFTWTypes.hh"
    #include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#else
    #include "MHO_FastFourierTransform.hh"
#endif


#include "MHO_MultidimensionalFastFourierTransform.hh"
#include "MHO_MultidimensionalPaddedFastFourierTransform.hh"

#include "MHO_JSONHeaderWrapper.hh"

namespace hops
{

using xpower_type = MHO_TableContainer< std::complex<double>, MHO_AxisPack< MHO_Axis<double> > >;
using phasor_type = MHO_TableContainer< std::complex<double>, MHO_AxisPack< MHO_Axis<double>, MHO_Axis<double> > >;
using xpower_amp_type = MHO_TableContainer< double, MHO_AxisPack< MHO_Axis<double> > >;


class MHO_ComputePlotData
{
    public:
        MHO_ComputePlotData();
        virtual ~MHO_ComputePlotData(){};

        void EnableOptimizeClosure(){fRot.SetOptimizeClosureTrue();}
        void DisableOptimizeClosure(){fRot.SetOptimizeClosureFalse();}
        void SetMBDAnchor(std::string flag){fMBDAnchor = flag;}

        void SetContainerStore(MHO_ContainerStore* cStore){fContainerStore = cStore;}
        void SetParameterStore(MHO_ParameterStore* pStore){fParamStore = pStore;}
        void SetVexInfo(const mho_json& vex_info){fVexInfo = vex_info;}

        void Initialize();

        void DumpInfoToJSON(mho_json& plot_dict);

    //protected:

        #pragma message("TODO FIXME, temporary kludge to pass sbd amp data for test")
        xpower_amp_type calc_mbd();
        xpower_amp_type calc_sbd();
        phasor_type calc_segs();
        xpower_type calc_xpower_spec();
        xpower_amp_type calc_dr();

        double calc_phase();

        void calc_freqrms(phasor_type& phasors, double coh_avg_phase, double fringe_amp, double total_summed_weights, double& freqrms_phase, double& freqrms_amp);

        //these functions copied from ffmath and minmax.c -- TODO move to MHO_Math library
        int parabola(double y[3], double lower, double upper, double* x_max, double* amp_max, double q[3]);
        double dwin(double value, double lower, double upper)
        {
            if (value < lower) return (lower);
            else if (value > upper) return (upper);
            else return (value);
        }

        double fRefFreq;
        double fTotalSummedWeights;
        double fMBDelay;
        double fDelayRate;
        double fFringeRate;
        double fSBDelay;
        double fAmp;

        std::size_t fSBDMaxBin;

        visibility_type* fVisibilities;
        visibility_type* fSBDArray;
        weight_type* fWeights;

        xpower_type fDRWorkspace;
        xpower_amp_type fDRAmpWorkspace;

        xpower_type fMBDWorkspace;
        xpower_amp_type fMBDAmpWorkspace;

        MHO_ContainerStore* fContainerStore;
        MHO_ParameterStore* fParamStore;
        mho_json fVexInfo;

        MHO_MultidimensionalPaddedFastFourierTransform< xpower_type > fPaddedFFTEngine;
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

};

}

#endif /* end of include guard: MHO_ComputePlotData_HH__ */
