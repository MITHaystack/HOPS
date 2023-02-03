#ifndef MHO_DelayRate_HH__
#define MHO_DelayRate_HH__

/*
*File: MHO_DelayRate.hh
*Class: MHO_DelayRate
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <cmath>
#include <complex>

#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"

#include "MHO_UnaryOperator.hh"
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


class MHO_DelayRate: public MHO_UnaryOperator< ch_sbd_type >
{
    public:
        MHO_DelayRate();
        virtual ~MHO_DelayRate();

    protected:

        using XArgType = ch_sbd_type;


        virtual bool InitializeInPlace(XArgType* in) override {return false;};
        virtual bool ExecuteInPlace(XArgType* in) override {return false;};

        virtual bool InitializeOutOfPlace(const XArgType* in, XArgType* out) override;
        virtual bool ExecuteOutOfPlace(const XArgType* in1, XArgType* out) override;

    private:

        std::size_t fInDims[CH_VIS_NDIM];
        std::size_t fOutDims[CH_VIS_NDIM];

        void ConditionallyResizeOutput(const std::size_t* dims, std::size_t size, XArgType* out);

        MHO_MultidimensionalPaddedFastFourierTransform< ch_visibility_type > fPaddedFFTEngine;

        MHO_SubSample<ch_sbd_type> fSubSampler;
        MHO_CyclicRotator<ch_sbd_type> fCyclicRotator;

        bool fInitialized;

};


}


#endif /* end of include guard: MHO_DelayRate */
