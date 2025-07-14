#ifndef MHO_FastFourierTransform_HH__
#define MHO_FastFourierTransform_HH__

#include <complex>

#include "MHO_BitReversalPermutation.hh"
#include "MHO_FastFourierTransformCalls.hh"
#include "MHO_FastFourierTransformUtilities.hh"
#include "MHO_FastFourierTransformWorkspace.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_UnaryOperator.hh"

namespace hops
{

/*!
 *@file MHO_FastFourierTransform
 *@class MHO_FastFourierTransform.hh
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Oct 23 12:02:01 2020 -0400
 *@brief Wrapper class for the native FFT implementation
 */

/**
 * @brief Class MHO_FastFourierTransform
 */
template< typename XFloatType >
class MHO_FastFourierTransform: public MHO_UnaryOperator< MHO_NDArrayWrapper< std::complex< XFloatType >, 1 > >
{
    public:
        using XArrayType = MHO_NDArrayWrapper< std::complex< XFloatType >, 1 >;

        MHO_FastFourierTransform()
        {
            fForward = true;
            fInitialized = false;
        }

        virtual ~MHO_FastFourierTransform(){};

        /**
         * @brief Setter for forward flag (FFT direction)
         * @note This is a virtual function.
         */
        virtual void SetForward() { fForward = true; };

        /**
         * @brief Setter for backward flag (FFT direction)
         * @note This is a virtual function.
         */
        virtual void SetBackward() { fForward = false; };

    protected:
        /**
         * @brief Initializes in-place transformation and checks input array validity.
         * 
         * @param in Input array pointer for initialization.
         * @return Boolean indicating successful initialization.
         * @note This is a virtual function.
         */
        virtual bool InitializeInPlace(XArrayType* in) override;
        /**
         * @brief Function ExecuteInPlace, does the FFT on the array in-place
         * 
         * @param in (XArrayType*)
         * @return Return value (bool)
         * @note This is a virtual function.
         */
        virtual bool ExecuteInPlace(XArrayType* in) override;
        /**
         * @brief Initializes out-of-place FFT by checking input/output array sizes and resizing if necessary.
         * 
         * @param in Const reference to input XArrayType
         * @param out Reference to output XArrayType
         * @return Boolean indicating successful initialization
         * @note This is a virtual function.
         */
        virtual bool InitializeOutOfPlace(const XArrayType* in, XArrayType* out) override;
        /**
         * @brief Copies input array to output and executes in-place FFT in the output array
         * 
         * @param in Const reference to input array.
         * @param out (XArrayType*)
         * @return Boolean indicating success of ExecuteInPlace operation.
         * @note This is a virtual function.
         */
        virtual bool ExecuteOutOfPlace(const XArrayType* in, XArrayType* out) override;

    private:
        bool fForward;
        bool fInitialized;
        MHO_FastFourierTransformWorkspace< XFloatType > fW;
};

/**
 * @brief Function MHO_FastFourierTransform<XFloatType>::InitializeInPlace
 * 
 * @param in (XArrayType*)
 * @return Return value (template< typename XFloatType > bool MHO_FastFourierTransform< XFloatType)
 */
template< typename XFloatType > bool MHO_FastFourierTransform< XFloatType >::InitializeInPlace(XArrayType* in)
{
    if(in != nullptr)
    {
        if(fW.fN != in->GetSize())
        {
            fW.Resize(in->GetSize());
        }
        fInitialized = true;
    }
    else
    {
        fInitialized = false;
    }
    return fInitialized;
}

/**
 * @brief Function MHO_FastFourierTransform<XFloatType>::InitializeOutOfPlace
 * 
 * @param in (const XArrayType*)
 * @param out (XArrayType*)
 * @return Return value (bool MHO_FastFourierTransform< XFloatType)
 */
template< typename XFloatType >
bool MHO_FastFourierTransform< XFloatType >::InitializeOutOfPlace(const XArrayType* in, XArrayType* out)
{
    if((in != nullptr && out != nullptr) && (in->GetSize() == out->GetSize()))
    {
        if(fW.fN != in->GetSize())
        {
            fW.Resize(in->GetSize());
        }
        fInitialized = true;
    }
    else
    {
        fInitialized = false;
    }
    return fInitialized;
}

///Make a call to execute the FFT plan and perform the transformation
template< typename XFloatType > bool MHO_FastFourierTransform< XFloatType >::ExecuteInPlace(XArrayType* in)
{
    if(fInitialized)
    {
        if(fW.IsRadix2())
        {
            FFTRadix2(in->GetData(), fW, fForward);
        }
        else
        {
            FFTBluestein(in->GetData(), fW, fForward);
        }
        return true;
    }
    else
    {
        //error
        msg_error("math",
                  "FFT input/output array dimensions are not valid or intialization failed. Aborting transform." << eom);
        return false;
    }
}

///Make a call to execute the FFT plan and perform the transformation
template< typename XFloatType >
bool MHO_FastFourierTransform< XFloatType >::ExecuteOutOfPlace(const XArrayType* in, XArrayType* out)
{
    //the arrays are not identical so copy the input over to the output
    std::memcpy((void*)out->GetData(), (void*)in->GetData(), fW.fN * sizeof(std::complex< XFloatType >));
    return ExecuteInPlace(out);
}

} // namespace hops

#endif /*! MHO_FastFourierTransform_H__ */
