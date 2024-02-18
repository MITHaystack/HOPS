#ifndef MHO_ManualChannelDelayCorrectionBuilderBuilder_HH__
#define MHO_ManualChannelDelayCorrectionBuilderBuilder_HH__

/*
*File: MHO_ManualChannelDelayCorrectionBuilder.hh
*Class: MHO_ManualChannelDelayCorrectionBuilder
*Author:
*Email:
*Date:
*Description:
*/

#include "MHO_OperatorBuilder.hh"
#include "MHO_ChannelQuantity.hh"

namespace hops
{

class MHO_ManualChannelDelayCorrectionBuilder:
    public MHO_OperatorBuilder,
    public MHO_ChannelQuantity
{
    public:

        MHO_ManualChannelDelayCorrectionBuilder(MHO_OperatorToolbox* toolbox,
                                                MHO_ContainerStore* cstore = nullptr,
                                                MHO_ParameterStore* pstore = nullptr):
            MHO_OperatorBuilder(toolbox, cstore, pstore),
            MHO_ChannelQuantity()
        {};

        virtual ~MHO_ManualChannelDelayCorrectionBuilder(){};

        virtual bool Build() override;

    private:

        std::string ParsePolFromName(const std::string& name);
        std::string ExtractStationMk4ID();
};

}//end namespace


#endif /* end of include guard: MHO_ManualChannelDelayCorrectionBuilderBuilder_HH__ */
