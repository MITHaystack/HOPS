#ifndef MHO_NormFX_HH__
#define MHO_NormFX_HH__

/*
*File: MHO_NormFX.hh
*Class: MHO_NormFX
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <cmath>
#include <complex>

#include "MHO_FFTWTypes.hh"
#include "MHO_TableContainer.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_BinaryOperator.hh"

#include "MHO_NaNMasker.hh"
#include "MHO_FunctorBroadcaster.hh"
#include "MHO_ComplexConjugator.hh"
#include "MHO_CyclicRotator.hh"
#include "MHO_SubSample.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"
#include "MHO_MultidimensionalPaddedFastFourierTransform.hh"

#ifdef HOPS_USE_FFTW3
    #include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#else
    #include "MHO_FastFourierTransform.hh"
#endif





namespace hops
{


class MHO_NormFX: public MHO_BinaryOperator<
    ch_baseline_data_type,
    ch_baseline_weight_type,
    ch_baseline_sbd_type >
{
    public:
        MHO_NormFX();
        virtual ~MHO_NormFX();

    protected:

        using XArgType1 = ch_baseline_data_type;
        using XArgType2 = ch_baseline_weight_type;
        using XArgType3 = ch_baseline_sbd_type;

        virtual bool InitializeImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out) override;
        virtual bool ExecuteImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out) override;

    private:

        void run_old_normfx_core(const XArgType1* in1, const XArgType2* in2, XArgType3* out);

        std::size_t fInDims[CH_VIS_NDIM];
        std::size_t fWorkDims[CH_VIS_NDIM];
        std::size_t fOutDims[CH_VIS_NDIM];

        //only needed for the old routine
        #ifdef HOPS_USE_FFTW3
            MHO_MultidimensionalFastFourierTransformFFTW<double,1> fFFTEngine;
        #else
            MHO_MultidimensionalFastFourierTransform<double,1> fFFTEngine;
        #endif
        MHO_NDArrayWrapper< std::complex<double>, 1 > xp_spec;
        MHO_NDArrayWrapper< std::complex<double>, 1 > S;
        MHO_NDArrayWrapper< std::complex<double>, 1 > xlag;

        typedef MHO_NaNMasker<ch_baseline_data_type> nanMaskerType;
        typedef MHO_ComplexConjugator<ch_baseline_data_type> conjType;

        MHO_FunctorBroadcaster<ch_baseline_data_type, nanMaskerType> fNaNBroadcaster;
        MHO_FunctorBroadcaster<ch_baseline_data_type, conjType> fConjBroadcaster;

        MHO_MultidimensionalPaddedFastFourierTransform<VFP_TYPE, CH_VIS_NDIM> fPaddedFFTEngine;

        MHO_SubSample<ch_baseline_sbd_type> fSubSampler;
        MHO_CyclicRotator<ch_baseline_sbd_type> fCyclicRotator;

        ch_baseline_sbd_type fWorkspace;
        bool fInitialized;
        bool fIsUSB;

};


}


#endif /* end of include guard: MHO_NormFX */
