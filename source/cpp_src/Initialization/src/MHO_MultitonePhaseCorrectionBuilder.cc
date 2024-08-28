#include "MHO_MultitonePhaseCorrectionBuilder.hh"
#include "MHO_MultitonePhaseCorrection.hh"

#include "MHO_Meta.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

bool
MHO_MultitonePhaseCorrectionBuilder::Build()
{
    if( IsConfigurationOk() )
    {
        msg_debug("initialization", "building a multitone phase correction operator."<< eom);
        //assume attributes are ok for now - TODO add checks!

        std::string op_name = fAttributes["name"].get<std::string>();
        std::string op_category = "calibration";
        double priority = fFormat["priority"].get<double>();
        std::string mk4id = ExtractStationMk4ID(op_name);

        //check pc_mode values to see if this operator should be built at all (defaults to true)
        //first we check if there is a 'pc_mode' defined under '/control/station/pc_mode'
        std::string pc_mode = "multitone";
        if(this->fParameterStore->IsPresent("/control/station/pc_mode"))
        {
            pc_mode = this->fParameterStore->GetAs<std::string>("/control/station/pc_mode");
        }

        //however, any station specific value under '/control/station/<mk4id>/pc_mode' will
        //override the generic /control/station/pc_mode
        std::string station_pcmode_path = std::string("/control/station/") + mk4id + "/pc_mode";
        if(this->fParameterStore->IsPresent(station_pcmode_path) )
        {
            pc_mode = this->fParameterStore->GetAs<std::string>(station_pcmode_path);
        }

        if(pc_mode == "multitone")
        {
            //grab the correct pcal data
            multitone_pcal_type* pcal_data = nullptr;
            if(op_name == "ref_multitone_pcal")
            {
                pcal_data = fContainerStore->GetObject<multitone_pcal_type>(std::string("ref_pcal"));
            }

            if(op_name == "rem_multitone_pcal")
            {
                pcal_data = fContainerStore->GetObject<multitone_pcal_type>(std::string("rem_pcal"));
            }

            //this is not necessarily an error...many stations do not have pcal
            if(pcal_data == nullptr )
            {
                msg_debug("initialization", "cannot construct MHO_MultitonePhaseCorrection without pcal data for station: "<<mk4id<< "." << eom);
                return false;
            }

            //retrieve the visibility data from the container store
            visibility_type* vis_data = fContainerStore->GetObject<visibility_type>(std::string("vis"));
            if( vis_data == nullptr )
            {
                msg_error("initialization", "cannot construct MHO_MultitonePhaseCorrection without visibility data." << eom);
                return false;
            }

            //retrieve the visibility data from the container store
            weight_type* weights = fContainerStore->GetObject<weight_type>(std::string("weight"));
            if( weights == nullptr )
            {
                msg_warn("initialization", "No weights available for MHO_MultitonePhaseCorrection, assuming weights of 1." << eom);
            }

            //grab the appropriate pc_period station parameter
            int pc_period = ExtractPCPeriod(mk4id);
            //grab the sampler delays (if present) and label each pol with them
            AttachSamplerDelays(pcal_data, mk4id);
            AttachPCToneMask(pcal_data, mk4id);

            //build the operator
            MHO_MultitonePhaseCorrection* op = new MHO_MultitonePhaseCorrection();
            op->SetName(op_name);
            op->SetArgs(vis_data);
            op->SetStationMk4ID(mk4id);
            op->SetMultitonePCData(pcal_data);
            op->SetPCPeriod(pc_period);
            op->SetWeights(weights);
            op->SetPriority(priority);

            msg_debug("initialization", "creating operator: "<<op_name<<" for station: "<<mk4id<<"."<<eom);

            bool replace_duplicates = true;
            this->fOperatorToolbox->AddOperator(op,op->GetName(),op_category,replace_duplicates);
            return true;
        }

        //multitone pcal not triggered for this station
        msg_debug("initialization", "MHO_MultitonePhaseCorrection will not be applied to station: "<<mk4id<<"." << eom);
        return false;
    }
    return false;
}



std::string
MHO_MultitonePhaseCorrectionBuilder::ExtractStationMk4ID(std::string op_name)
{
    std::string station_id = "?";
    if(op_name == fRefOpName){station_id = this->fParameterStore->GetAs<std::string>("/ref_station/mk4id");}
    if(op_name == fRemOpName){station_id = this->fParameterStore->GetAs<std::string>("/rem_station/mk4id");}
    return station_id;
}


int
MHO_MultitonePhaseCorrectionBuilder::ExtractPCPeriod(std::string mk4id)
{
    int pc_period = 1; //default value
    //generic path
    std::string pc_period_path = "/control/station/pc_period";
    //station specific path
    std::string station_pc_period_path = std::string("/control/station/") + mk4id + "/pc_period";
    if(fParameterStore->IsPresent(pc_period_path)){fParameterStore->Get(pc_period_path, pc_period);}
    if(fParameterStore->IsPresent(station_pc_period_path)){fParameterStore->Get(station_pc_period_path, pc_period);}
    return pc_period;
}

