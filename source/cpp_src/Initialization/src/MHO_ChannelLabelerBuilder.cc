#include "MHO_ChannelLabelerBuilder.hh"
#include "MHO_ChannelLabeler.hh"
#include "MHO_ContainerDefinitions.hh"

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

#include <vector>
#include <map>
#include <cstdlib>

namespace hops
{


bool
MHO_ChannelLabelerBuilder::Build()
{
    if( IsConfigurationOk() )
    {
        msg_debug("initialization", "building channel labeling operator."<< eom);
        //assume attributes are ok for now - TODO add checks!
        std::map< std::string, double> label2freq;
        std::string op_name = "chan_ids";
        std::string op_category = "labeling";
        if(! fAttributes.empty() )
        {
            std::string channel_name_str = fAttributes["value"]["channel_names"].get<std::string>();
            std::vector<double> chan_freqs = fAttributes["value"]["channel_frequencies"].get< std::vector<double> >();
            //construct channel <-> freq map
            label2freq = MapChannelQuantities(channel_name_str, chan_freqs);
            msg_debug("initialization", "channel labeling operator was given a map of size: "<< label2freq.size() << eom );
        }
        else 
        {
            msg_debug("initialization", "default channel labeling operator being created."<< eom );
        }

        //retrieve the arguments to operate on from the container store
        visibility_type* vis_data = fContainerStore->GetObject<visibility_type>(std::string("vis"));
        weight_type* wt_data = fContainerStore->GetObject<weight_type>(std::string("weight"));

        if( vis_data == nullptr || wt_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_ChannelLabeler without visibility or weight data." << eom);
            return false;
        }

        MHO_ChannelLabeler<visibility_type>* vis_op = new MHO_ChannelLabeler<visibility_type>();
        MHO_ChannelLabeler<weight_type>* wt_op = new MHO_ChannelLabeler<weight_type>();

        //now set the arguments
        vis_op->SetArgs(vis_data);
        wt_op->SetArgs(wt_data);

        //assuming we have a valid, chan -> freq map, set it, otherwise default op will be created
        if( label2freq.size() > 0 )
        {
            vis_op->SetChannelLabelToFrequencyMap(label2freq);
            wt_op->SetChannelLabelToFrequencyMap(label2freq);
        }

        bool replace_duplicates = true; //replces the default labeler
        #pragma message("TODO - figure out proper naming/retrieval scheme for operators")
        
        vis_op->SetName(op_name + ":vis");
        wt_op->SetName(op_name + ":weight");
        
        fOperatorToolbox->AddOperator(vis_op, vis_op->GetName(), op_category, replace_duplicates);
        fOperatorToolbox->AddOperator(wt_op, wt_op->GetName(), op_category, replace_duplicates);

        return true;


    }
    return false;
}


}//end namespace
