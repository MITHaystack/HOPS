#ifndef MHO_ChannelQuantity_HH__
#define MHO_ChannelQuantity_HH__

/*!
*@file MHO_ChannelQuantity.hh
*@class MHO_ChannelQuantity
*@date
*@brief class for storing and mapping per-channel control quantities
*@author J. Barrett - barrettj@mit.edu
*/

#include <map>
#include <string>
#include <vector>

namespace hops
{

class MHO_ChannelQuantity
{
    public:
        MHO_ChannelQuantity(){};
        ~MHO_ChannelQuantity(){};

    protected:

        //if the number of channel names isn't the same as the number of values, returns and empty map
        std::map< std::string, double> MapChannelQuantities(std::string chan_names, std::vector<double>& values);

};

}

#endif /*! end of include guard: MHO_ChannelQuantity_HH__ */
