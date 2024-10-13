#ifndef MHO_TableContainerBufferBuilder_HH__
#define MHO_TableContainerBufferBuilder_HH__

#include "MHO_ExtensibleElement.hh"
#include "MHO_TableContainer.hh"

namespace hops
{

/*!
 *@file MHO_TableContainerBufferBuilder.hh
 *@class MHO_TableContainerBufferBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date
 *@brief
 */

template< typename XValueType, typename XAxisPackType >
class MHO_TableContainerBufferBuilder: public MHO_ExtendedElement< MHO_TableContainerBuffer >::ExtendedVisitor
{
    public:
        MHO_TableContainerBufferBuilder();
        virtual ~MHO_TableContainerBufferBuilder();

    private:
};

} // namespace hops

#endif /*! end of include guard: MHO_TableContainerBufferBuilder */
