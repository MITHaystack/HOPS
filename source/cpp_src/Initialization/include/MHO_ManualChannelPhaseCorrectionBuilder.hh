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
 *@brief build a manual per-channel pc_phase operator
 */

/**
 * @brief Class MHO_ManualChannelPhaseCorrectionBuilder
 */
class MHO_ManualChannelPhaseCorrectionBuilder: public MHO_OperatorBuilder, public MHO_ChannelQuantity
{
    public:
        MHO_ManualChannelPhaseCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata)
            : MHO_OperatorBuilder(toolbox, fdata){};

        MHO_ManualChannelPhaseCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                                                MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore), MHO_ChannelQuantity(){};

        virtual ~MHO_ManualChannelPhaseCorrectionBuilder(){};

        /**
         * @brief Constructs and initializes a MHO_ManualChannelPhaseCorrection operator instance
         *
         * @return bool indicating success/failure of construction
         */
        virtual bool Build() override;

    private:
        /**
         * @brief Parses a polarization string from a given name.
         *
         * @param name Input name to parse.
         * @return Polarization string ('X', 'Y', 'R', 'L' or '?').
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

#endif /*! end of include guard: MHO_ManualChannelPhaseCorrectionBuilderBuilder_HH__ */
