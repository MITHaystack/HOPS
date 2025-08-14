#ifndef MHO_BasicFringeFitter_HH__
#define MHO_BasicFringeFitter_HH__

#include "MHO_ContainerDefinitions.hh"
#include "MHO_FringeFitter.hh"

//initialization
#include "MHO_OperatorBuilderManager.hh"
#include "MHO_ParameterConfigurator.hh"
#include "MHO_ParameterManager.hh"

#include "MHO_MixedSidebandNormFX.hh"
#include "MHO_NormFX.hh"
#include "MHO_SingleSidebandNormFX.hh"

#include "MHO_InterpolateFringePeak.hh"
#include "MHO_MBDelaySearch.hh"

namespace hops
{

/*!
 *@file MHO_BasicFringeFitter.hh
 *@class MHO_BasicFringeFitter
 *@author J. Barrettj - barrettj@mit.edu
 *@date Wed Sep 20 15:37:46 2023 -0400
 *@brief basic single-baseline single-polproduct (or sum) fringe fitter, no bells or whistles
 * basic run scheme: configure, init, then: while(!IsFinished() ){ pre-run, run, post-run }
 */

/**
 * @brief Class MHO_BasicFringeFitter
 */
class MHO_BasicFringeFitter: public MHO_FringeFitter
{

    public:
        MHO_BasicFringeFitter(MHO_FringeData* data);
        virtual ~MHO_BasicFringeFitter();

        /**
         * @brief Configures fringe data and initializes operator build manager.
         * @note This is a virtual function.
         */
        virtual void Configure() override;

        /**
         * @brief Initializes fringe search operators and loads necessary data.
         * @note This is a virtual function.
         */
        virtual void Initialize() override;

        /**
         * @brief Executes user-specified scripts before running fringe fitting.
         * @note This is a virtual function.
         */
        virtual void PreRun() override;

        /**
         * @brief Runs fringe fitting algorithm if this pass is not finished and not skipped.
         *
         * @return void
         * @note This is a virtual function.
         */
        virtual void Run() override;

        /**
         * @brief Executes user-specified scripts after fringe fitting if not skipped.
         * @note This is a virtual function.
         */
        virtual void PostRun() override;

        /**
         * @brief Finalizes fringe fitting process by plotting data and executing final operators.
         * @note This is a virtual function.
         */
        virtual void Finalize() override;

        /**
         * @brief Checks if the fringe fitting process is finished.
         *
         * @return True if finished, false otherwise.
         * @note This is a virtual function.
         */
        virtual bool IsFinished() override;

        /**
         * @brief Accepts and invokes a visitor to visit this object.
         *
         * @param visitor MHO_FringeFitterVisitor used to visit this object
         * @note This is a virtual function.
         */
        virtual void Accept(MHO_FringeFitterVisitor* visitor) override { visitor->Visit(this); };

    protected:
        //main work functions, operators and works space for basic fringe search function

        /**
         * @brief Performs coarse search in delay/delay-rate space for fringe fitting.
         * @param set_windows Flag to set windows for SBD/MBD/DR searches.
         */
        void coarse_fringe_search(bool set_windows = true);

        /**
         * @brief Performs fine interpolation step to search for peak over a 5x5x5 grid around the peak.
         */
        void interpolate_peak();

        /**
         * @brief Checks if visibility data contains mixed sideband channels (USB and LSB).
         * we switch the MHO_NormFX* object depending on the type of freq setup (single or mixed sideband)
         * @param vis Input visibility_type pointer
         * @return True if mixed sidebands are present, false otherwise
         */
        bool ContainsMixedSideband(visibility_type* vis);

        //visibility/weight caching mechanism
        //to allow for user-determined outside-loop iteration (prerun, run, postrun)
        bool fEnableCaching;
        virtual void Cache() override;
        virtual void Refresh() override;

        //ptr to the operator to transform vis from frequency to single-band delay space
        //(we switch depending on the type of freq setup)
        MHO_NormFX* fNormFXOp;

        //actually working operators
        MHO_MixedSidebandNormFX fMSBNormFXOp;  //used when there is mixed LSB or USB or double-sideband data
        MHO_SingleSidebandNormFX fSSBNormFXOp; //used when there is only LSB or USB data

        MHO_MBDelaySearch* fMBDSearch;
        MHO_InterpolateFringePeak fPeakInterpolator;
        visibility_type* vis_data;
        weight_type* wt_data;
        visibility_type* sbd_data;

        //ovex info
        mho_json fVexInfo;
};

} // namespace hops

#endif /*! end of include guard: MHO_BasicFringeFitter_HH__ */
