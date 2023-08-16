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
using xpower_amp_type = MHO_TableContainer< double, MHO_AxisPack< MHO_Axis<double> > >;


class MHO_ComputePlotData
{
    public:
        MHO_ComputePlotData(){};
        virtual ~MHO_ComputePlotData(){};

        void SetSummedWeights(double total_ap_frac){fTotalSummedWeights = total_ap_frac;}
        void SetReferenceFrequency(double ref_freq){fRefFreq = ref_freq;}
        void SetMBDelay(double mbdelay){fMBDelay = mbdelay;}
        void SetDelayRate(double dr){fDelayRate = dr;}
        void SetFringeRate(double fr){fFringeRate = fr;}
        void SetSBDelay(double sbdelay){fSBDelay = sbdelay;}
        void SetSBDelayBin(std::size_t max_sbd_bin){fSBDMaxBin = max_sbd_bin;};
        void SetAmplitude(double amp){fAmp = amp;}

        void SetSBDArray(visibility_type* sbd_arr){fSBDArray = sbd_arr;}
        void SetWeights(weight_type* weights){fWeights = weights;}
        void SetVisibilities(visibility_type* vis_data){fVisibilities = vis_data;}
        void SetVexInfo(const mho_json& vex_info){fVexInfo = vex_info;}

        mho_json DumpInfoToJSON();

    //protected:

        #pragma message("TODO FIXME, temporary kludge to pass sbd amp data for test")
        xpower_amp_type calc_mbd();
        xpower_amp_type calc_sbd();
        xpower_type calc_xpower();
        xpower_type calc_xpower_KLUDGE();
        xpower_type calc_xpower_KLUDGE2();
        xpower_type calc_xpower_KLUDGE3();
        xpower_amp_type calc_dr();

        double calc_phase();

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

        mho_json fVexInfo;

        MHO_MultidimensionalPaddedFastFourierTransform< xpower_type > fPaddedFFTEngine;
        MHO_MultidimensionalFastFourierTransform< xpower_type > fFFTEngine;
        MHO_CyclicRotator< xpower_type > fCyclicRotator;

};

}

#endif /* end of include guard: MHO_ComputePlotData_HH__ */
