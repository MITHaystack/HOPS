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

/**
 * @brief Class MHO_ManualPolDelayCorrectionBuilder
 */
class MHO_ManualPolDelayCorrectionBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_ManualPolDelayCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata)
            : MHO_OperatorBuilder(toolbox, fdata){};

        MHO_ManualPolDelayCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                                            MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_ManualPolDelayCorrectionBuilder(){};

        /**
         * @brief Constructs and adds a manual phase delay correction builder to the toolbox.
         * 
         * @return bool indicating success
         */
        virtual bool Build() override;

    private:
        /**
         * @brief Parses polarisation from name and returns corresponding string.
         * 
         * @param name Input polarisation name to parse
         * @return Corresponding string for polarisation ('X', 'Y', 'R', 'L' or '?')
         */
        std::string ParsePolFromName(const std::string& name);
        /**
         * @brief Extracts and returns the station identifier from the conditions vector.
         * 
         * @return The extracted station identifier as a string.
         */
        std::string ExtractStationIdentifier();
};

} // namespace hops

#endif /*! end of include guard: MHO_ManualPolDelayCorrectionBuilderBuilder_HH__ */
