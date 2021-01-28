#ifndef MHO_Axis_HH__
#define MHO_Axis_HH__

/*
*File: MHO_Axis.hh
*Class: MHO_Axis
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include "MHO_VectorContainer.hh"
#include "MHO_Interval.hh"
#include "MHO_IntervalLabel.hh"
#include "MHO_IntervalLabelTree.hh"

namespace hops
{

template< typename XValueType >
class MHO_Axis: public MHO_VectorContainer< XValueType >, public MHO_IntervalLabelTree
{

    public:
        MHO_Axis():
            MHO_VectorContainer<XValueType>(),
            MHO_IntervalLabelTree()
        {};

        virtual ~MHO_Axis(){};

    private:

};

}

#endif /* end of include guard: MHO_Axis */