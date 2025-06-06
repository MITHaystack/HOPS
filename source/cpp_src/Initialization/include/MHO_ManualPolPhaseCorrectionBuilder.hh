#ifndef MHO_ManualPolPhaseCorrectionBuilderBuilder_HH__
#define MHO_ManualPolPhaseCorrectionBuilderBuilder_HH__

#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_ManualPolPhaseCorrectionBuilder.hh
 *@class MHO_ManualPolPhaseCorrectionBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date Tue Jun 20 12:35:56 2023 -0400
 *@brief
 */

class MHO_ManualPolPhaseCorrectionBuilder: public MHO_OperatorBuilder
{
    public:

        MHO_ManualPolPhaseCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata)
            : MHO_OperatorBuilder(toolbox, fdata){};

        MHO_ManualPolPhaseCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                                            MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_ManualPolPhaseCorrectionBuilder(){};

        virtual bool Build() override;

    private:
        std::string ParsePolFromName(const std::string& name);
        std::string ExtractStationIdentifier();
};

} // namespace hops

#endif /*! end of include guard: MHO_ManualPolPhaseCorrectionBuilderBuilder_HH__ */
