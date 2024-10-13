#ifndef MHO_StationDelayCorrectionBuilderBuilder_HH__
#define MHO_StationDelayCorrectionBuilderBuilder_HH__

#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_StationDelayCorrectionBuilder.hh
 *@class MHO_StationDelayCorrectionBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date Tue Jun 20 12:35:56 2023 -0400
 *@brief
 */

class MHO_StationDelayCorrectionBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_StationDelayCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                                          MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_StationDelayCorrectionBuilder(){};

        virtual bool Build() override;

    private:
        std::string ParsePolFromName(const std::string& name);
        std::string ExtractStationIdentifier();
};

} // namespace hops

#endif /*! end of include guard: MHO_StationDelayCorrectionBuilderBuilder_HH__ */
