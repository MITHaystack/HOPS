#ifndef MHO_FastFourierTransformCalls_HH__
#define MHO_FastFourierTransformCalls_HH__

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstddef>

#include "MHO_BitReversalPermutation.hh"
#include "MHO_FastFourierTransformUtilities.hh"
#include "MHO_FastFourierTransformWorkspace.hh"
#include "MHO_Message.hh"

namespace hops
{

/*!
 *@file MHO_FastFourierTransformCalls.hh
 *@class MHO_FastFourierTransformCalls
 *@date Tue Aug 1 15:53:56 2023 -0400
 *@brief native FFT implemenation calls
 *@author J. Barrett - barrettj@mit.edu
 */

template< typename XFloatType >
void FFTRadix2(std::complex< XFloatType >* data, MHO_FastFourierTransformWorkspace< XFloatType >& work, bool isForward,
               unsigned int stride = 1)
{
    //for DFT we conjugate first (NOTE: this matches FFTW3 convention)
    if(isForward)
    {
        MHO_FastFourierTransformUtilities< XFloatType >::Conjugate(work.fN, data, stride);
    }

    //use radix-2
    MHO_BitReversalPermutation::PermuteArray< std::complex< XFloatType > >(work.fN, work.fPermutation, data, stride);
    MHO_FastFourierTransformUtilities< XFloatType >::FFTRadixTwo_DIT(work.fN, data, work.fTwiddle, stride);

    //for DFT we conjugate again (NOTE: this matches FFTW3 convention)
    if(isForward)
    {
        MHO_FastFourierTransformUtilities< XFloatType >::Conjugate(work.fN, data, stride);
    }
}

//workspace is expected to be initialized before call
template< typename XFloatType >
void FFTBluestein(std::complex< XFloatType >* data, MHO_FastFourierTransformWorkspace< XFloatType >& work, bool isForward,
                  unsigned int stride = 1)
{
    //for DFT we conjugate first (NOTE: this matches FFTW3 convention)
    if(isForward)
    {
        MHO_FastFourierTransformUtilities< XFloatType >::Conjugate(work.fN, data, stride);
    }

    //use bluestein algorithm for arbitrary (non-power of 2) N
    MHO_FastFourierTransformUtilities< XFloatType >::FFTBluestein(work.fN, work.fM, data, work.fTwiddle, work.fConjugateTwiddle,
                                                                  work.fScale, work.fCirculant, work.fWorkspace, stride);

    //for DFT we conjugate again (NOTE: this matches FFTW3 convention)
    if(isForward)
    {
        MHO_FastFourierTransformUtilities< XFloatType >::Conjugate(work.fN, data, stride);
    }
}

} // namespace hops

#endif /*! MHO_FastFourierTransformCalls_HH__ */
