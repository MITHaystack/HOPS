#ifndef MHO_LinearDParCorrectionBuilderBuilder_HH__
#define MHO_LinearDParCorrectionBuilderBuilder_HH__


#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
*@file MHO_LinearDParCorrectionBuilder.hh
*@class MHO_LinearDParCorrectionBuilder
*@author J. Barrett - barrettj@mit.edu
*@date Mon Jan 15 11:23:36 2024 -0500
*@brief
*/


class MHO_LinearDParCorrectionBuilder:
    public MHO_OperatorBuilder
{
    public:

        MHO_LinearDParCorrectionBuilder(MHO_OperatorToolbox* toolbox,
                                                MHO_ContainerStore* cstore = nullptr,
                                                MHO_ParameterStore* pstore = nullptr):
            MHO_OperatorBuilder(toolbox, cstore, pstore)
        {};

        virtual ~MHO_LinearDParCorrectionBuilder(){};

        virtual bool Build() override;

    private:

        // std::string ParsePolFromName(const std::string& name);
        // std::string ExtractStationMk4ID();
};

}//end namespace


#endif /*! end of include guard: MHO_LinearDParCorrectionBuilderBuilder_HH__ */
