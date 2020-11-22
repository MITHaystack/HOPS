#ifndef MHOVisibilityChannelizer_HH__
#define MHOVisibilityChannelizer_HH__

/*
*File: MHOVisibilityChannelizer.hh
*Class: MHOVisibilityChannelizer
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include "MHOArrayWrapper.hh"
#include "MHOArrayOperator.hh"

#include "MHOVisibilities.hh"
#include "MHOChannelizedVisibilities.hh"

namespace hops
{

class MHOVisibilityChannelizer: public MHOArrayOperator< baseline_data_type, ch_baseline_data_type>
{
    public:
        MHOVisibilityChannelizer();
        virtual ~MHOVisibilityChannelizer();

        virtual bool Initialize() override;
        virtual bool ExecuteOperation() override;

    private:

        bool fInitialized;

};

}

#endif /* end of include guard: MHOVisibilityChannelizer */