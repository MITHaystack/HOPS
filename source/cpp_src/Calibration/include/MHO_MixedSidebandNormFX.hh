#ifndef MHO_MixedSidebandNormFX_HH__
#define MHO_MixedSidebandNormFX_HH__

#include <cmath>
#include <complex>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_TableContainer.hh"

#include "MHO_UnaryOperator.hh"
#include "MHO_ComplexConjugator.hh"
#include "MHO_CyclicRotator.hh"
#include "MHO_EndZeroPadder.hh"
#include "MHO_FunctorBroadcaster.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"
#include "MHO_NaNMasker.hh"
#include "MHO_SubSample.hh"
#include "MHO_NormFX.hh"

#ifdef HOPS_USE_FFTW3
    #include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#endif

namespace hops
{

/*!
 *@file MHO_MixedSidebandNormFX.hh
 *@class MHO_MixedSidebandNormFX
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Jul 9 11:47:00 2021 -0400
 *@brief implements a subset of the functionality found in norm_fx.c,
 *mainly the transform from frequency to delay space -- this implementation is
 *closer to the original since it preserves the extra padding factor (8x), which
 *is later followed by a factor of 2 sub-sampling. The original motivation for
 *this extra computation appears to be lost.
 */

class MHO_MixedSidebandNormFX: public MHO_NormFX //public MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_MixedSidebandNormFX();
        virtual ~MHO_MixedSidebandNormFX();

    protected:
        using XArgType = visibility_type;

        virtual bool InitializeInPlace(XArgType* in) override;
        virtual bool InitializeOutOfPlace(const XArgType* in, XArgType* out) override;

        virtual bool ExecuteInPlace(XArgType* in) override;
        virtual bool ExecuteOutOfPlace(const XArgType* in, XArgType* out) override;

        // virtual bool InitializeInPlace(const XArgType* in, XArgType* out) override;
        // virtual bool Execute(const XArgType* in, XArgType* out) override;

    private:
        std::size_t fInDims[VIS_NDIM];
        std::size_t fWorkDims[VIS_NDIM];
        std::size_t fOutDims[VIS_NDIM];

        typedef MHO_NaNMasker< visibility_type > nanMaskerType;
        typedef MHO_ComplexConjugator< visibility_type > conjType;

        MHO_FunctorBroadcaster< visibility_type, nanMaskerType > fNaNBroadcaster;

#ifdef HOPS_USE_FFTW3
        using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransformFFTW< visibility_type >;
#else
        using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransform< visibility_type >;
#endif

        FFT_ENGINE_TYPE fFFTEngine;
        MHO_EndZeroPadder< visibility_type > fZeroPadder;
        MHO_SubSample< visibility_type > fSubSampler;
        MHO_CyclicRotator< visibility_type > fCyclicRotator;

        visibility_type fWorkspace;
        bool fInitialized;

        void FillWorkspace(const visibility_type* in, visibility_type* workspace);

        //bool ApplyWeights(visibility_type* out, weight_type* w, bool invert);
};

} // namespace hops

#endif /*! end of include guard: MHO_MixedSidebandNormFX */
