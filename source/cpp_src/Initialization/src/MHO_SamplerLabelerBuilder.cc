#include "MHO_SamplerLabelerBuilder.hh"
#include "MHO_SamplerLabeler.hh"
#include "MHO_ContainerDefinitions.hh"

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

#include <vector>
#include <map>
#include <cstdlib>

namespace hops
{


bool
MHO_SamplerLabelerBuilder::Build()
{
    if( IsConfigurationOk() )
    {
        msg_debug("initialization", "building sampler labeling operator."<< eom);

        std::string op_name = fAttributes["name"].get<std::string>();
        std::string op_category = "labeling";

        //retrieve the arguments to operate on from the container store
        visibility_type* vis_data = fContainerStore->GetObject<visibility_type>(std::string("vis"));

        if( vis_data == nullptr)
        {
            msg_error("initialization", "cannot construct MHO_SamplerLabeler without visibility or weight data." << eom);
            return false;
        }
        
        //get the reference and remote station mk4ids
        std::string ref_id = this->fParameterStore->GetAs<std::string>("/ref_station/mk4id");
        std::string rem_id = this->fParameterStore->GetAs<std::string>("/rem_station/mk4id");
        
        //now determine the path to the channel->sampler info, if it exists at all
        std::string generic_path = "/control/station/samplers";
        std::string ref_path = "";
        std::string rem_path = "";
        if(this->fParameterStore->IsPresent(generic_path))
        {
            //generic info available (applies to all stations)
            ref_path = generic_path;
            rem_path = generic_path;
        }
        //specific ref-station info available
        if(this->fParameterStore->IsPresent(std::string("/control/station/") + ref_id + "/samplers"))
        {
            ref_path = std::string("/control/station/") + ref_id + "/samplers";
        }
        //specific rem-station info available
        if(this->fParameterStore->IsPresent(std::string("/control/station/") + rem_id + "/samplers"))
        {
            rem_path = std::string("/control/station/") + rem_id + "/samplers";
        }

        //bail out if no info available
        if(ref_pcal == "" && rem_path == "")
        {
            msg_debug("initialization", "will not build MHO_SamplerLabeler operator, as there is no sampler-info present." << eom);
            return false;
        }

        //info mapping channel labels to sampler indexes
        std::vector< std::string > ref_sampler_info;
        std::vector< std::string > rem_sampler_info;

        //next retrieve the necessary parameter values (samplers) from the parameter store
        if(ref_path != "")
        {
            this->fParameterStore->Get(ref_path, ref_sampler_info);
        }
        
        if(rem_path != "")
        {
            this->fParameterStore->Get(rem_path, rem_sampler_info);
        }
        
        MHO_SamplerLabeler<visibility_type>* op = new MHO_SamplerLabeler<visibility_type>();
        if(ref_sampler_info.size() != 0 ){op->SetRemoteStationSamplerChannelSets(ref_sampler_info);}
        if(rem_sampler_info.size() != 0 ){op->SetRemoteStationSamplerChannelSets(rem_sampler_info);}
        op->SetArgs(vis_data);
        op->SetName(op_name);
        
        fOperatorToolbox->AddOperator(op, op->GetName(), op_category);
        return true;
    }
    return false;
}


}//end namespace
