#ifndef MHO_FastFourierTransform_HH__
#define MHO_FastFourierTransform_HH__


#include <complex>

#include "MHO_NDArrayWrapper.hh"
#include "MHO_UnaryOperator.hh"
#include "MHO_BitReversalPermutation.hh"
#include "MHO_FastFourierTransformUtilities.hh"
#include "MHO_FastFourierTransformWorkspace.hh"
#include "MHO_FastFourierTransformCalls.hh"

namespace hops
{

/*!
*@file MHO_FastFourierTransform
*@class MHO_FastFourierTransform.hh
*@author J. Barrett - barrettj@mit.edu
*@date Fri Oct 23 12:02:01 2020 -0400
*@brief
*/


template< typename XFloatType >
class MHO_FastFourierTransform:
    public MHO_UnaryOperator< MHO_NDArrayWrapper< std::complex<XFloatType>, 1> >
{
    public:

        using XArrayType = MHO_NDArrayWrapper< std::complex< XFloatType>, 1 >;

        MHO_FastFourierTransform()
        {
            fForward = true;
            fInitialized = false;
        }
        virtual ~MHO_FastFourierTransform(){};

        virtual void SetForward(){fForward = true;};
        virtual void SetBackward(){fForward = false;};

    protected:

        virtual bool InitializeInPlace(XArrayType* in) override;
        virtual bool ExecuteInPlace(XArrayType* in) override;
        virtual bool InitializeOutOfPlace(const XArrayType* in, XArrayType* out) override;
        virtual bool ExecuteOutOfPlace(const XArrayType* in, XArrayType* out) override;

    private:

        bool fForward;
        bool fInitialized;
        MHO_FastFourierTransformWorkspace<XFloatType> fW;

};

template< typename XFloatType >
bool
MHO_FastFourierTransform<XFloatType>::InitializeInPlace(XArrayType* in)
{
    if( in != nullptr )
    {
        if( fW.fN != in->GetSize() ){ fW.Resize( in->GetSize() ); }
        fInitialized = true;
    }
    else{ fInitialized = false; }
    return fInitialized;
}


template< typename XFloatType >
bool
MHO_FastFourierTransform<XFloatType>::InitializeOutOfPlace(const XArrayType* in, XArrayType* out)
{
    if( (in != nullptr && out != nullptr) && (in->GetSize() == out->GetSize()) )
    {
        if( fW.fN != in->GetSize() ){ fW.Resize( in->GetSize() ); }
        fInitialized = true;
    }
    else{ fInitialized = false; }
    return fInitialized;
}


///Make a call to execute the FFT plan and perform the transformation
template< typename XFloatType >
bool
MHO_FastFourierTransform<XFloatType>::ExecuteInPlace(XArrayType* in)
{
    if(fInitialized)
    {
        if( fW.IsRadix2() ){ FFTRadix2(in->GetData(), fW, fForward); }
        else{ FFTBluestein(in->GetData(), fW, fForward); }
        return true;
    }
    else
    {
        //error
        msg_error("math", "FFT input/output array dimensions are not valid or intialization failed. Aborting transform." << eom);
        return false;
    }
}

///Make a call to execute the FFT plan and perform the transformation
template< typename XFloatType >
bool
MHO_FastFourierTransform<XFloatType>::ExecuteOutOfPlace(const XArrayType* in, XArrayType* out)
{
    //the arrays are not identical so copy the input over to the output
    std::memcpy( (void*) out->GetData(), (void*) in->GetData(), fW.fN*sizeof(std::complex<XFloatType>) );
    return ExecuteInPlace(out);
}


}


#endif /*! MHO_FastFourierTransform_H__ */
