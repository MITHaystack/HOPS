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

#include "MHO_TableContainer.hh"
#include "MHO_ChannelizedVisibilities.hh"

namespace hops
{

class MHO_NormFX: public MHO_NDArrayOperator< ch_baseline_data_type, ch_baseline_data_type >
{
    public:
        MHO_NormFX();
        virtual ~MHO_NormFX();

        virtual bool Initialize() override;
        virtual bool ExecuteOperation() override;

    private:
};


}


#endif /* end of include guard: MHO_NormFX */
