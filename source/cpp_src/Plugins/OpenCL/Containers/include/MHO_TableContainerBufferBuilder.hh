#ifndef MHO_TableContainerBufferBuilder_HH__
#define MHO_TableContainerBufferBuilder_HH__

/*
*File: MHO_TableContainerBufferBuilder.hh
*Class: MHO_TableContainerBufferBuilder
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include "MHO_TableContainer.hh"
#include "MHO_ExtensibleElement.hh"


namespace hops
{

template< typename XValueType, typename XAxisPackType >
class MHO_TableContainerBufferBuilder: public MHO_ExtendedElement< MHO_TableContainerBuffer >::ExtendedVisitor
{
    public:
        MHO_TableContainerBufferBuilder();
        virtual ~MHO_TableContainerBufferBuilder();
    private:
};

}

#endif /* end of include guard: MHO_TableContainerBufferBuilder */
