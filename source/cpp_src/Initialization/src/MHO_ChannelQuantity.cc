#include "MHO_ChannelQuantity.hh"
#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

//if the number of channel names isn't the same as the number of values, returns and empty map
std::map< std::string, double > MHO_ChannelQuantity::MapChannelQuantities(std::string channel_name_str,
                                                                          std::vector< double >& values)
{
    std::map< std::string, double > chan_quantity_map;
    std::vector< std::string > chan_names;
    //split the channel specifier
    if(channel_name_str.find(",") != std::string::npos)
    {
        chan_names = SplitString(channel_name_str, std::string(","));
    }
    else
    {
        chan_names = SplitString(channel_name_str);
    } //split every char

    if(values.size() != chan_names.size())
    {
        msg_error("initialization", "bad control statement data, the number of channels specified ("
                                        << chan_names.size() << ") did not match the number of quantities passed ("
                                        << values.size() << ")" << eom);
        //in this case, the resulting map will be truncated to
        //the shortest of either chan_names or values, but mis-matched items
        //cannot be fixed
    }

    //create the map, channel name -> freq,
    chan_quantity_map = zip_into_map(chan_names, values);

    return chan_quantity_map;
}

} // namespace hops
