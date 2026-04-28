#ifndef MHO_ChannelUtilities_HH__
#define MHO_ChannelUtilities_HH__

#include <map>
#include <string>
#include <vector>

namespace hops
{

/*!
 *@file MHO_ChannelUtilities.hh
 *@date Thu Jun 8 13:56:30 2023 -0400
 *@brief Utility functions for mapping per-channel control quantities
 *@author J. Barrett - barrettj@mit.edu
 */

/// Maps channel names to corresponding values and returns a map.
/// If the number of channel names isn't the same as the number of values, an error is logged
/// and the map is truncated to the shortest of the two.
///
/// @param chan_names Input string containing comma-separated (or concatenated 1-character, with no space) channel names.
/// @param values Reference to input vector of double values.
/// @return Map with channel names as keys and corresponding values.
std::map< std::string, double > MapChannelQuantities(std::string chan_names, std::vector< double >& values);

} // namespace hops

#endif /*! end of include guard: MHO_ChannelUtilities_HH__ */
