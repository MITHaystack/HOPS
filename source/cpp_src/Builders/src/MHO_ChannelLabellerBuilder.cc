#include "MHO_ChannelLabellerBuilder.hh"
#include "MHO_ChannelLabeller.hh"
#include "MHO_ContainerDefinitions.hh"

#include "MHO_Meta.hh"
#include "MHO_ControlUtilities.hh"

#include <cstdlib>

namespace hops
{


bool
MHO_ChannelLabellerBuilder::Build()
{
    MHO_ChannelLabeller<visibility_type>* op = new MHO_ChannelLabeller<visibility_type>();

    //assume attributes are ok for now - TODO add checks!
    std::string op_name = fAttributes["name"].get<std::string>();
    std::string channel_name_str = fAttributes["channel_names"].get<std::string>();
    std::vector<double> chan_freqs = fAttributes["channel_frequencies"].get< std::vector<double> >();
    std::vector< std::string > chan_names = SplitChannelLabels(channel_name_str);

    if( chan_freqs.size() == chan_names.size() )
    {
        auto label2freq = zip_into_map(chan_names, chan_freqs); //name -> freq
        op->SetChannelLabelToFrequencyMap(label2freq);
        //return a default channel labelling operator

        bool replace_duplicates = true;
        MHO_OperatorToolbox::GetInstance().AddOperator(op,op_name,replace_duplicates);

        return true;
    }
    else
    {
        delete op;
        op = nullptr;
        msg_error("builders", "cannot label channels with an unequal number of elements " <<
                  "(labels, freqs) = (" << chan_names.size() << ", " << chan_freqs.size() << ")"
                  << eom );
        return false;
    }
}



}//end namespace
