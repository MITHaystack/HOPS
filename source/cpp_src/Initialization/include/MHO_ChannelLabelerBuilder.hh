#ifndef MHO_ChannelLabelerBuilderBuilder_HH__
#define MHO_ChannelLabelerBuilderBuilder_HH__



#include "MHO_OperatorBuilder.hh"
#include "MHO_ChannelQuantity.hh"

namespace hops
{

/*!
*@file MHO_ChannelLabelerBuilder.hh
*@class MHO_ChannelLabelerBuilder
*@author
*@date Fri Jun 2 16:09:43 2023 -0400
*@brief
*/

class MHO_ChannelLabelerBuilder:
    public MHO_OperatorBuilder,
    public MHO_ChannelQuantity
{
    public:

        MHO_ChannelLabelerBuilder(MHO_OperatorToolbox* toolbox,
                                   MHO_ContainerStore* cstore = nullptr,
                                   MHO_ParameterStore* pstore = nullptr):
            MHO_OperatorBuilder(toolbox, cstore, pstore),
            MHO_ChannelQuantity()
        {};

        virtual ~MHO_ChannelLabelerBuilder(){};

        virtual bool Build() override;

    private:

};

}//end namespace


#endif /*! end of include guard: MHO_ChannelLabelerBuilderBuilder_HH__ */