void
MHO_MultitonePhaseCorrectionBuilder::AttachSamplerDelays(multitone_pcal_type* pcal_data, std::string mk4id)
{
    //pulls the appropriate sampler delays from the parameter store, and labels the pcal data with them
    auto pol_ax = &(std::get<MTPCAL_POL_AXIS>(*pcal_data));

    for(std::size_t p = 0; p < pol_ax->GetSize(); p++)
    {
        std::string pol = pol_ax->at(p);
        std::string sampler_delay_key = GetSamplerDelayKey(pol);
        std::string sampler_groups_key = "samplers";
        if(sampler_delay_key != "")
        {
            //generic path
            std::vector<double> delays;
            std::string sd_path = std::string("/control/station/") + sampler_delay_key;
            std::string station_sd_path = std::string("/control/station/") + mk4id + "/" + sampler_delay_key;
            if(fParameterStore->IsPresent(sd_path)){fParameterStore->Get(sd_path, delays);}
            if(fParameterStore->IsPresent(station_sd_path)){fParameterStore->Get(station_sd_path, delays);}

            std::vector< std::string > sampler_groups;
            std::string sg_path = std::string("/control/station/") + sampler_groups_key;
            std::string station_sg_path = std::string("/control/station/") + mk4id + "/" + sampler_groups_key;
            if(fParameterStore->IsPresent(sg_path)){fParameterStore->Get(sg_path, sampler_groups);}
            if(fParameterStore->IsPresent(station_sg_path)){fParameterStore->Get(station_sg_path, sampler_groups);}

            if(delays.size() != 0 && delays.size() == sampler_groups.size())
            {
                pol_ax->InsertIndexLabelKeyValue(p, "sampler_delays", delays);
            }
            else if( sampler_groups.size() != 0)
            {
                msg_warn("initialization", "number of sampler delays specified for station: "<< mk4id << "(" <<delays.size() <<
                    ") does not match the number of samplers defined ("<<sampler_groups.size()<<"), assuming delays are all zero." << eom);
                delays.resize(sampler_groups.size(), 0.0);
                pol_ax->InsertIndexLabelKeyValue(p, "sampler_delays", delays);
            }
        }
    }
}


std::string
MHO_MultitonePhaseCorrectionBuilder::GetSamplerDelayKey(std::string pol)
{
    std::string key = "";
    if(pol == "X" || pol == "x"){key = "sampler_delay_x";}
    if(pol == "Y" || pol == "y"){key = "sampler_delay_y";}
    if(pol == "R" || pol == "r"){key = "sampler_delay_r";}
    if(pol == "L" || pol == "l"){key = "sampler_delay_l";}
    if(pol == "H" || pol == "h"){key = "sampler_delay_h";}
    if(pol == "V" || pol == "v"){key = "sampler_delay_v";}
    return key;
}


void MHO_MultitonePhaseCorrectionBuilder::AttachPCToneMask(multitone_pcal_type* pcal_data, std::string mk4id)
{
    //pulls the appropriate pc_tonemask data from the parameter store and labels the pcal data
    std::string pc_tonemask_key = "pc_tonemask";
    std::string channel_names_key = "channel_names";
    std::string tone_masks_key = "tone_masks";

    std::string channel_names = "";
    std::vector< int > tone_masks;
    std::string prefix;
    std::string chan_path;
    std::string mask_path;

    //generic paths
    prefix = std::string("/control/station/") + pc_tonemask_key;
    chan_path = prefix + "/" + channel_names_key;
    mask_path = prefix + "/" + tone_masks_key;
    if(fParameterStore->IsPresent(chan_path) && fParameterStore->IsPresent(mask_path) )
    {
        fParameterStore->Get(chan_path, channel_names);
        fParameterStore->Get(mask_path, tone_masks);
    }

    //station specific paths
    prefix = std::string("/control/station/") + mk4id + "/" + pc_tonemask_key;
    chan_path = prefix + "/" + channel_names_key;
    mask_path = prefix + "/" + tone_masks_key;
    if(fParameterStore->IsPresent(chan_path) && fParameterStore->IsPresent(mask_path) )
    {
        fParameterStore->Get(chan_path, channel_names);
        fParameterStore->Get(mask_path, tone_masks);
    }

    if(tone_masks.size() != 0 && (channel_names.size() == tone_masks.size()) )
    {
        pcal_data->Insert("pc_tonemask_channels", channel_names);
        pcal_data->Insert("pc_tonemask_bitmasks", tone_masks);
    }
    // else
    // {
    //     msg_warn("initialization", "could not configure pc_tonemask for station: "<< mk4id<< ", assuming no pc tone masks." << eom);
    // }

}

}//end namespace
