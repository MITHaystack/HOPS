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
std::vector< std::string > 
SplitChannelLabels(const std::string& channel_names)
{
    std::vector< std::string > split_channel_names;
    std::string comma = ",";

    if( channel_names.find(comma) == std::string::npos )
    {
        //assume single char labels only, use every character as a label
        for(std::size_t i=0; i<channel_names.size(); i++)
        {
            std::string chn = "";
            chn += channel_names[i];
            split_channel_names.push_back(chn);
        }
    }
    else //assume we are using a comma delimiter for possibly multi-char labels
    {
        MHO_Tokenizer tokenizer;
        tokenizer.SetDelimiter(comma);
        tokenizer.SetIncludeEmptyTokensFalse();
        tokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
        tokenizer.SetString(&channel_names);
        tokenizer.GetTokens(&split_channel_names);
    }
    return split_channel_names;
};


}


#endif /* end of include guard: MHO_ControlUtilities_HH__ */
