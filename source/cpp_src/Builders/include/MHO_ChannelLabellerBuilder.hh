#ifndef MHO_ChannelLabellerBuilderBuilder_HH__
#define MHO_ChannelLabellerBuilderBuilder_HH__

/*
*File: MHO_ChannelLabellerBuilder.hh
*Class: MHO_ChannelLabellerBuilder
*Author:
*Email:
*Date:
*Description:
*/

#include "MHO_OperatorBuilder.hh"
#include "MHO_ChannelQuantity.hh"

namespace hops
{

class MHO_ChannelLabellerBuilder: 
    public MHO_OperatorBuilder,
    public MHO_ChannelQuantity
{
    public:

        MHO_ChannelLabellerBuilder(MHO_OperatorToolbox* toolbox, 
                                   MHO_ContainerStore* cstore = nullptr,
                                   MHO_ParameterStore* pstore = nullptr):
            MHO_OperatorBuilder(toolbox, cstore, pstore),
            MHO_ChannelQuantity()
            {};

        virtual ~MHO_ChannelLabellerBuilder(){};

        virtual bool Build() override;

    private:

};

}//end namespace


#endif /* end of include guard: MHO_ChannelLabellerBuilderBuilder_HH__ */
