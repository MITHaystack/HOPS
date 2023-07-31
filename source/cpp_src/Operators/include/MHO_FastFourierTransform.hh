#ifndef MHO_FastFourierTransform_HH__
#define MHO_FastFourierTransform_HH__

#include <complex>

#include "MHO_NDArrayWrapper.hh"
#include "MHO_UnaryOperator.hh"
#include "MHO_BitReversalPermutation.hh"
#include "MHO_FastFourierTransformUtilities.hh"
#include "MHO_FastFourierTransformWorkspace.hh"

namespace hops
{

template< typename XFloatType >
class MHO_FastFourierTransform:
    public MHO_UnaryOperator< MHO_NDArrayWrapper< std::complex<XFloatType>, 1> >
{
    public:

        using XArrayType = MHO_NDArrayWrapper< std::complex< XFloatType>, 1 >;

        MHO_FastFourierTransform();
        virtual ~MHO_FastFourierTransform();

        virtual void SetForward();
        virtual void SetBackward();

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
MHO_FastFourierTransform<XFloatType>::MHO_FastFourierTransform()
{
    fForward = true;
    fInitialized = false;
}

template< typename XFloatType >
MHO_FastFourierTransform<XFloatType>::~MHO_FastFourierTransform(){}

template< typename XFloatType >
void
MHO_FastFourierTransform<XFloatType>::SetForward(){fForward = true;}

template< typename XFloatType >
void
MHO_FastFourierTransform<XFloatType>::SetBackward(){fForward = false;}

template< typename XFloatType >
bool
MHO_FastFourierTransform<XFloatType>::InitializeInPlace(XArrayType* in)
{
    if( in != nullptr )
    {
        if( fW.fN != in->GetSize() )
        {
            fW.Resize( in->GetSize() );
            fInitialized = true;
        }
    }
    else
    {
        fInitialized = false;
    }
    return fInitialized;
}


template< typename XFloatType >
bool
MHO_FastFourierTransform<XFloatType>::InitializeOutOfPlace(const XArrayType* in, XArrayType* out)
{
    if( (in != nullptr && out != nullptr) && (in->GetSize() == out->GetSize()) )
    {
        if( fW.fN != in->GetSize() )
        {
            fW.Resize( in->GetSize() );
            fInitialized = true;
        }
    }
    else
    {
        fInitialized = false;
    }
    return fInitialized;
}


///Make a call to execute the FFT plan and perform the transformation
template< typename XFloatType >
bool
MHO_FastFourierTransform<XFloatType>::ExecuteInPlace(XArrayType* in)
{
    if(fInitialized)
    {
        //for DFT we conjugate first (NOTE: this matches FFTW3 convention)
        if(fForward)
        {
            std::complex<XFloatType>* data = in->GetData();
            for(unsigned int i=0; i<fW.fN; i++)
            {
                data[i] = std::conj(data[i]);
            }
        }

        if(fW.fM == 0) //size is a power of two
        {
            //use radix-2
            MHO_BitReversalPermutation::PermuteArray< std::complex<XFloatType> >(fW.fN, fW.fPermutation, in->GetData());
            MHO_FastFourierTransformUtilities<XFloatType>::FFTRadixTwo_DIT(fW.fN, in->GetData(), fW.fTwiddle);
        }
        else
        {
            //use bluestein algorithm for arbitrary N
            MHO_FastFourierTransformUtilities<XFloatType>::FFTBluestein(fW.fN, fW.fM, in->GetData(), fW.fTwiddle, fW.fConjugateTwiddle, fW.fScale, fW.fCirculant, fW.fWorkspace);
        }

        //for DFT we conjugate again (NOTE: this matches FFTW3 convention)
        if(fForward)
        {
            std::complex<XFloatType>* data = in->GetData();
            for(unsigned int i=0; i<fW.fN; i++)
            {
                data[i] = std::conj(data[i]);
            }
        }

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


#endif /* MHO_FastFourierTransform_H__ */
