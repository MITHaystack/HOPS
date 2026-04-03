#ifndef MHO_AdhocPhaseCorrectionBuilder_HH__
#define MHO_AdhocPhaseCorrectionBuilder_HH__

#include "MHO_OperatorBuilder.hh"

namespace hops
{

/*!
 *@file MHO_AdhocPhaseCorrectionBuilder.hh
 *@class MHO_AdhocPhaseCorrectionBuilder
 *@author J. Barrett - barrettj@mit.edu
 *@date
 *@brief Builds a MHO_AdhocPhaseCorrection operator from control-file parameters.
 *
 * Triggered by an 'adhoc_phase <algorithm_type>' compound statement in the
 * control file. The algorithm_type string selects the correction mode:
 *   "sinewave"   -> AdhocPhaseMode::SINEWAVE
 *   "polynomial" -> AdhocPhaseMode::POLYNOMIAL
 *   "file"       -> AdhocPhaseMode::PHYLE
 *
 * Auxiliary scalar parameters consumed from the parameter store:
 *   /control/config/adhoc_tref    (real, seconds from scan start)
 *   /control/station/<ref_id or rem_id>/adhoc_period  (real, seconds)
 *   /control/station/<ref_id or rem_id>/adhoc_amp     (real, degrees -- converted to radians internally) (units?)
 *   /control/station/<ref_id or rem_id>/adhoc_poly    (list_real, degrees/s^n -- converted to radians/s^n) (units?)
 *
 * Per-station file parameters consumed from the parameter store (no generic parameters):
 *   /control/station/<ref_id>/adhoc_file    (string, ref-station specific override)
 *   /control/station/<rem_id>/adhoc_file    (string, rem-station specific override)
 *   /control/station/<ref_id>/adhoc_file_chans
 *   /control/station/<rem_id>/adhoc_file_chans
 */

/**
 * @brief Class MHO_AdhocPhaseCorrectionBuilder
 */
class MHO_AdhocPhaseCorrectionBuilder: public MHO_OperatorBuilder
{
    public:
        MHO_AdhocPhaseCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_FringeData* fdata)
            : MHO_OperatorBuilder(toolbox, fdata){};

        MHO_AdhocPhaseCorrectionBuilder(MHO_OperatorToolbox* toolbox, MHO_ContainerStore* cstore = nullptr,
                                        MHO_ParameterStore* pstore = nullptr)
            : MHO_OperatorBuilder(toolbox, cstore, pstore){};

        virtual ~MHO_AdhocPhaseCorrectionBuilder(){};

        /**
         * @brief Constructs and registers a MHO_AdhocPhaseCorrection operator.
         *
         * @return true on success, false if required data or parameters are missing.
         */
        virtual bool Build() override;
};

} // namespace hops

#endif /*! end of include guard: MHO_AdhocPhaseCorrectionBuilder_HH__ */
