#include "MHO_ManualChannelPhaseCorrectionBuilder.hh"
#include "MHO_ManualChannelPhaseCorrection.hh"

#include "MHO_Meta.hh"
#include "MHO_ControlUtilities.hh"


namespace hops
{




bool
MHO_ManualChannelPhaseCorrectionBuilder::Build()
{
    MHO_ManualChannelPhaseCorrection* op = new MHO_ManualChannelPhaseCorrection();

    //assume attributes are ok for now - TODO add checks!
    std::string op_name = fAttributes["name"].get<std::string>();
    std::string channel_name_str = fAttributes["channel_names"].get<std::string>();
    std::vector<double> pc_phases = fAttributes["pc_phases"].get< std::vector<double> >();
    std::vector< std::string > chan_names = SplitChannelLabels(channel_name_str);

    // {
    //     "name": "pc_phases_x",
    //     "statement_type": "parameter",
    //     "type" : "compound",
    //     "parameters":
    //     {
    //         "channel_names": {"type": "string"},
    //         "pc_phases": {"type": "list_real"}
    //     },
    //     "fields":
    //     [
    //         "channel_names",
    //         "pc_phases"
    //     ]
    // }


    if( pc_phases.size() == chan_names.size() )
    {
        auto label2pcp = zip_into_map(chan_names, pc_phases); //name -> freq
        //op->SetChannelLabelToFrequencyMap(label2freq);
        //return a default channel labelling operator

        // bool replace_duplicates = true;
        // MHO_OperatorToolbox::GetInstance().AddOperator(op,op_name,replace_duplicates);

        return false;
        //return true;
    }
    else
    {
        delete op;
        op = nullptr;
        msg_error("builders", "cannot set pc_phases with unequal number of channels/elements " <<
                  "(channels, pc_phases) = (" << chan_names.size() << ", " << pc_phases.size() << ")"
                  << eom );
        return false;
    }
}



}//end namespace
