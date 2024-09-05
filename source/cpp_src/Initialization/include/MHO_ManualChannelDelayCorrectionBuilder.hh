#ifndef MHO_ManualChannelDelayCorrectionBuilderBuilder_HH__
#define MHO_ManualChannelDelayCorrectionBuilderBuilder_HH__


#include "MHO_OperatorBuilder.hh"
#include "MHO_ChannelQuantity.hh"

namespace hops
{

/*!
*@file MHO_ManualChannelDelayCorrectionBuilder.hh
*@class MHO_ManualChannelDelayCorrectionBuilder
*@author J. Barrett - barrettj@mit.edu
*@date Wed May 31 17:11:03 2023 -0400
*@brief
*/

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
        std::string ExtractStationIdentifier();
};

}//end namespace


#endif /*! end of include guard: MHO_ManualChannelDelayCorrectionBuilderBuilder_HH__ */
