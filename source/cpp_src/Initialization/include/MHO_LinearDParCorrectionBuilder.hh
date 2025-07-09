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
 *@brief builds a delta-parallactic angle correciton operator
 */

/**
 * @brief Class MHO_LinearDParCorrectionBuilder
 */
class MHO_LinearDParCorrectionBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_LinearDParCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata)
            : MHO_OperatorBuilder(toolbox, fdata){};

        MHO_LinearDParCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                                        MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_LinearDParCorrectionBuilder(){};

        /**
         * @brief Constructs and initializes the linear-pol DPar (delta parallactic angle) correction operator.
         * 
         * @return True if construction is successful, false otherwise.
         */
        virtual bool Build() override;

    private:
        // std::string ParsePolFromName(const std::string& name);
        // std::string ExtractStationMk4ID();
};

} // namespace hops

#endif /*! end of include guard: MHO_LinearDParCorrectionBuilderBuilder_HH__ */
