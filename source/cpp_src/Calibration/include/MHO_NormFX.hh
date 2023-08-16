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

#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"

#include "MHO_BinaryOperator.hh"
#include "MHO_NaNMasker.hh"
#include "MHO_FunctorBroadcaster.hh"
#include "MHO_ComplexConjugator.hh"
#include "MHO_CyclicRotator.hh"
#include "MHO_SubSample.hh"
#include "MHO_EndZeroPadder.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"

#ifdef HOPS_USE_FFTW3
#include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#endif

namespace hops
{


class MHO_NormFX: public MHO_BinaryOperator<
    visibility_type,
    weight_type,
    sbd_type >
{
    public:
        MHO_NormFX();
        virtual ~MHO_NormFX();

    protected:

        using XArgType1 = visibility_type;
        using XArgType2 = weight_type;
        using XArgType3 = sbd_type;

        virtual bool InitializeImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out) override;
        virtual bool ExecuteImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out) override;

    private:

        std::size_t fInDims[VIS_NDIM];
        std::size_t fWorkDims[VIS_NDIM];
        std::size_t fOutDims[VIS_NDIM];

        typedef MHO_NaNMasker<visibility_type> nanMaskerType;
        typedef MHO_ComplexConjugator<sbd_type> conjType;

        MHO_FunctorBroadcaster<visibility_type, nanMaskerType> fNaNBroadcaster;
        MHO_FunctorBroadcaster<visibility_type, conjType> fConjBroadcaster;
        
        #ifdef HOPS_USE_FFTW3
        using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransformFFTW< visibility_type >;
        #else
        using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransform< visibility_type >;
        #endif

        FFT_ENGINE_TYPE fFFTEngine;
        MHO_EndZeroPadder< visibility_type > fZeroPadder;
        MHO_SubSample<sbd_type> fSubSampler;
        MHO_CyclicRotator<sbd_type> fCyclicRotator;

        sbd_type fWorkspace;
        bool fInitialized;
        bool fIsUSB;

};


}


#endif /* end of include guard: MHO_NormFX */
