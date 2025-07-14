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
 *@brief build a manual per-polarization pc_delay operator
 */

/**
 * @brief Class MHO_ManualPolPhaseCorrectionBuilder
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

        /**
         * @brief Constructs and adds a new MHO_ManualPolPhaseCorrection operator to the toolbox
         * 
         * @return bool indicating success
         */
        virtual bool Build() override;

    private:
        /**
         * @brief Parses a polarization string from a given name.
         * 
         * @param name Input name to parse polarization from.
         * @return Polarization string ('X', 'Y', 'R', 'L' or '?').
         */
        std::string ParsePolFromName(const std::string& name);
        /**
         * @brief Extracts and returns the station identifier from the conditions vector.
         * 
         * @return Station identifier as a string.
         */
        std::string ExtractStationIdentifier();
};

} // namespace hops

#endif /*! end of include guard: MHO_ManualPolPhaseCorrectionBuilderBuilder_HH__ */
