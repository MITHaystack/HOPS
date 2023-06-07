#include "MHO_ChannelLabellerBuilder.hh"
#include "MHO_ChannelLabeller.hh"
#include "MHO_ContainerDefinitions.hh"

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

#include <vector>
#include <map>
#include <cstdlib>

namespace hops
{


bool
MHO_ChannelLabellerBuilder::Build()
{

    //assume attributes are ok for now - TODO add checks!
    std::string op_name = fAttributes["name"].get<std::string>();
    std::string channel_name_str = fAttributes["channel_names"].get<std::string>();
    std::vector<double> chan_freqs = fAttributes["channel_frequencies"].get< std::vector<double> >();
    std::vector< std::string > chan_names;

    //split the channel specifier
    if(channel_name_str.find(",") != std::string::npos){chan_names= SplitString(channel_name_str, std::string(",") );}
    else{chan_names= SplitString(channel_name_str);} //split every char}

    if( chan_freqs.size() == chan_names.size() )
    {
        //create the map, channel name -> freq
        auto label2freq = zip_into_map(chan_names, chan_freqs); 
        
        //retrieve the arguments to operate on from the container store
        std::string vis_name = "vis";
        std::string weight_name = "weight";
        auto visID = fContainerStore->GetObjectUUID(vis_name);
        auto wtID = fContainerStore->GetObjectUUID(weight_name);
        
        if( visID.is_empty() || wtID.is_empty() )
        {
            msg_fatal("builders", "cannot construct MHO_ChannelLabeller without visibility or weight data." << eom);
            return false;
        }
        
        visibility_type* vis_data = fContainerStore->GetObject<visibility_type>(visID);
        weight_type* wt_data = fContainerStore->GetObject<weight_type>(wtID);

        MHO_ChannelLabeller<visibility_type>* vis_op = new MHO_ChannelLabeller<visibility_type>();
        MHO_ChannelLabeller<weight_type>* wt_op = new MHO_ChannelLabeller<weight_type>();

        vis_op->SetChannelLabelToFrequencyMap(label2freq);
        wt_op->SetChannelLabelToFrequencyMap(label2freq);

        //now set the arguments 
        vis_op->SetArgs(vis_data);
        wt_op->SetArgs(wt_data);

        bool replace_duplicates = true;
        fOperatorToolbox->AddOperator(vis_op,op_name + ":" + vis_name , replace_duplicates);
        fOperatorToolbox->AddOperator(wt_op,op_name + ":" + weight_name , replace_duplicates);

        return true;
    }
    else
    {
        msg_error("builders", "cannot label channels with an unequal number of elements " <<
                  "(labels, freqs) = (" << chan_names.size() << ", " << chan_freqs.size() << ")"
                  << eom );
        return false;
    }
}



}//end namespace
