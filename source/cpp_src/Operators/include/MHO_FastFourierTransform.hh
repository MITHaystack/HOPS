#ifndef MHO_FastFourierTransform_HH__
#define MHO_FastFourierTransform_HH__

#include <complex>
#include <iostream>

#include "MHO_NDArrayWrapper.hh"
#include "MHO_NDArrayOperator.hh"
#include "MHO_BitReversalPermutation.hh"
#include "MHO_FastFourierTransformUtilities.hh"

namespace hops
{

template< typename XFloatType >
class MHO_FastFourierTransform: 
    public MHO_NDArrayOperator< MHO_NDArrayWrapper< std::complex< XFloatType >, 1>,
                             MHO_NDArrayWrapper< std::complex< XFloatType >, 1> >
{
    public:

        MHO_FastFourierTransform();
        virtual ~MHO_FastFourierTransform();

        virtual void SetSize(unsigned int N);

        virtual void SetForward();
        virtual void SetBackward();

        virtual bool Initialize() override;
        virtual bool ExecuteOperation() override;

    private:

        virtual void AllocateWorkspace();
        virtual void DealocateWorkspace();

        bool fIsValid;
        bool fForward;
        bool fInitialized;
        bool fSizeIsPowerOfTwo;

        //auxilliary workspace needed for basic 1D transform
        unsigned int fN;
        unsigned int fM;
        unsigned int* fPermutation;
        std::complex<XFloatType>* fTwiddle;
        std::complex<XFloatType>* fConjugateTwiddle;
        std::complex<XFloatType>* fScale;
        std::complex<XFloatType>* fCirculant;
        std::complex<XFloatType>* fWorkspace;

};


template< typename XFloatType >
MHO_FastFourierTransform<XFloatType>::MHO_FastFourierTransform()
{
    fIsValid = true;
    fForward = true;
    fInitialized = false;
    fSizeIsPowerOfTwo = false;

    fN = 0;
    fM = 0;
    fPermutation = NULL;
    fTwiddle = NULL;
    fConjugateTwiddle = NULL;
    fScale = NULL;
    fCirculant = NULL;
    fWorkspace = NULL;
}

template< typename XFloatType >
MHO_FastFourierTransform<XFloatType>::~MHO_FastFourierTransform()
{
    DealocateWorkspace();
}

template< typename XFloatType >
void
MHO_FastFourierTransform<XFloatType>::SetSize(unsigned int N)
{
    if(N != fN)
    {
        fN = N;
        fSizeIsPowerOfTwo = MHO_BitReversalPermutation::IsPowerOfTwo(N);
        fM = MHO_FastFourierTransformUtilities<XFloatType>::ComputeBluesteinArraySize(N);
        fInitialized = false;
    }
}

template< typename XFloatType >
void
MHO_FastFourierTransform<XFloatType>::SetForward(){fForward = true;}

template< typename XFloatType >
void
MHO_FastFourierTransform<XFloatType>::SetBackward(){fForward = false;}

template< typename XFloatType >
bool
MHO_FastFourierTransform<XFloatType>::Initialize()
{
    if(this->fInput->GetSize() != fN || this->fOutput->GetSize() != fN)
    {
        fIsValid = false;
    }

    if( !fInitialized )
    {
        //initialize
        DealocateWorkspace();
        AllocateWorkspace();

        //compute the permutation arrays and twiddle factors
        if(fSizeIsPowerOfTwo)
        {
            //use radix-2
            MHO_BitReversalPermutation::ComputeBitReversedIndicesBaseTwo(fN, fPermutation);
            MHO_FastFourierTransformUtilities<XFloatType>::ComputeTwiddleFactors(fN, fTwiddle);
            MHO_FastFourierTransformUtilities<XFloatType>::ComputeConjugateTwiddleFactors(fN, fConjugateTwiddle);
        }
        else
        {
            //use Bluestein algorithm
            MHO_BitReversalPermutation::ComputeBitReversedIndicesBaseTwo(fM, fPermutation);
            MHO_FastFourierTransformUtilities<XFloatType>::ComputeTwiddleFactors(fM, fTwiddle);
            MHO_FastFourierTransformUtilities<XFloatType>::ComputeConjugateTwiddleFactors(fM, fConjugateTwiddle);
            MHO_FastFourierTransformUtilities<XFloatType>::ComputeBluesteinScaleFactors(fN, fScale);
            MHO_FastFourierTransformUtilities<XFloatType>::ComputeBluesteinCirculantVector(fN, fM, fTwiddle, fScale, fCirculant);
        }

        fIsValid = true;
        fInitialized = true;
    }

    return (fInitialized && fIsValid);
}

///Make a call to execute the FFT plan and perform the transformation
template< typename XFloatType >
bool
MHO_FastFourierTransform<XFloatType>::ExecuteOperation()
{
    if(fIsValid && fInitialized)
    {
        //if input and output point to the same array, don't bother copying data over
        if(this->fInput != this->fOutput)
        {
            //the arrays are not identical so copy the input over to the output
            std::memcpy( (void*) this->fOutput->GetData(), (void*) this->fInput->GetData(), fN*sizeof(std::complex<XFloatType>) );
        }


        //for DFT we conjugate first (NOTE: this matches FFTW3 convention)
        if(fForward)
        {
            std::complex<XFloatType>* data = this->fOutput->GetData();
            for(unsigned int i=0; i<fN; i++)
            {
                data[i] = std::conj(data[i]);
            }
        }

        if(fSizeIsPowerOfTwo)
        {
            //use radix-2
            MHO_BitReversalPermutation::PermuteArray< std::complex<XFloatType> >(fN, fPermutation, this->fOutput->GetData());
            MHO_FastFourierTransformUtilities<XFloatType>::FFTRadixTwo_DIT(fN, this->fOutput->GetData(), fTwiddle);
        }
        else
        {
            //use bluestein algorithm for arbitrary N
            MHO_FastFourierTransformUtilities<XFloatType>::FFTBluestein(fN, fM, this->fOutput->GetData(), fTwiddle, fConjugateTwiddle, fScale, fCirculant, fWorkspace);
        }

        //for DFT we conjugate again (NOTE: this matches FFTW3 convention)
        if(fForward)
        {
            std::complex<XFloatType>* data = this->fOutput->GetData();
            for(unsigned int i=0; i<fN; i++)
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

template< typename XFloatType >
void
MHO_FastFourierTransform<XFloatType>::AllocateWorkspace()
{
    if(!fSizeIsPowerOfTwo)
    {
        //can't perform an in-place transform, need workspace
        std::cout<<"fM = "<<fM<<std::endl;
        fPermutation = new unsigned int[fM];
        fTwiddle = new std::complex<XFloatType>[fM];
        fConjugateTwiddle = new std::complex<XFloatType>[fM];
        fScale = new std::complex<XFloatType>[fN];
        fCirculant = new std::complex<XFloatType>[fM];
        fWorkspace = new std::complex<XFloatType>[fM];
    }
    else
    {
        //can do an in-place transform,
        //only need space for the permutation array, and twiddle factors
        fPermutation = new unsigned int[fN];
        fTwiddle = new std::complex<XFloatType>[fN];
        fConjugateTwiddle = new std::complex<XFloatType>[fN];
    }
}

template< typename XFloatType >
void
MHO_FastFourierTransform<XFloatType>::DealocateWorkspace()
{
    delete[] fPermutation; fPermutation = NULL;
    delete[] fTwiddle; fTwiddle = NULL;
    delete[] fConjugateTwiddle; fConjugateTwiddle = NULL;
    delete[] fScale; fScale = NULL;
    delete[] fCirculant; fCirculant = NULL;
    delete[] fWorkspace; fWorkspace = NULL;
}







}


#endif /* MHO_FastFourierTransform_H__ */
