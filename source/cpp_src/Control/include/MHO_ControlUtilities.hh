#ifndef MHO_ControlUtilities_HH__
#define MHO_ControlUtilities_HH__

#include <vector>
#include <string>
#include "MHO_Tokenizer.hh"


namespace hops
{

//fuction which splits the commonly used merged-label channel specification from
//a control file into a vector of channel labels
//for example "abc" -> ["a", "b", "c"]
//or "a,b,c" -> ["a","b","c"] note: the comma syntax is not supported in the original format
std::vector< std::string > SplitChannelLabels(const std::string& channel_names);


}


#endif /* end of include guard: MHO_ControlUtilities_HH__ */
