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

#include "MHO_MultidimensionalFastFourierTransform.hh"

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
        
        MHO_MultidimensionalFastFourierTransform<double,1> fFFTEngine;

        MHO_NDArrayWrapper< std::complex<double>, 1 > xp_spec;
        MHO_NDArrayWrapper< std::complex<double>, 1 > S;
        MHO_NDArrayWrapper< std::complex<double>, 1 > xlag;

};


}


#endif /* end of include guard: MHO_NormFX */
