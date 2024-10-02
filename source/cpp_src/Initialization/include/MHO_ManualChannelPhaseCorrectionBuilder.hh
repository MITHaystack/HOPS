#ifndef MHO_ManualChannelPhaseCorrectionBuilderBuilder_HH__
#define MHO_ManualChannelPhaseCorrectionBuilderBuilder_HH__

#include "MHO_ChannelQuantity.hh"
#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_ManualChannelPhaseCorrectionBuilder.hh
 *@class MHO_ManualChannelPhaseCorrectionBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date Wed May 31 17:11:03 2023 -0400
 *@brief
 */

class MHO_ManualChannelPhaseCorrectionBuilder: public MHO_OperatorBuilder, public MHO_ChannelQuantity
{
    public:
        MHO_ManualChannelPhaseCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                                                MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore), MHO_ChannelQuantity(){};

        virtual ~MHO_ManualChannelPhaseCorrectionBuilder(){};

        virtual bool Build() override;

    private:
        std::string ParsePolFromName(const std::string& name);
        std::string ExtractStationIdentifier();
};

} // namespace hops

#endif /*! end of include guard: MHO_ManualChannelPhaseCorrectionBuilderBuilder_HH__ */
