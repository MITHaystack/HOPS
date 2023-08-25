#include "ffit.hh"

//extract useful information about this scan from the vex info
void extract_vex_info(const mho_json& vexInfo, MHO_ParameterStore* paramStore)
{
    //get scan name and source name
    mho_json::json_pointer sched_pointer("/$SCHED");
    auto sched = vexInfo.at(sched_pointer);
    if(sched.size() != 1)
    {
        msg_error("main", "root file contains missing or ambiguous $SCHED information." << eom );
        std::exit(1);
    }

    //get the source information
    std::string scnName = sched.begin().key();
    std::string src_loc = "/$SCHED/" + scnName + "/source/0/source";
    mho_json::json_pointer src_jptr(src_loc);
    std::string srcName = vexInfo.at(src_jptr).get<std::string>();
    paramStore->Set("/vex/scan/name",scnName);
    paramStore->Set("/vex/scan/source/name",srcName);

    mho_json src_section = vexInfo["$SOURCE"];
    auto src_info = src_section.begin().value();
    std::string ra = src_info["ra"];
    std::string dec = src_info["dec"];
    paramStore->Set("/vex/scan/source/ra", ra);
    paramStore->Set("/vex/scan/source/dec", dec);

    //get the fourfit reference time 
    std::string frt_loc = "/$SCHED/" + scnName + "/fourfit_reftime";
    mho_json::json_pointer frt_jptr(frt_loc);
    std::string frt_string = vexInfo.at(frt_jptr).get<std::string>();
    std::cout<<"FOURFIT REFERENCE TIME = "<<frt_string<<std::endl;
    paramStore->Set("/vex/scan/fourfit_reftime", frt_string);
    
    std::string start_loc = "/$SCHED/" + scnName + "/start";
    mho_json::json_pointer start_jptr(start_loc);
    std::string start_string = vexInfo.at(start_jptr).get<std::string>();
    std::cout<<"START TIME = "<<start_string<<std::endl;
    paramStore->Set("/vex/scan/start", start_string);
    
    //get experiment info
    mho_json exper_section = vexInfo["$EXPER"];
    auto exper_info = exper_section.begin().value();
    std::string exper_name = exper_info["exper_name"];
    paramStore->Set("/vex/experiment_name", exper_name); //required

    if(exper_info.contains("exper_num"))
    {
        std::stringstream ss;
        ss << exper_info["exper_num"];
        paramStore->Set("/vex/experiment_number", ss.str());
    }
    else
    {
        paramStore->Set("/vex/experiment_number", "----");
    }

    //NOTE: this implicitly assumes that all channels have the same bandwidth!
    mho_json freq_section = vexInfo["$FREQ"];
    auto freq_info = freq_section.begin().value();
    double sample_rate = freq_info["sample_rate"]["value"];
    std::string sample_rate_units = freq_info["sample_rate"]["units"];
    double factor = 1.0;
    #pragma message("TODO: handle units other than MHz")
    if(sample_rate_units == "MHz")
    {
        factor = 1e6;
    }
    sample_rate *= sample_rate*factor;
    
    paramStore->Set("/vex/scan/sample_rate", sample_rate);
    paramStore->Set("/vex/scan/sample_period", 1.0/sample_rate);
    
    //TODO FIXME, we also need extract the clock model, as well as station information
    
}
