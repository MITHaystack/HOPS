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
    if( IsConfigurationOk() )
    {
        msg_debug("builders", "building channel labelling operator."<< eom);
        //assume attributes are ok for now - TODO add checks!
        std::string op_name = fAttributes["name"].get<std::string>();
        std::string channel_name_str = fAttributes["channel_names"].get<std::string>();
        std::vector<double> chan_freqs = fAttributes["channel_frequencies"].get< std::vector<double> >();
        //construct channel <-> freq map
        auto label2freq = MapChannelQuantities(channel_name_str, chan_freqs);

        if( label2freq.size() > 0 )
        {
            //retrieve the arguments to operate on from the container store
            visibility_type* vis_data = fContainerStore->GetObject<visibility_type>(std::string("vis"));
            weight_type* wt_data = fContainerStore->GetObject<weight_type>(std::string("weight"));

            if( vis_data == nullptr || wt_data == nullptr)
            {
                msg_error("builders", "cannot construct MHO_ChannelLabeller without visibility or weight data." << eom);
                return false;
            }

            MHO_ChannelLabeller<visibility_type>* vis_op = new MHO_ChannelLabeller<visibility_type>();
            MHO_ChannelLabeller<weight_type>* wt_op = new MHO_ChannelLabeller<weight_type>();

            vis_op->SetChannelLabelToFrequencyMap(label2freq);
            wt_op->SetChannelLabelToFrequencyMap(label2freq);

            //now set the arguments
            vis_op->SetArgs(vis_data);
            wt_op->SetArgs(wt_data);

            bool replace_duplicates = true;
            #pragma message("TODO - figure out proper naming/retrieval scheme for operators")
            fOperatorToolbox->AddOperator(vis_op,op_name + ":vis", replace_duplicates);
            fOperatorToolbox->AddOperator(wt_op,op_name + ":weight", replace_duplicates);

            return true;
        }
        else
        {
            msg_error("builders", "cannot label channels with an unequal number of channels/elements." << eom );
            return false;
        }
    }
    return false;
}


}//end namespace
