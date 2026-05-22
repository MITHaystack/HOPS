#ifndef MHO_DiFXZoomBandRebuilder_HH__
#define MHO_DiFXZoomBandRebuilder_HH__

#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "MHO_JSONHeaderWrapper.hh"

namespace hops
{

/*!
 *@file  MHO_DiFXZoomBandRebuilder.hh
 *@class MHO_DiFXZoomBandRebuilder
 *@author  J. Barrett - barrettj@mit.edu
 *@date Fri May 22 03:56:07 PM EDT 2026
 *@brief Creates new $FREQ/$BBC/$IF VEX sections for stations that DiFX correlated
 * via zoom bands (zoomFreqId/zoomBandFreqId rather than the recorded-channel
 * tables). The source VEX describes the *recorded* channels, but the
 * visibilities DiFX emits are from the zoom freqs. Without this rebuild,
 * chan_def lookups during channel naming would fail to match the visibility
 * data (not a big deal for fourfit4, but it is for fourfit3). 
 * One synthetic freq/bbc/if triple is created per station per scan and
 * the station's $MODE links are repointed to the new tables.
 *
 * Used only when HasZoomBands() && WantZoomChannels() -- i.e. the DiFX input
 * declares zoom freqs and the user hasn't explicitly asked for a different (native) bandwidth via -w.
 */
class MHO_DiFXZoomBandRebuilder
{
    public:
        MHO_DiFXZoomBandRebuilder(): fInput(nullptr), fSelectByBandwidth(false), fOnlyBandwidth(0.0) {}

        void SetDiFXInputData(const mho_json* input) { fInput = input; }

        void SetSelectionByBandwidth(double bw)
        {
            fSelectByBandwidth = true;
            fOnlyBandwidth = bw;
        }

        void ClearSelectionByBandwidth() { fSelectByBandwidth = false; }

        //frequency-band labels (e.g. {"X", 8000, 9000}); used as chan_def.band_id link values
        void SetFrequencyBands(const std::vector< std::tuple< std::string, double, double > >& fbands) { fFreqBands = fbands; }

        //DiFX uppercase 2-char station code -> VEX (mixed-case) 2-char station code
        void SetDiFX2VexStationCodes(const std::map< std::string, std::string >& m) { fDiFX2VexStationCodes = m; }

        //true if any datastream has nZoomFreq > 0
        bool HasZoomBands() const;

        //true if no bandwidth filter is active, or the active filter matches the zoom bandwidth.
        //If false, the rebuild should be skipped (native channels will be used).
        bool WantZoomChannels() const;

        //union of all global zoom-freq indices across every datastream. Use this to set the
        //visibility processor's zoom selection so native records are discarded.
        std::set< int > CollectZoomFreqIndices() const;

        //rewrite vex_root["$FREQ"], $BBC, $IF and the station links under $MODE[mode_name]
        //for every station whose datastream uses zoom bands.
        void RebuildFreqSections(mho_json& vex_root, const std::string& mode_name);

    private:
        const mho_json* fInput;

        bool fSelectByBandwidth;
        double fOnlyBandwidth;

        std::vector< std::tuple< std::string, double, double > > fFreqBands;
        std::map< std::string, std::string > fDiFX2VexStationCodes;
};

} // namespace hops

#endif /*! end of include guard: MHO_DiFXZoomBandRebuilder */
