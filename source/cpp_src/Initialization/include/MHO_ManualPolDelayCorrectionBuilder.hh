#ifndef MHO_ManualPolDelayCorrectionBuilderBuilder_HH__
#define MHO_ManualPolDelayCorrectionBuilderBuilder_HH__

#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_ManualPolDelayCorrectionBuilder.hh
 *@class MHO_ManualPolDelayCorrectionBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date Tue Jun 20 12:35:56 2023 -0400
 *@brief
 */

class MHO_ManualPolDelayCorrectionBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_ManualPolDelayCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                                            MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_ManualPolDelayCorrectionBuilder(){};

        virtual bool Build() override;

    private:
        std::string ParsePolFromName(const std::string& name);
        std::string ExtractStationIdentifier();
};

} // namespace hops

#endif /*! end of include guard: MHO_ManualPolDelayCorrectionBuilderBuilder_HH__ */
