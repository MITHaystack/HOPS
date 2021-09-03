#ifndef MHO_NormFX_HH__
#define MHO_NormFX_HH__

/*
*File: MHO_NormFX.hh
*Class: MHO_NormFX
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <cmath>
#include <complex>

#include "MHO_FFTWTypes.hh"
#include "MHO_TableContainer.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_BinaryNDArrayOperator.hh"

#include "MHO_NaNMasker.hh"
#include "MHO_FunctorBroadcaster.hh"
#include "MHO_ComplexConjugator.hh"
#include "MHO_ScalarMultiply.hh"
#include "MHO_CyclicRotator.hh"
#include "MHO_SubSample.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"
#include "MHO_MultidimensionalPaddedFastFourierTransform.hh"




namespace hops
{


class MHO_NormFX: public MHO_BinaryNDArrayOperator<
    ch_baseline_data_type,
    ch_baseline_weight_type,
    ch_baseline_sbd_type >
{
    public:
        MHO_NormFX();
        virtual ~MHO_NormFX();

        virtual bool Initialize() override;
        virtual bool ExecuteOperation() override;

    private:

        std::size_t fInDims[CH_VIS_NDIM];
        std::size_t fWorkDims[CH_VIS_NDIM];
        std::size_t fOutDims[CH_VIS_NDIM];

        //only needed for the old routine
        MHO_MultidimensionalFastFourierTransform<double,1> fFFTEngine;
        MHO_NDArrayWrapper< std::complex<double>, 1 > xp_spec;
        MHO_NDArrayWrapper< std::complex<double>, 1 > S;
        MHO_NDArrayWrapper< std::complex<double>, 1 > xlag;

        //sub operations, eventually should be moved out of this class
        MHO_NaNMasker<ch_baseline_data_type, ch_baseline_data_type> fNaNMasker; //replace NaNs in data with zero
        MHO_FunctorBroadcaster<ch_baseline_data_type, ch_baseline_data_type> fNaNBroadcaster;
        MHO_FunctorBroadcaster<ch_baseline_data_type, ch_baseline_data_type> fConjBroadcaster;
        MHO_FunctorBroadcaster<ch_baseline_data_type, ch_baseline_data_type> fNormBroadcaster;
        MHO_ComplexConjugator<ch_baseline_data_type, ch_baseline_data_type> fConjugator; //complex conjugate

        MHO_MultidimensionalPaddedFastFourierTransform<VFP_TYPE, CH_VIS_NDIM> fPaddedFFTEngine;

        MHO_SubSample<ch_baseline_sbd_type, ch_baseline_sbd_type> fSubSampler;
        MHO_CyclicRotator<ch_baseline_sbd_type, ch_baseline_sbd_type> fCyclicRotator;
        MHO_ScalarMultiply<double, ch_baseline_sbd_type, ch_baseline_sbd_type> fScalarMultiplier;


        ch_baseline_sbd_type fWorkspace;
        bool fInitialized;
        bool fIsUSB;

};


}


#endif /* end of include guard: MHO_NormFX */
