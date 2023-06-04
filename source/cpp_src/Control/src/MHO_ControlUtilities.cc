#include "MHO_ControlUtilities.hh"

namespace hops
{

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
