#include "MHO_Operator.hh"
#include <limits>

namespace hops
{
    MHO_Operator::MHO_Operator()
    {
        fPriority = std::numeric_limits<double>::max();    
    };
    
    MHO_Operator::~MHO_Operator(){};
}
