#ifndef MHO_DiFXChannelNameConstructor_HH__
#define MHO_DiFXChannelNameConstructor_HH__

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_SkyFreqGrid.hh"

namespace hops
{

/*!
 *@file  MHO_DiFXChannelNameConstructor.hh
 *@class  MHO_DiFXChannelNameConstructor
 *@author  J. Barrett - barrettj@mit.edu
 *@date Tue Jul 12 16:54:42 2022 -0400
 *@brief  Name channels in a vex object, according to the d2m4 convention.
 * Needed in order to convert vex to ovex.
 */

/**
 * @brief Class MHO_DiFXChannelNameConstructor
 */
class MHO_DiFXChannelNameConstructor
{
    public:
        MHO_DiFXChannelNameConstructor();
        virtual ~MHO_DiFXChannelNameConstructor();

        /**
         * @brief Adds a frequency range for a specific band label.
         *
         * @param band_label Band label as string
         * @param freq_low Lower frequency limit in Hz
         * @param freq_high Upper frequency limit in Hz
         */
        void AddBandLabel(std::string band_label, double freq_low, double freq_high);

        /**
         * @brief Adds channel names to VEX experiment data based on scan and mode information.
         *
         * @param vex_root Reference to the root JSON object containing VEX experiment data
         */
        void AddChannelNames(mho_json& vex_root);

        /**
         * @brief Provide a precomputed global sky-frequency grid (MHz, sorted ascending,
         * deduped). When set, AddChannelNames uses this grid for chidx assignment
         * (instead of building one per-station from chan_defs) and drops any chan_def
         * whose sky_freq isn't in the grid. This is what keeps chan_def.channel_name in
         * lockstep with the chan_ids MHO_DiFXBaselineProcessor writes into mark4 t101 records.
         *
         * @param grid_MHz Sorted, deduped sky frequencies in MHz
         * @param tol Match tolerance in MHz (default 0.001)
         */
        void SetGlobalSkyFreqGrid(const std::vector< double >& grid_MHz, double tol = MHO_SkyFreqGrid::DEFAULT_TOL_MHZ)
        {
            fGlobalGrid = MHO_SkyFreqGrid(grid_MHz, tol);
            fHasGlobalGrid = true;
        }

        /**
         * @brief Setter for scan name
         * if the (o)vex file has more than one scan, we may want to specify
         * a specific one in order to construct the channel names,
         * otherwise, we will just use the first in the schedule (assumes there is not later change to channel setup)
         *
         * @param scan_id New scan ID to set
         */
        void SetScanName(std::string scan_id) { fScanID = scan_id; }

    private:
        /**
         * @brief Assigns a band label to a given sky frequency.
         *
         * @param sky_freq Input sky frequency in Hz
         * @return Band label as string
         */
        std::string BandLabelFromSkyFreq(double sky_freq);

        /**
         * @brief Class band_range
         */
        struct band_range
        {
                double fLow;
                double fHigh;
                std::string fLabel;
        };

        std::vector< band_range > fBandRangeLabels;
        std::string fScanID;

        //scan-wide canonical grid (set by SetGlobalSkyFreqGrid); used by AddChannelNames
        //when fHasGlobalGrid is true. When a per-station table has no chan_def on the
        //global grid we temporarily swap in a per-table grid (see AddChannelNames).
        MHO_SkyFreqGrid fGlobalGrid;
        bool fHasGlobalGrid; //true if SetGlobalSkyFreqGrid has been called for this scan
};

} // namespace hops

#endif /*! end of include guard: MHO_DiFXChannelNameConstructor */
