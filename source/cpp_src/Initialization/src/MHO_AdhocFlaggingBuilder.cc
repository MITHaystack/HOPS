#include "MHO_AdhocFlaggingBuilder.hh"
#include "MHO_AdhocFlagging.hh"

namespace hops
{

bool MHO_AdhocFlaggingBuilder::Build()
{
    if(!IsConfigurationOk())
    {
        return false;
    }

    msg_debug("initialization", "building adhoc_flag_file operator." << eom);

    // The toolbox name for the single unified flagging operator.
    // Using a distinct name avoids collision with the control statement name.
    const std::string op_name = "adhoc_flagging";
    const std::string op_category = fFormat["operator_category"].get< std::string >();
    double priority = fFormat["priority"].get< double >();

    // Extract the flag file path from the compound statement value.
    std::string flag_file = fAttributes["value"]["flag_file"].get< std::string >();

    // Retrieve the weight data (the operator acts on weights).
    weight_type* wt_data = fContainerStore->GetObject< weight_type >(std::string("weight"));
    if(wt_data == nullptr)
    {
        msg_error("initialization", "adhoc_flag_file: cannot construct MHO_AdhocFlagging without weight data." << eom);
        return false;
    }

    // Determine which station(s) this statement applies to.
    std::string ref_id = fParameterStore->GetAs< std::string >("/ref_station/site_id");
    std::string rem_id = fParameterStore->GetAs< std::string >("/rem_station/site_id");
    std::string station_id = ExtractStationIdentifier();

    bool applies_to_ref = (station_id == "??" || station_id == ref_id);
    bool applies_to_rem = (station_id == "??" || station_id == rem_id);

    msg_debug("initialization", "adhoc_flag_file: station_id='" << station_id << "'  ref='" << ref_id << "'  rem='" << rem_id
                                                                << "'  flag_file='" << flag_file << "'"
                                                                << "  applies_ref=" << applies_to_ref
                                                                << "  applies_rem=" << applies_to_rem << eom);

    // look for an already-created operator in the toolbox from a previous call
    // (e.g. the ref station's statement was processed first; now updating rem).
    MHO_AdhocFlagging* flag_op = nullptr;
    MHO_Operator* existing = fOperatorToolbox->GetOperator(op_name);
    if(existing != nullptr)
    {
        flag_op = dynamic_cast< MHO_AdhocFlagging* >(existing);
        if(flag_op == nullptr)
        {
            msg_error("initialization",
                      "adhoc_flag_file: operator named '"
                          << op_name << "' already exists in the toolbox but is not an MHO_AdhocFlagging instance." << eom);
            return false;
        }
    }

    bool first_time = (flag_op == nullptr);
    if(first_time)
    {
        flag_op = new MHO_AdhocFlagging();
        flag_op->SetArgs(wt_data);
        flag_op->SetName(op_name);
        flag_op->SetPriority(priority);
        flag_op->SetParameterStore(fParameterStore);
    }

    // Set the file path for whichever station(s) this statement covers.
    if(applies_to_ref)
    {
        flag_op->SetRefFlagFile(flag_file);
    }
    if(applies_to_rem)
    {
        flag_op->SetRemFlagFile(flag_file);
    }

    // Register the operator on first creation only; subsequent calls only update it.
    if(first_time)
    {
        bool replace_duplicates = false;
        fOperatorToolbox->AddOperator(flag_op, op_name, op_category, replace_duplicates);
    }

    msg_debug("initialization", "adhoc_flag_file: operator '" << op_name << "' " << (first_time ? "created" : "updated")
                                                              << " (ref='" << flag_op->GetRefFlagFile() << "'  rem='"
                                                              << flag_op->GetRemFlagFile() << "')" << eom);

    return true;
}

std::string MHO_AdhocFlaggingBuilder::ExtractStationIdentifier()
{
    std::string station_id = "??";
    std::vector< std::string > condition_values = fConditions["value"].get< std::vector< std::string > >();

    for(auto it = condition_values.begin(); it != condition_values.end(); it++)
    {
        if(*it == "station")
        {
            ++it;
            if(it != condition_values.end())
            {
                station_id = *it;
                return station_id;
            }
        }
    }
    return station_id;
}

} // namespace hops
