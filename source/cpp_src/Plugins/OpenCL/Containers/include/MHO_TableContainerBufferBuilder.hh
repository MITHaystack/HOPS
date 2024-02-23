#ifndef MHO_TableContainerBufferBuilder_HH__
#define MHO_TableContainerBufferBuilder_HH__

/*!
*@file MHO_TableContainerBufferBuilder.hh
*@class MHO_TableContainerBufferBuilder
*@author J. Barrett - barrettj@mit.edu 
*
*@date
*@brief
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

#endif /*! end of include guard: MHO_TableContainerBufferBuilder */
