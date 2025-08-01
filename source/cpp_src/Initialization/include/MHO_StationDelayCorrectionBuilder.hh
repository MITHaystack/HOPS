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
 *@brief Builds a station delay operator
 */

/**
 * @brief Class MHO_StationDelayCorrectionBuilder
 */
class MHO_StationDelayCorrectionBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_StationDelayCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata)
            : MHO_OperatorBuilder(toolbox, fdata){};

        MHO_StationDelayCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                                          MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_StationDelayCorrectionBuilder(){};

        /**
         * @brief Constructs and initializes the MHO_StationDelayCorrection operator, and adds to toolbox
         *
         * @return bool indicating success of construction
         */
        virtual bool Build() override;

    private:
        /**
         * @brief Function ParsePolFromName
         *
         * @param name (const std::string&)
         * @return Return value (std::string)
         */
        std::string ParsePolFromName(const std::string& name);
        /**
         * @brief Extracts and returns the first station identifier found in the conditions vector.
         *
         * @return The extracted station identifier as a string.
         */
        std::string ExtractStationIdentifier();
};

} // namespace hops

#endif /*! end of include guard: MHO_StationDelayCorrectionBuilderBuilder_HH__ */
