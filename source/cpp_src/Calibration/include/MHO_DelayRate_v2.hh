#ifndef MHO_DelayRate_v2_HH__
#define MHO_DelayRate_v2_HH__

/*
*File: MHO_DelayRate_v2.hh
*Class: MHO_DelayRate_v2
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
#include "MHO_CyclicRotator.hh"
#include "MHO_SubSample.hh"

#ifdef HOPS_USE_FFTW3
    #include "MHO_FFTWTypes.hh"
    #include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#else
    #include "MHO_FastFourierTransform.hh"
#endif


#include "MHO_MultidimensionalFastFourierTransform.hh"
#include "MHO_MultidimensionalPaddedFastFourierTransform.hh"





namespace hops
{


class MHO_DelayRate_v2: public MHO_BinaryOperator<
    visibility_type,
    weight_type,
    sbd_type >
{
    public:
        MHO_DelayRate_v2();
        virtual ~MHO_DelayRate_v2();
        
    protected:

        using XArgType1 = visibility_type;
        using XArgType2 = weight_type;
        using XArgType3 = sbd_type;

        virtual bool InitializeImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out) override;
        virtual bool ExecuteImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out) override;

    private:

        std::size_t fInDims[VIS_NDIM];
        std::size_t fOutDims[VIS_NDIM];

        void ConditionallyResizeOutput(const std::size_t* dims, std::size_t size, XArgType3* out);

        MHO_MultidimensionalPaddedFastFourierTransform< visibility_type > fPaddedFFTEngine;

        MHO_SubSample<sbd_type> fSubSampler;
        MHO_CyclicRotator<sbd_type> fCyclicRotator;
        
        sbd_type fWorkspace;

        bool fInitialized;

};


}


#endif /* end of include guard: MHO_DelayRate_v2 */
