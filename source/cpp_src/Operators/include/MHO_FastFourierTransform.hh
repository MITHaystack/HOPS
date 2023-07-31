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

        // virtual void AllocateWorkspace();
        // virtual void DealocateWorkspace();

        bool fForward;
        //bool fSizeIsPowerOfTwo;
        bool fInitialized;

        MHO_FastFourierTransformWorkspace<XFloatType> fW;

        // //auxilliary workspace needed for basic 1D transform
        // unsigned int fN;
        // unsigned int fM;
        // unsigned int* fPermutation;
        // std::complex<XFloatType>* fTwiddle;
        // std::complex<XFloatType>* fConjugateTwiddle;
        // std::complex<XFloatType>* fScale;
        // std::complex<XFloatType>* fCirculant;
        // std::complex<XFloatType>* fWorkspace;

};


template< typename XFloatType >
MHO_FastFourierTransform<XFloatType>::MHO_FastFourierTransform()
{
    fForward = true;
    //fSizeIsPowerOfTwo = false;
    fInitialized = false;

    // fN = 0;
    // fM = 0;
    // fPermutation = NULL;
    // fTwiddle = NULL;
    // fConjugateTwiddle = NULL;
    // fScale = NULL;
    // fCirculant = NULL;
    // fWorkspace = NULL;
}

template< typename XFloatType >
MHO_FastFourierTransform<XFloatType>::~MHO_FastFourierTransform()
{
    // DealocateWorkspace();
}

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
            // fN = in->GetSize();
            // fSizeIsPowerOfTwo = MHO_BitReversalPermutation::IsPowerOfTwo(fN);
            // fM = MHO_FastFourierTransformUtilities<XFloatType>::ComputeBluesteinArraySize(fN);
            // //initialize the workspace
            // DealocateWorkspace();
            // AllocateWorkspace();
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
            // fN = in->GetSize();
            // fSizeIsPowerOfTwo = MHO_BitReversalPermutation::IsPowerOfTwo(fN);
            // fM = MHO_FastFourierTransformUtilities<XFloatType>::ComputeBluesteinArraySize(fN);
            // //initialize the workspace
            // DealocateWorkspace();
            // AllocateWorkspace();
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

        //if(fSizeIsPowerOfTwo)
        if(fW.fM == 0)
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


// template< typename XFloatType >
// void
// MHO_FastFourierTransform<XFloatType>::AllocateWorkspace()
// {
//     if(!fSizeIsPowerOfTwo)
//     {
//         //can't perform an in-place transform, need workspace
//         fPermutation = new unsigned int[fM];
//         fTwiddle = new std::complex<XFloatType>[fM];
//         fConjugateTwiddle = new std::complex<XFloatType>[fM];
//         fScale = new std::complex<XFloatType>[fN];
//         fCirculant = new std::complex<XFloatType>[fM];
//         fWorkspace = new std::complex<XFloatType>[fM];
//         //use Bluestein algorithm
//         MHO_BitReversalPermutation::ComputeBitReversedIndicesBaseTwo(fM, fPermutation);
//         MHO_FastFourierTransformUtilities<XFloatType>::ComputeTwiddleFactors(fM, fTwiddle);
//         MHO_FastFourierTransformUtilities<XFloatType>::ComputeConjugateTwiddleFactors(fM, fConjugateTwiddle);
//         MHO_FastFourierTransformUtilities<XFloatType>::ComputeBluesteinScaleFactors(fN, fScale);
//         MHO_FastFourierTransformUtilities<XFloatType>::ComputeBluesteinCirculantVector(fN, fM, fTwiddle, fScale, fCirculant);
//     }
//     else
//     {
//         //can do an in-place transform,
//         //only need space for the permutation array, and twiddle factors
//         fPermutation = new unsigned int[fN];
//         fTwiddle = new std::complex<XFloatType>[fN];
//         fConjugateTwiddle = new std::complex<XFloatType>[fN];
//         //use radix-2
//         MHO_BitReversalPermutation::ComputeBitReversedIndicesBaseTwo(fN, fPermutation);
//         MHO_FastFourierTransformUtilities<XFloatType>::ComputeTwiddleFactors(fN, fTwiddle);
//         MHO_FastFourierTransformUtilities<XFloatType>::ComputeConjugateTwiddleFactors(fN, fConjugateTwiddle);
//     }
// }
// 
// 
// template< typename XFloatType >
// void
// MHO_FastFourierTransform<XFloatType>::DealocateWorkspace()
// {
//     delete[] fPermutation; fPermutation = NULL;
//     delete[] fTwiddle; fTwiddle = NULL;
//     delete[] fConjugateTwiddle; fConjugateTwiddle = NULL;
//     delete[] fScale; fScale = NULL;
//     delete[] fCirculant; fCirculant = NULL;
//     delete[] fWorkspace; fWorkspace = NULL;
// }
// 






}


#endif /* MHO_FastFourierTransform_H__ */
