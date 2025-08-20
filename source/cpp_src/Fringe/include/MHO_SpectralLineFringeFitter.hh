#ifndef MHO_SpectralLineFringeFitter_HH__
#define MHO_SpectralLineFringeFitter_HH__

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
 *@file MHO_SpectralLineFringeFitter.hh
 *@class MHO_SpectralLineFringeFitter
 *@author J. Barrettj - barrettj@mit.edu
 *@date Wed Sep 20 15:37:46 2023 -0400
 *@brief experimental spectral line fringe fitter
 * basic run scheme: configure, init, then: while(!IsFinished() ){ pre-run, run, post-run }
 */

/**
 * @brief Class MHO_SpectralLineFringeFitter
 */
class MHO_SpectralLineFringeFitter: public MHO_FringeFitter
{

    public:
        MHO_SpectralLineFringeFitter(MHO_FringeData* data);
        virtual ~MHO_SpectralLineFringeFitter();

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
        
        //visibility/weight caching mechanism
        //to allow for user-determined outside-loop iteration (prerun, run, postrun)
        bool fEnableCaching;
        virtual void Cache() override;
        virtual void Refresh() override;

        visibility_type* vis_data;
        weight_type* wt_data;
        visibility_type* sbd_data;

        //ovex info
        mho_json fVexInfo;
};

} // namespace hops

#endif /*! end of include guard: MHO_SpectralLineFringeFitter_HH__ */
