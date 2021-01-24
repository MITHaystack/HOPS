#include <cstring>
#include <cmath>
#include <iostream>
#include <cstddef>

#include "MHO_Message.hh"
#include "MHO_FastFourierTransform.hh"
#include "MHO_BitReversalPermutation.hh"
#include "MHO_FastFourierTransformUtilities.hh"

namespace hops
{

MHO_FastFourierTransform::MHO_FastFourierTransform()
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

MHO_FastFourierTransform::~MHO_FastFourierTransform()
{
    DealocateWorkspace();
}

void
MHO_FastFourierTransform::SetSize(unsigned int N)
{
    if(N != fN)
    {
        fN = N;
        fSizeIsPowerOfTwo = MHO_BitReversalPermutation::IsPowerOfTwo(N);
        fM = MHO_FastFourierTransformUtilities::ComputeBluesteinArraySize(N);
        fInitialized = false;
    }
}

void
MHO_FastFourierTransform::SetForward(){fForward = true;}

void
MHO_FastFourierTransform::SetBackward(){fForward = false;}

bool
MHO_FastFourierTransform::Initialize()
{
    if(fInput->GetSize() != fN || fOutput->GetSize() != fN)
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
            MHO_FastFourierTransformUtilities::ComputeTwiddleFactors(fN, fTwiddle);
            MHO_FastFourierTransformUtilities::ComputeConjugateTwiddleFactors(fN, fConjugateTwiddle);
        }
        else
        {
            //use Bluestein algorithm
            MHO_BitReversalPermutation::ComputeBitReversedIndicesBaseTwo(fM, fPermutation);
            MHO_FastFourierTransformUtilities::ComputeTwiddleFactors(fM, fTwiddle);
            MHO_FastFourierTransformUtilities::ComputeConjugateTwiddleFactors(fM, fConjugateTwiddle);
            MHO_FastFourierTransformUtilities::ComputeBluesteinScaleFactors(fN, fScale);
            MHO_FastFourierTransformUtilities::ComputeBluesteinCirculantVector(fN, fM, fTwiddle, fScale, fCirculant);
        }

        fIsValid = true;
        fInitialized = true;
    }

    return (fInitialized && fIsValid);
}

///Make a call to execute the FFT plan and perform the transformation
bool
MHO_FastFourierTransform::ExecuteOperation()
{
    if(fIsValid && fInitialized)
    {
        //if input and output point to the same array, don't bother copying data over
        if(fInput != fOutput)
        {
            //the arrays are not identical so copy the input over to the output
            std::memcpy( (void*) fOutput->GetData(), (void*) fInput->GetData(), fN*sizeof(std::complex<double>) );
        }


        //for DFT we conjugate first (NOTE: this matches FFTW3 convention)
        if(fForward)
        {
            std::complex<double>* data = fOutput->GetData();
            for(unsigned int i=0; i<fN; i++)
            {
                data[i] = std::conj(data[i]);
            }
        }

        if(fSizeIsPowerOfTwo)
        {
            //use radix-2
            MHO_BitReversalPermutation::PermuteArray< std::complex<double> >(fN, fPermutation, fOutput->GetData());
            MHO_FastFourierTransformUtilities::FFTRadixTwo_DIT(fN, fOutput->GetData(), fTwiddle);
        }
        else
        {
            //use bluestein algorithm for arbitrary N
            MHO_FastFourierTransformUtilities::FFTBluestein(fN, fM, fOutput->GetData(), fTwiddle, fConjugateTwiddle, fScale, fCirculant, fWorkspace);
        }

        //for DFT we conjugate again (NOTE: this matches FFTW3 convention)
        if(fForward)
        {
            std::complex<double>* data = fOutput->GetData();
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

void
MHO_FastFourierTransform::AllocateWorkspace()
{
    if(!fSizeIsPowerOfTwo)
    {
        //can't perform an in-place transform, need workspace
        fPermutation = new unsigned int[fM];
        fTwiddle = new std::complex<double>[fM];
        fConjugateTwiddle = new std::complex<double>[fM];
        fScale = new std::complex<double>[fN];
        fCirculant = new std::complex<double>[fM];
        fWorkspace = new std::complex<double>[fM];
    }
    else
    {
        //can do an in-place transform,
        //only need space for the permutation array, and twiddle factors
        fPermutation = new unsigned int[fN];
        fTwiddle = new std::complex<double>[fN];
        fConjugateTwiddle = new std::complex<double>[fN];
    }
}

void
MHO_FastFourierTransform::DealocateWorkspace()
{
    delete[] fPermutation; fPermutation = NULL;
    delete[] fTwiddle; fTwiddle = NULL;
    delete[] fConjugateTwiddle; fConjugateTwiddle = NULL;
    delete[] fScale; fScale = NULL;
    delete[] fCirculant; fCirculant = NULL;
    delete[] fWorkspace; fWorkspace = NULL;
}

}
