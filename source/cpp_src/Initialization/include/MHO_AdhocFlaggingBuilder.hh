#ifndef MHO_AdhocFlaggingBuilder_HH__
#define MHO_AdhocFlaggingBuilder_HH__

#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_AdhocFlaggingBuilder.hh
 *@class MHO_AdhocFlaggingBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@brief Builds an MHO_AdhocFlagging operator from control-file parameters.
 *
 * Triggered by an 'adhoc_flag_file <path>' compound statement in
 * the control file.  The statement may appear inside a station-conditional
 * block ('if station X { ... }') to set the flag file for a specific station,
 * or outside any condition to apply the same file to both stations.
 *
 * Because a baseline has two stations (reference and remote), the builder
 * maintains a single 'adhoc_flagging' operator in the toolbox and updates
 * its ref/rem file paths as each control statement is processed.  The first
 * call creates the operator; subsequent calls find the existing instance via
 * the toolbox and update whichever station file path applies.
 *
 * Parameters consumed from the compound statement:
 *   flag_file  (string)  path to the adhoc flag file for this station
 *
 * Station identity is extracted from the 'if station <id>' condition in
 * fConditions, then compared against:
 *   /ref_station/site_id  (parameter store)
 *   /rem_station/site_id  (parameter store)
 */

/**
 * @brief Class MHO_AdhocFlaggingBuilder
 */
class MHO_AdhocFlaggingBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_AdhocFlaggingBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata): MHO_OperatorBuilder(toolbox, fdata){};

        MHO_AdhocFlaggingBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                                 MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_AdhocFlaggingBuilder(){};

        /**
         * @brief Constructs (or updates) the MHO_AdhocFlagging operator and registers it.
         *
         * @return true on success, false if required data or parameters are missing.
         */
        virtual bool Build() override;

    private:
        /**
         * @brief Extracts the first station identifier from the 'if station' condition.
         *
         * @return The station code string, or "??" if no station condition is found
         *         (indicating a global / baseline-wide statement).
         */
        std::string ExtractStationIdentifier();
};

} // namespace hops

#endif /*! end of include guard: MHO_AdhocFlaggingBuilder_HH__ */
