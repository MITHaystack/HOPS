#ifndef MHO_FringePass_HH__
#define MHO_FringePass_HH__

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "MHO_Profiler.hh"

#include "MHO_FringeControlInitialization.hh"
#include "MHO_FringeData.hh"
#include "MHO_FringeDataInitializer.hh"
#include "MHO_FringeFitter.hh"
#include "MHO_FringeFitterFactory.hh"
#include "MHO_FringePlotVisitor.hh"
#include "MHO_ParameterStore.hh"

namespace hops
{

/*!
 *@file  MHO_FringePass.hh
 *@class MHO_FringePass
 *@author J. Barrett - barrettj@mit.edu
 *@date
 *@brief Encapsulates a single-baseline, single-pol-product fringe-fitting run.
 *
 * MHO_FringePass owns an MHO_FringeData and drives the full per-pass
 * lifecycle: data initialisation, control-file evaluation, fringe-fitter
 * construction, run loop, and visitor dispatch.  It is the reusable core
 * shared between fourfit4 (command-line driver) and the Python library API.
 *
 * Typical usage:
 * @code
 *   MHO_FringePass pass;
 *   pass.CopyCommandLineParams(cmdline_params);
 *   pass.SetScanDirectory(spec["input_directory"]);
 *   pass.SetBaseline(spec["baseline"]);
 *   pass.SetPolProduct(spec["polprod"]);
 *   pass.SetFrequencyGroup(spec["frequency_group"]);
 *   pass.SetScanName(spec["scan"]);
 *   pass.SetBuildTimestamp(HOPS_BUILD_TIMESTAMP);
 *
 *   if (!pass.Initialize()) continue;
 *   if (!pass.Configure())  continue;
 *   pass.Run(plugin_visitors, output_visitors, plot_visitors);
 *   if (pass.IsSkipped())   continue;
 * @endcode
 *
 * @note MHO_FringePass does not depend on pybind11.  To support Python
 * control files (.py extension), inject an evaluator via
 * SetPythonControlEvaluator() before calling Configure().  fourfit4.cc does
 * this via MHO_PyControlEvaluator::Evaluate when built with USE_PYBIND11.
 */

class MHO_FringePass
{
    public:
        /**
         * @brief Signature of a Python control-file evaluator, matching
         * MHO_PyControlEvaluator::Evaluate.  Injected from outside so that
         * MHO_Fringe does not link against pybind11.
         */
        using ControlEvaluatorFn =
            std::function< bool(MHO_ParameterStore*, const mho_json&, mho_json&) >;

        MHO_FringePass();
        virtual ~MHO_FringePass() = default;

        // -----------------------------------------------------------------------
        // Input setters
        // Call these before Initialize().
        // -----------------------------------------------------------------------

        /**
         * @brief Bulk-copy a command-line parameter store.  Populates the
         * /cmdline/* keys consumed by downstream helpers.  Call this first,
         * then use the per-pass setters below to supply pass-specific fields.
         */
        void CopyCommandLineParams(const MHO_ParameterStore& cmdline_params);

        void SetScanDirectory(const std::string& dir);
        void SetBaseline(const std::string& baseline);
        void SetPolProduct(const std::string& polprod);
        void SetFrequencyGroup(const std::string& fgroup);
        void SetScanName(const std::string& scan);
        void SetRootFile(const std::string& root_file);
        void SetBuildTimestamp(const std::string& ts);

        /**
         * @brief Override the control-file path.  If not set, the path from
         * /cmdline/control_file (populated by CopyCommandLineParams) is used.
         */
        void SetControlFile(const std::string& path);

        /**
         * @brief Inject a Python control evaluator so that MHO_Fringe itself
         * does not need to link against pybind11.  Must be called before
         * Configure() if .py control files are to be supported.
         */
        void SetPythonControlEvaluator(ControlEvaluatorFn fn);

        // -----------------------------------------------------------------------
        // Lifecycle
        // -----------------------------------------------------------------------

        /**
         * @brief Open the scan directory, load vex data, and populate the
         * parameter store.
         * @return false if the scan directory or requested baseline is invalid.
         */
        bool Initialize();

        /**
         * @brief Evaluate the control file and apply the resulting statements.
         * Detects .py vs DSL extension automatically.
         * @return false on parse/evaluation failure or if the pass was marked
         * skipped by a 'skip' control statement.
         */
        bool Configure();

        /**
         * @brief Construct the fringe fitter, register plugin visitors, run the
         * fit loop, finalize, and dispatch output/plot visitors.
         *
         * @param plugin_visitors  Visitors registered on the fitter *before*
         *                         Configure() - typically the Python/Julia plugin
         *                         operators.  Accepted before ffit->Configure().
         * @param output_visitors  File-output visitors accepted after Finalize().
         *                         Pass an empty vector to suppress file output.
         * @param plot_visitors    Plot visitors accepted after output visitors.
         * @return false only on hard errors (fitter construction failure); a
         * skipped pass returns true - check IsSkipped() afterwards.
         */
        bool Run(const std::vector< MHO_FringeFitterVisitor* >& plugin_visitors = {},
                 const std::vector< MHO_FringeFitterVisitor* >& output_visitors = {},
                 const std::vector< MHO_FringePlotVisitor* >&   plot_visitors   = {});

        // -----------------------------------------------------------------------
        // Status and results
        // -----------------------------------------------------------------------

        bool IsSkipped();

        /**
         * @brief Collect profiler events and store them at /profile/events in
         * the pass parameter store.  Call after Run() completes.
         */
        void FlushProfileEvents();

        MHO_FringeData* GetFringeData() { return &fData; }

    private:
        MHO_FringeData fData;
        std::unique_ptr< MHO_FringeFitterFactory > fFactory; // owns the fitter

        std::string fControlFileOverride; // empty -> use /cmdline/control_file
        ControlEvaluatorFn fPythonEvaluator;
};

} // namespace hops

#endif /*! end of include guard: MHO_FringePass_HH__ */
