#ifndef MHO_ManualChannelDelayCorrectionBuilder_HH__
#define MHO_ManualChannelDelayCorrectionBuilder_HH__

#include "MHO_ChannelUtilities.hh"
#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_ManualChannelDelayCorrectionBuilder.hh
 *@class MHO_ManualChannelDelayCorrectionBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date Wed May 31 17:11:03 2023 -0400
 *@brief build a manual per-channel pc_delay operator
 */

/**
 * @brief Class MHO_ManualChannelDelayCorrectionBuilder
 */
class MHO_ManualChannelDelayCorrectionBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_ManualChannelDelayCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata)
            : MHO_OperatorBuilder(toolbox, fdata){};

        MHO_ManualChannelDelayCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                                                MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_ManualChannelDelayCorrectionBuilder(){};

        /**
         * @brief Constructs and adds a new MHO_ManualChannelDelayCorrection operator to toolbox
         *
         * @return bool indicating success
         */
        virtual bool Build() override;

    private:
        /**
         * @brief Parses a polarization string from a given name.
         *
         * @param name Input name string to parse.
         * @return Polarization string ('X', 'Y', 'R', 'L' or '?').
         */
        std::string ParsePolFromName(const std::string& name);
};

} // namespace hops

#endif /*! end of include guard: MHO_ManualChannelDelayCorrectionBuilder_HH__ */
