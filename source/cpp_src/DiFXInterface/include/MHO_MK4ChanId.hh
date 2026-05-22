#ifndef MHO_MK4ChanId_HH__
#define MHO_MK4ChanId_HH__

#include <iomanip>
#include <sstream>
#include <string>

namespace hops
{

/*!
 *@file  MHO_MK4ChanId.hh
 *@author  J. Barrett - barrettj@mit.edu
 *@date Fri May 22 03:56:07 PM EDT 2026
 *@brief Single class for canonical mark4 chan_id / chan_def.channel_name formatter:
 * uses: band + 2-digit zero-padded chidx + sideband + pol difx2mark4 convention
 * Should be used everywhere we crate or compare chan_ids so the t101 ref/rem chan_id,
 * so that the ovex chan_def.channel_name, and the zoom-band synthesized chan_defs
 * all share one definition.
 *
 * Note: the chidx field is minimum-width 2 (zero-padded for values < 10) but
 * not capped. Values >= 100 print as their full decimal representation, mark4
 * fourfit3 can only consume <= 64 chan_defs anyways, but the
 * fourfit4 visibility path has no such limit.
 */
class MHO_MK4ChanId
{
    public:
        static std::string Make(const std::string& band, int chidx, const std::string& sideband, char pol)
        {
            std::ostringstream ss;
            ss << band << std::setfill('0') << std::setw(2) << chidx << sideband << pol;
            return ss.str();
        }

        static std::string Make(const std::string& band, int chidx, const std::string& sideband, const std::string& pol)
        {
            return Make(band, chidx, sideband, pol.empty() ? '-' : pol[0]);
        }
};

} // namespace hops

#endif /*! end of include guard: MHO_MK4ChanId */
