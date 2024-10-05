#ifndef MHO_SingleSidebandNormFX_HH__
#define MHO_SingleSidebandNormFX_HH__

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
#include "MHO_SBDTableGenerator.hh"
#include "MHO_NormFX.hh"

#ifdef HOPS_USE_FFTW3
    #include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#endif

namespace hops
{

/*!
 *@file MHO_SingleSidebandNormFX.hh
 *@class MHO_SingleSidebandNormFX
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Jul 9 11:47:00 2021 -0400
 *@brief implements a subset of the functionality found in norm_fx.c,
 *mainly the transform from frequency to delay space with a reduced
 *zero padding factor (2x smaller than original implementation) 
 since we only have a single sideband to worry about
 */

class MHO_SingleSidebandNormFX: public MHO_NormFX //MHO_UnaryOperator< visibility_type >
{
    public:
        MHO_SingleSidebandNormFX();
        virtual ~MHO_SingleSidebandNormFX();

    protected:
        using XArgType = visibility_type;

        virtual bool InitializeInPlace(XArgType* in) override;
        virtual bool InitializeOutOfPlace(const XArgType* in, XArgType* out) override;

        virtual bool ExecuteInPlace(XArgType* in) override;
        virtual bool ExecuteOutOfPlace(const XArgType* in, XArgType* out) override;

    private:
        std::size_t fInDims[VIS_NDIM];
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
        MHO_CyclicRotator< visibility_type > fCyclicRotator;
        MHO_SBDTableGenerator fSBDGen;
        bool fInitialized;
};

} // namespace hops

#endif /*! end of include guard: MHO_SingleSidebandNormFX */
