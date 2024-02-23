#ifndef MHO_ManualPolDelayCorrectionBuilderBuilder_HH__
#define MHO_ManualPolDelayCorrectionBuilderBuilder_HH__

/*!
*@file MHO_ManualPolDelayCorrectionBuilder.hh
*@class MHO_ManualPolDelayCorrectionBuilder
*@author
*Email:
*@date
*@brief
*/

#include "MHO_OperatorBuilder.hh"

namespace hops
{

class MHO_ManualPolDelayCorrectionBuilder:
    public MHO_OperatorBuilder
{
    public:

        MHO_ManualPolDelayCorrectionBuilder(MHO_OperatorToolbox* toolbox,
                                                MHO_ContainerStore* cstore = nullptr,
                                                MHO_ParameterStore* pstore = nullptr):
            MHO_OperatorBuilder(toolbox, cstore, pstore)
        {};

        virtual ~MHO_ManualPolDelayCorrectionBuilder(){};

        virtual bool Build() override;

    private:

        std::string ParsePolFromName(const std::string& name);
        std::string ExtractStationMk4ID();
};

}//end namespace


#endif /*! end of include guard: MHO_ManualPolDelayCorrectionBuilderBuilder_HH__ */
