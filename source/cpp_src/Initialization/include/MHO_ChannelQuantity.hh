#ifndef MHO_ChannelQuantity_HH__
#define MHO_ChannelQuantity_HH__

#include <map>
#include <string>
#include <vector>

namespace hops
{

/*!
 *@file MHO_ChannelQuantity.hh
 *@class MHO_ChannelQuantity
 *@date Thu Jun 8 13:56:30 2023 -0400
 *@brief class for storing and mapping per-channel control quantities (a typical task for fourfit control files)
 *@author J. Barrett - barrettj@mit.edu
 */

class MHO_ChannelQuantity
{
    public:
        MHO_ChannelQuantity(){};
        ~MHO_ChannelQuantity(){};

    protected:
        /**
         * @brief Maps channel names to corresponding values and returns a map.
         * if the number of channel names isn't the same as the number of values, this returns an empty map
         *
         * @param chan_names Input string containing comma-separated (or concatenated 1-character, with no space) channel names.
         * @param values Reference to input vector of double values.
         * @return Map with channel names as keys and corresponding values.
         */
        std::map< std::string, double > MapChannelQuantities(std::string chan_names, std::vector< double >& values);
};

} // namespace hops

#endif /*! end of include guard: MHO_ChannelQuantity_HH__ */
