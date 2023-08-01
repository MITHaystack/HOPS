#ifndef MHO_FastFourierTransform_HH__
#define MHO_FastFourierTransform_HH__

#include <complex>
#include <cstddef>
#include <cmath>
#include <algorithm>

#include "MHO_Message.hh"
#include "MHO_BitReversalPermutation.hh"
#include "MHO_FastFourierTransformUtilities.hh"
#include "MHO_FastFourierTransformWorkspace.hh"


namespace hops
{

//workspace is expected to be initialized before call
template< typename XFloatType >
void FFTRadix2(std::complex<XFloatType>* data, MHO_FastFourierTransformWorkspace<XFloatType>& work, bool isForward, unsigned int stride = 1)
{
    //for DFT we conjugate first (NOTE: this matches FFTW3 convention)
    if(isForward){ MHO_FastFourierTransformUtilities<XFloatType>::Conjugate(work.fN, data, stride); }

    //use radix-2
    MHO_BitReversalPermutation::PermuteArray< std::complex<XFloatType> >(work.fN, work.fPermutation, data);
    MHO_FastFourierTransformUtilities<XFloatType>::FFTRadixTwo_DIT(work.fN, data, work.fTwiddle);

    //for DFT we conjugate again (NOTE: this matches FFTW3 convention)
    if(isForward){ MHO_FastFourierTransformUtilities<XFloatType>::Conjugate(work.fN, data, stride); }
}

//workspace is expected to be initialized before call
template< typename XFloatType >
void FFTBluestein(std::complex<XFloatType>* data, MHO_FastFourierTransformWorkspace<XFloatType>& work, bool isForward, unsigned int stride = 1)
{
    //for DFT we conjugate first (NOTE: this matches FFTW3 convention)
    if(isForward){ MHO_FastFourierTransformUtilities<XFloatType>::Conjugate(work.fN, data, stride); }

    //use bluestein algorithm for arbitrary (non-power of 2) N
    MHO_FastFourierTransformUtilities<XFloatType>::FFTBluestein(work.fN, work.fM, data, work.fTwiddle, work.fConjugateTwiddle, work.fScale, work.fCirculant, work.fWorkspace);

    //for DFT we conjugate again (NOTE: this matches FFTW3 convention)
    if(isForward){ MHO_FastFourierTransformUtilities<XFloatType>::Conjugate(work.fN, data, stride); }
}

}

#endif /* MHO_FastFourierTransform_HH__ */
