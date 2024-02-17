#include "MHO_ChannelQuantity.hh"
#include "MHO_Tokenizer.hh"
#include "MHO_Meta.hh"

namespace hops
{

//if the number of channel names isn't the same as the number of values, returns and empty map
std::map< std::string, double>
MHO_ChannelQuantity::MapChannelQuantities(std::string channel_name_str, std::vector<double>& values)
{
    std::map< std::string, double> chan_quantity_map;
    std::vector< std::string > chan_names;
    //split the channel specifier
    if(channel_name_str.find(",") != std::string::npos){chan_names= SplitString(channel_name_str, std::string(",") );}
    else{chan_names = SplitString(channel_name_str);} //split every char

    if( values.size() == chan_names.size() )
    {
        //create the map, channel name -> freq
        chan_quantity_map = zip_into_map(chan_names, values);
    }
    else
    {
        msg_fatal("initialization", "bad control statement data, the number of channels specified ("<<chan_names.size()
            <<") did not match the number of quantities passed ("<<values.size() <<")" << eom );
        std::exit(1);
    }

    return chan_quantity_map;
}

}
