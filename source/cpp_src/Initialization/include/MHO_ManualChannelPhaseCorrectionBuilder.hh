#ifndef MHO_ManualChannelPhaseCorrectionBuilderBuilder_HH__
#define MHO_ManualChannelPhaseCorrectionBuilderBuilder_HH__

/*!
*@file MHO_ManualChannelPhaseCorrectionBuilder.hh
*@class MHO_ManualChannelPhaseCorrectionBuilder
*@author J. Barrett - barrettj@mit.edu
*@date
*@brief
*/

#include "MHO_OperatorBuilder.hh"
#include "MHO_ChannelQuantity.hh"

namespace hops
{

class MHO_ManualChannelPhaseCorrectionBuilder:
    public MHO_OperatorBuilder,
    public MHO_ChannelQuantity
{
    public:

        MHO_ManualChannelPhaseCorrectionBuilder(MHO_OperatorToolbox* toolbox,
                                                MHO_ContainerStore* cstore = nullptr,
                                                MHO_ParameterStore* pstore = nullptr):
            MHO_OperatorBuilder(toolbox, cstore, pstore),
            MHO_ChannelQuantity()
        {};

        virtual ~MHO_ManualChannelPhaseCorrectionBuilder(){};

        virtual bool Build() override;

    private:

        std::string ParsePolFromName(const std::string& name);
        std::string ExtractStationMk4ID();
};

}//end namespace


#endif /*! end of include guard: MHO_ManualChannelPhaseCorrectionBuilderBuilder_HH__ */
