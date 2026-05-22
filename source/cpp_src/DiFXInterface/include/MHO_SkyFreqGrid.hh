#ifndef MHO_SkyFreqGrid_HH__
#define MHO_SkyFreqGrid_HH__

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <vector>

namespace hops
{

/*!
 *@file  MHO_SkyFreqGrid.hh
 *@class MHO_SkyFreqGrid
 *@author  J. Barrett - barrettj@mit.edu
 *@date Fri May 22 03:56:07 PM EDT 2026
 *@brief Creates a sorted, tolerance-deduplicated list of sky frequencies (MHz) with
 * indexed lookup. Used by both MHO_DiFXBaselineProcessor (for mark4 t101 chan_id chidx) 
 * and MHO_DiFXChannelNameConstructor (for chan_def.channel_name chidx) so the two 
 * stay in inline with each other across all stations in a scan.
 */
class MHO_SkyFreqGrid
{
    public:
        static constexpr double DEFAULT_TOL_MHZ = 0.001;

        MHO_SkyFreqGrid(): fTol(DEFAULT_TOL_MHZ) {}

        explicit MHO_SkyFreqGrid(std::vector< double > grid_MHz, double tol = DEFAULT_TOL_MHZ): fGrid(std::move(grid_MHz)), fTol(tol)
        {
            std::sort(fGrid.begin(), fGrid.end());
        }

        //queue a sky_freq for inclusion; takes effect after Finalize()
        void Add(double sky_freq_MHz) { fPending.push_back(sky_freq_MHz); }

        //sort pending entries and merge into the canonical grid, deduplicating within fTol
        void Finalize()
        {
            std::sort(fPending.begin(), fPending.end());
            fGrid.clear();
            for(double f : fPending)
            {
                bool dup = false;
                for(double g : fGrid)
                {
                    if(std::fabs(f - g) < fTol)
                    {
                        dup = true;
                        break;
                    }
                }
                if(!dup)
                {
                    fGrid.push_back(f);
                }
            }
            std::sort(fGrid.begin(), fGrid.end());
            fPending.clear();
        }

        void SetTolerance(double tol_MHz) { fTol = tol_MHz; }

        double GetTolerance() const { return fTol; }

        bool Empty() const { return fGrid.empty(); }

        std::size_t Size() const { return fGrid.size(); }

        const std::vector< double >& Frequencies() const { return fGrid; }

        //returns true if some grid entry is within tolerance of sky_freq_MHz;
        //out_index holds that entry's position when found.
        bool FindIndex(double sky_freq_MHz, std::size_t& out_index) const
        {
            for(std::size_t i = 0; i < fGrid.size(); i++)
            {
                if(std::fabs(sky_freq_MHz - fGrid[i]) < fTol)
                {
                    out_index = i;
                    return true;
                }
            }
            return false;
        }

        bool Contains(double sky_freq_MHz) const
        {
            std::size_t idx;
            return FindIndex(sky_freq_MHz, idx);
        }

    private:
        std::vector< double > fGrid;
        std::vector< double > fPending;
        double fTol;
};

} // namespace hops

#endif /*! end of include guard: MHO_SkyFreqGrid */
