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
#include "MHOUnaryArrayOperator.hh"

#include "MHOVisibilities.hh"
#include "MHOChannelizedVisibilities.hh"

class MHOVisibilityChannelizer: public MHOUnaryArrayOperator<>
{
    public:
        MHOVisibilityChannelizer();
        virtual ~MHOVisibilityChannelizer();
    private:

};

#endif /* end of include guard: MHOVisibilityChannelizer */