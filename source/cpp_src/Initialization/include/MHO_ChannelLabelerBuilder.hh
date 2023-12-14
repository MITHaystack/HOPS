#ifndef MHO_ChannelLabelerBuilderBuilder_HH__
#define MHO_ChannelLabelerBuilderBuilder_HH__

/*
*File: MHO_ChannelLabelerBuilder.hh
*Class: MHO_ChannelLabelerBuilder
*Author:
*Email:
*Date:
*Description:
*/

#include "MHO_OperatorBuilder.hh"
#include "MHO_ChannelQuantity.hh"

namespace hops
{

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


#endif /* end of include guard: MHO_ChannelLabelerBuilderBuilder_HH__ */
