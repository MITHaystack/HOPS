#include "MHO_ChannelLabellerBuilder.hh"
#include "MHO_ChannelLabeller.hh"

#include <cstdlib>

namespace hops
{


std::pair<std::string, MHO_Operator*>
MHO_ChannelLabellerBuilder::Build()
{
    MHO_ChannelLabeller* op = new MHO_ChannelLabeller();

    //assume attributes are ok
    //TODO add checks!
    std::string op_name = fAttributes["name"].get<std::string>();
    std::string channel_name_str = fAttributes["channel_names"].get<std::string>();
    std::vector<double> chan_freqs = fAttributes["channel_names"].get< std::vector<double> >();
    std::vector< std::string > chan_names = ParseChannelLabels(channel_name_str);

    if( chan_freqs.size() == chan_names.size()
    {
        std::map< std::string, double> label2freq;
        for(std::size_t i=0; i<chan_freqs.size(); i++)
        {
            label2freq[ chan_names[i] ] = chan_freqs[i];
        }
        op->SetChannelLabelToFrequencyMap(label2freq);
        //return a default channel labelling operator 
        return std::make_pair<std::string, MHO_Operator*>("channel_labeller", op);
    }
    else 
    {
        delete op;
        op = nullptr;
        msg_error("builders", "cannot label channels with an unequal number of elements (labels, freqs) = (" <<
                << chan_names.size() << ", " << chan_freqs.size() << ")" << eom );
            )
        return std::make_pair<std::string, MHO_Operator*>("channel_labeller", op);
    }

    // {
    //     "name": "chan_ids",
    //     "statement_type": "parameter",
    //     "type" : "compound",
    //     "parameters":
    //     {
    //         "channel_names": {"type": "string"},
    //         "channel_frequencies": {"type": "list_real"}
    //     },
    //     "fields": 
    //     [
    //         "channel_names", 
    //         "channel_frequencies"
    //     ]
    // }


    mho_json chan_names = fAttributes["chan"]
    std::vector< std::string > labels = ParseChannelLabels(const std::string& channel_names);


}

std::vector< std::string > 
MHO_ChannelLabellerBuilder::ParseChannelLabels(const std::string& channel_names)
{
    std::vector< std::string > split_channel_names;
    std::string comma = ",";
    if( channel_names.find(comma) != std::string::npos )
    {
        //assume single char labels only, use every character as a label
        for(std::size_t i=0; i<channel_names.size(); i++)
        {
            std::string chn = "" + channel_names[i]
            split_channel_names.push_back(chn);
        }
    }
    else //assume we are ussing comma delimiter for possibly multi-char labels
    {
        fTokenizer.SetDelimiter(comma);
        fTokenizer.SetIncludeEmptyTokensFalse();
        fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
        fTokenizer.SetString(&channel_names);
        fTokenizer.GetTokens(&split_channel_names);
    }

    return split_channel_names;
}



}//end namespace
