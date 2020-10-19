#ifndef MHOAxis_HH__
#define MHOAxis_HH__

/*
*File: MHOAxis.hh
*Class: MHOAxis
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include "MHOVectorContainer.hh"
#include "MHOInterval.hh"
#include "MHOIntervalLabel.hh"
#include "MHOIntervalLabelTree.hh"

namespace hops
{

template< typename XValueType >
class MHOAxis: public MHOVectorContainer< XValueType >, public MHOIntervalLabelTree
{

    public:
        MHOAxis():
            MHOVectorContainer<XValueType>(),
            MHOIntervalLabelTree()
        {};

        virtual ~MHOAxis(){};

    private:

};

}

#endif /* end of include guard: MHOAxis */