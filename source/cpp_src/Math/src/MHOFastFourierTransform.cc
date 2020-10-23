#include <cstring>
#include <cmath>
#include <iostream>
#include <cstddef>

#include "MHOFastFourierTransform.hh"

#include "MHOBitReversalPermutation.hh"
#include "MHOFastFourierTransformUtilities.hh"

namespace hops
{



MHOFastFourierTransform::MHOFastFourierTransform()
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

MHOFastFourierTransform::~MHOFastFourierTransform()
{
    DealocateWorkspace();
}

void
MHOFastFourierTransform::SetSize(unsigned int N)
{
    if(N != fN)
    {
        fN = N;
        fSizeIsPowerOfTwo = MHOBitReversalPermutation::IsPowerOfTwo(N);
        fM = MHOFastFourierTransformUtilities::ComputeBluesteinArraySize(N);
        fInitialized = false;
    }
}

void
MHOFastFourierTransform::SetForward(){fForward = true;}

void
MHOFastFourierTransform::SetBackward(){fForward = false;}

void
MHOFastFourierTransform::Initialize()
{
    // std::cout<<"fN = "<<fN<<std::endl;
    // std::cout<<"input size = "<<fInput->GetSize()<<std::endl;
    // std::cout<<"output size = "<<fOutput->GetSize()<<std::endl;
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
            MHOBitReversalPermutation::ComputeBitReversedIndicesBaseTwo(fN, fPermutation);
            MHOFastFourierTransformUtilities::ComputeTwiddleFactors(fN, fTwiddle);
            MHOFastFourierTransformUtilities::ComputeConjugateTwiddleFactors(fN, fConjugateTwiddle);
        }
        else
        {
            //use Bluestein algorithm
            MHOBitReversalPermutation::ComputeBitReversedIndicesBaseTwo(fM, fPermutation);
            MHOFastFourierTransformUtilities::ComputeTwiddleFactors(fM, fTwiddle);
            MHOFastFourierTransformUtilities::ComputeConjugateTwiddleFactors(fM, fConjugateTwiddle);
            MHOFastFourierTransformUtilities::ComputeBluesteinScaleFactors(fN, fScale);
            MHOFastFourierTransformUtilities::ComputeBluesteinCirculantVector(fN, fM, fTwiddle, fScale, fCirculant);
        }

        fIsValid = true;
        fInitialized = true;

        //std::cout<<"initialize = "<<fInitialized<<" and valid = "<<fIsValid<<std::endl;
    }
}

///Make a call to execute the FFT plan and perform the transformation
void
MHOFastFourierTransform::ExecuteOperation()
{
    //std::cout<<"initialize = "<<fInitialized<<" and valid = "<<fIsValid<<std::endl;

    if(fIsValid)
    {
        //if input and output point to the same array, don't bother copying data over
        if(fInput != fOutput)
        {
            //the arrays are not identical so copy the input over to the output
            std::memcpy( (void*) fOutput->GetRawData(), (void*) fInput->GetRawData(), fN*sizeof(std::complex<double>) );
        }


        if(!fForward) //for IDFT we conjugate first
        {
            std::complex<double>* data = fOutput->GetRawData();
            for(unsigned int i=0; i<fN; i++)
            {
                data[i] = std::conj(data[i]);
            }
        }

        if(fSizeIsPowerOfTwo)
        {
            //use radix-2
            MHOBitReversalPermutation::PermuteArray< std::complex<double> >(fN, fPermutation, fOutput->GetRawData());
            MHOFastFourierTransformUtilities::FFTRadixTwo_DIT(fN, fOutput->GetRawData(), fTwiddle);
        }
        else
        {
            //use bluestein algorithm for arbitrary N
            MHOFastFourierTransformUtilities::FFTBluestein(fN, fM, fOutput->GetRawData(), fTwiddle, fConjugateTwiddle, fScale, fCirculant, fWorkspace);
        }

        if(!fForward) //for IDFT we conjugate again
        {
            std::complex<double>* data = fOutput->GetRawData();
            for(unsigned int i=0; i<fN; i++)
            {
                data[i] = std::conj(data[i]);
            }
        }
    }
    else
    {
        //warning
        std::cout<<"MHOFastFourierTransform::ExecuteOperation: Warning, transform not valid. Aborting."<<std::endl;
    }
}

void
MHOFastFourierTransform::AllocateWorkspace()
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
MHOFastFourierTransform::DealocateWorkspace()
{
    delete[] fPermutation; fPermutation = NULL;
    delete[] fTwiddle; fTwiddle = NULL;
    delete[] fConjugateTwiddle; fConjugateTwiddle = NULL;
    delete[] fScale; fScale = NULL;
    delete[] fCirculant; fCirculant = NULL;
    delete[] fWorkspace; fWorkspace = NULL;
}

}
