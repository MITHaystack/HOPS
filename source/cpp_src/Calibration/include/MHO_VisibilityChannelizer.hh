#ifndef MHO_VisibilityChannelizer_HH__
#define MHO_VisibilityChannelizer_HH__

/*
*File: MHO_VisibilityChannelizer.hh
*Class: MHO_VisibilityChannelizer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include "MHO_NDArrayWrapper.hh"
#include "MHO_NDArrayOperator.hh"

#include "MHO_Visibilities.hh"
#include "MHO_ChannelizedVisibilities.hh"

namespace hops
{

class MHO_VisibilityChannelizer: public MHO_NDArrayOperator< baseline_data_type, ch_baseline_data_type>
{
    public:
        MHO_VisibilityChannelizer();
        virtual ~MHO_VisibilityChannelizer();

        virtual bool Initialize() override;
        virtual bool Execute() override;

    private:

        bool fInitialized;

};

}

#endif /* end of include guard: MHO_VisibilityChannelizer */