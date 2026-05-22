#ifndef MHO_DiFXOvexPatcher_HH__
#define MHO_DiFXOvexPatcher_HH__

#include <map>
#include <string>

#include "MHO_DiFXZoomBandRebuilder.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_StationCodeMap.hh"

namespace hops
{

/*!
 *@file  MHO_DiFXOvexPatcher.hh
 *@class MHO_DiFXOvexPatcher
 *@author  J. Barrett - barrettj@mit.edu
 *@date Fri May 22 03:56:07 PM EDT 2026
 *@brief takes care of all the wierd structural fix-ups that turn a freshly-parsed VEX json into
 * the special OVEX flavor the HOPS3 ovex parser will accept. Needed to make
 * fourfit3 happy reading the root.json produced by difx2hops -k. Does the following:
 *   - stamp $EXPER.exper_num and target_correlator = "difx"
 *   - clear $DAS and station $DAS links (HOPS3 ovex can't parse them)
 *   - assign mk4_site_ID for every $SITE and build the DiFX-uppercase ->
 *     VEX-case station code / name maps consumed downstream
 *   - wrap $ANTENNA.axis_offset in the {axis_type, offset} struct
 *   - link $STATION entries to their $CLOCK keyword
 *   - synthesize $TRACKS from datastream quantBits and relink $MODE.$TRACKS
 *   - link $GLOBAL.$EOP to the first $EOP (or a dummy)
 *   - delegate zoom-band $FREQ/$BBC/$IF synthesis to MHO_DiFXZoomBandRebuilder
 *   - rewrite chan_def.phase_cal_id to match the station's $PHASE_CAL_DETECT
 *     (if not then fourfit3 do_phase_cal_detect.c aborts)
 *   - stamp the boilerplate $EVEX/$CORR_INIT/$LOG/$PBS_INIT junk that HOPS3 expects
 *
 */
class MHO_DiFXOvexPatcher
{
    public:
        MHO_DiFXOvexPatcher(): fInput(nullptr), fStationCodeMap(nullptr), fZoomBandRebuilder(nullptr), fExperNum(0) {}

        void SetDiFXInputData(const mho_json* input) { fInput = input; }

        void SetStationCodeMap(MHO_StationCodeMap* m) { fStationCodeMap = m; }

        void SetExperimentNumber(int n) { fExperNum = n; }

        void SetZoomBandRebuilder(MHO_DiFXZoomBandRebuilder* zb) { fZoomBandRebuilder = zb; }

        //apply every OVEX patch in place. Caller must have already trimmed
        //vex_root to a single scan + source (see MHO_DiFXScanProcessor::CreateRootFileObject).
        void Patch(mho_json& vex_root, const std::string& mode_name);

        //populated by Patch() from $SITE iteration; consumed by MHO_DiFXScanProcessor
        //and MHO_DiFXBaselineProcessor...we need these for later
        const std::map< std::string, std::string >& GetDiFX2VexStationCodes() const { return fDiFX2VexStationCodes; }

        const std::map< std::string, std::string >& GetDiFX2VexStationNames() const { return fDiFX2VexStationNames; }

        //d2m4-style fourfit reference time for a scan: midpoint of latest start /
        //earliest stop across stations, truncated to integer second, returned as a
        //vex-format timestamp. Pure function -- no patcher state required.
        static std::string ComputeFourfitReftime(const mho_json& scan_obj);

    private:
        //rewrite chan_def.phase_cal_id entries so they reference a pcal_id actually
        //defined in the station's $PHASE_CAL_DETECT block (otherwise fourfit3 aborts).
        void NormalizePhaseCalIds(mho_json& vex_root, const std::string& mode_name);

        const mho_json* fInput;
        MHO_StationCodeMap* fStationCodeMap;
        MHO_DiFXZoomBandRebuilder* fZoomBandRebuilder;
        int fExperNum;

        std::map< std::string, std::string > fDiFX2VexStationCodes;
        std::map< std::string, std::string > fDiFX2VexStationNames;
};

} // namespace hops

#endif /*! end of include guard: MHO_DiFXOvexPatcher */
