#include "MHO_FringeControlInitialization.hh"
#include "MHO_DirectoryInterface.hh"


#include "MHO_ControlDefinitions.hh"
#include "MHO_ControlFileParser.hh"
#include "MHO_ControlConditionEvaluator.hh"
#include "MHO_ParameterManager.hh"

#include "MHO_VexInfoExtractor.hh"
#include "MHO_InitialFringeInfo.hh"


namespace hops
{

bool
MHO_FringeControlInitialization::need_ion_search(mho_json* control)
{
    std::vector< std::string > ion_keywords;
    ion_keywords.push_back("ion_win");
    ion_keywords.push_back("ion_npts");
    ion_keywords.push_back("ionosphere");
    ion_keywords.push_back("ion_smooth");
    //loop over control statements, find statements which contain ionospheric
    //search related settings, return true on first one encountered
    for(auto ctrl_iter = control->begin(); ctrl_iter != control->end(); ctrl_iter++)
    {
        auto ctrl_item = *(ctrl_iter);
        if( ctrl_iter->contains("statements") )
        {
            auto statements = &( (*ctrl_iter)["statements"] );
            for(auto stmt_iter = statements->begin(); stmt_iter != statements->end(); stmt_iter++)
            {
                std::string name = (*stmt_iter)["name"];
                for(std::size_t i=0;i<ion_keywords.size(); i++)
                {
                    if(name == ion_keywords[i]){return true;}
                }
            }
        }
    }
    return false;
}


void MHO_FringeControlInitialization::process_control_file(MHO_ParameterStore* paramStore, mho_json& control_format, mho_json& control_statements)
{
    //initialize by setting "is_finished" to false, and 'skipped' to false
    //these parameters must always be present
    paramStore->Set("/status/is_finished", false);
    paramStore->Set("/status/skipped", false);

    //these should all be present and ok at this point (handled by command line parse functions)
    std::string directory = paramStore->GetAs<std::string>("/files/directory");
    std::string control_file = paramStore->GetAs<std::string>("/files/control_file");
    std::string baseline = paramStore->GetAs<std::string>("/config/baseline");
    std::string polprod = paramStore->GetAs<std::string>("/config/polprod");
    std::string fgroup = paramStore->GetAs<std::string>("/config/fgroup");
    std::vector< std::string > pp_vec = paramStore->GetAs< std::vector< std::string > >("/config/polprod_set");

    ////////////////////////////////////////////////////////////////////////////
    //CONTROL CONSTRUCTION
    ////////////////////////////////////////////////////////////////////////////
    MHO_ControlFileParser cparser;
    MHO_ControlConditionEvaluator ceval;

    //specify the control format
    control_format = MHO_ControlDefinitions::GetControlFormat();

    //add default operations to the control format, so we can later trigger them
    add_default_operator_format_def(control_format);

    //add the set string info if it is available
    std::string set_string = paramStore->GetAs<std::string>("/cmdline/set_string");
    if(set_string != ""){cparser.PassSetString(set_string);}

    //now parse the control file and collect the applicable statements
    cparser.SetControlFile(control_file);
    auto control_contents = cparser.ParseControl();

    //stash the processed text in the parameter store
    //TODO FIXME -- we may want to move this elsewhere
    // std::string parsed_control = cparser.GetProcessedControlFileText();
    std::string parsed_control = cparser.GetLegacyProcessedControlFileText();
    paramStore->Set("/control/control_file_contents", parsed_control);

    //TODO -- where should frequency group information get stashed/retrieved?
    std::string srcName = paramStore->GetAs<std::string>("/vex/scan/source/name");
    std::string scnName = paramStore->GetAs<std::string>("/vex/scan/name");

    std::string fgroup_code = "?";
    if(fgroup != ""){fgroup_code = fgroup;}
    ceval.SetPassInformation(baseline, srcName, fgroup_code, scnName);//baseline, source, fgroup, scan
    control_statements = ceval.GetApplicableStatements(control_contents);

    //tack on default-operations to the control statements, so we can trigger
    //the build of these operators at the proper step (e.g. coarse selection, multitone pcal etc.)
    add_default_operators( (*(control_statements.begin()))["statements"] );

    //add the pol-product summation operator into the execution stream if we were passed
    //such a thing on the command line (e.g. RR+LL or I)
    std::size_t npp = pp_vec.size();
    if(npp > 1)
    {
        add_polprod_sum_operator( (*(control_statements.begin()))["statements"] );
    }
    else if( npp == 1)
    {
        //single polarization product, check if it is linear
        if( is_linear_polprod(pp_vec[0]) )
        {
            add_dpar_sign_correction_operator( (*(control_statements.begin()))["statements"] );
        }
    }

    //set some initial/default parameters (polprod, ref_freq)
    MHO_InitialFringeInfo::set_default_parameters_minimal(paramStore);

    //Unlike hops3 we don't want to default to using the ionospheric fringe fitter always, only when it is actually needed.
    //So in order to detect if ionospheric fitting is required we have to do a quick pre-pass of the control statements
    //to check for ion keywords before processing/consuming them
    bool do_ion = MHO_FringeControlInitialization::need_ion_search(&control_statements);
    paramStore->Set("/config/do_ion", do_ion);

    //configure parameter store from control statements
    //note that this class consumes relevant control statements (and removes them upon use)
    MHO_ParameterManager paramManager(paramStore, control_format);
    paramManager.SetControlStatements(&control_statements);
    paramManager.ConfigureAll();

    //the control statement 'skip' is special because we want to bail out
    //as soon as possible (before reading in data) in order to save time
    if( paramStore->IsPresent("skip") )
    {
        bool do_skip = paramStore->GetAs<bool>("skip");
        if(do_skip)
        {
            //set "is_finished" to true, since we are skipping this data
            paramStore->Set("/status/skipped", true);
            paramStore->Set("/status/is_finished", true);
        }
    }

}


void
MHO_FringeControlInitialization::add_default_operator_format_def(mho_json& format)
{
    //this is bit of a hack to get these operators
    //(which cannot be triggered via control file statements)
    //into the initialization stream (part 1)

    //add the data selection operator
    mho_json data_select_format =
    {
        {"name", "coarse_selection"},
        {"statement_type", "operator"},
        {"operator_category" , "selection"},
        {"type" , "empty"},
        {"priority", 1.01}
    };
    format["coarse_selection"] = data_select_format;

    mho_json sampler_labeler =
    {
        {"name", "sampler_labeler"},
        {"statement_type", "operator"},
        {"operator_category" , "labeling"},
        {"type" , "empty"},
        {"priority", 0.9}
    };
    format["sampler_labeler"] = sampler_labeler;

    //add a multitone pcal op for the reference station
    mho_json ref_multitone_pcal_format =
    {
        {"name", "ref_multitone_pcal"},
        {"statement_type", "operator"},
        {"operator_category" , "calibration"},
        {"type" , "empty"},
        {"priority", 3.1}
    };
    format["ref_multitone_pcal"] = ref_multitone_pcal_format;

    //add a multitone pcal op for the remote station
    mho_json rem_multitone_pcal_format =
    {
        {"name", "rem_multitone_pcal"},
        {"statement_type", "operator"},
        {"operator_category" , "calibration"},
        {"type" , "empty"},
        {"priority", 3.1}
    };
    format["rem_multitone_pcal"] = rem_multitone_pcal_format;

    mho_json polprod_sum_format =
    {
        {"name", "polproduct_sum"},
        {"statement_type", "operator"},
        {"operator_category" , "calibration"},
        {"type" , "empty"},
        {"priority", 3.99}
    };
    format["polproduct_sum"] = polprod_sum_format;

    mho_json dpar_corr_format =
    {
        {"name", "dpar_corr"},
        {"statement_type", "operator"},
        {"operator_category" , "calibration"},
        {"type" , "empty"},
        {"priority", 3.99}
    };
    format["dpar_corr"] = dpar_corr_format;
}


void
MHO_FringeControlInitialization::add_default_operators(mho_json& statements)
{
    //this is the rest of the default operators hack
    //in part 2 (here) we actually define control statements that trigger
    //these operators to be built and exectuted

    mho_json coarse_selection_hack =
    {
       {"name", "coarse_selection"},
       {"statement_type", "operator"},
       {"operator_category" , "selection"}
    };
    statements.push_back(coarse_selection_hack);

    mho_json sampler_hack =
    {
       {"name", "sampler_labeler"},
       {"statement_type", "operator"},
       {"operator_category" , "labeling"}
    };
    statements.push_back(sampler_hack);

    //add default ops for multi-tone pcal
    //note: this operator checks if the pcal data is available and if pc_mode != manual
    //if either condition fails, it does not get inserted into the execution stream
    mho_json ref_multitone_pcal_hack =
    {
        {"name", "ref_multitone_pcal"},
        {"statement_type", "operator"},
        {"operator_category" , "calibration"}
    };

    mho_json rem_multitone_pcal_hack =
    {
        {"name", "rem_multitone_pcal"},
        {"statement_type", "operator"},
        {"operator_category" , "calibration"}
    };

    statements.push_back(ref_multitone_pcal_hack);
    statements.push_back(rem_multitone_pcal_hack);
}

void
MHO_FringeControlInitialization::add_polprod_sum_operator(mho_json& statements)
{
    mho_json pp_sum_hack =
    {
       {"name", "polproduct_sum"},
       {"statement_type", "operator"},
       {"operator_category" , "calibration"}
    };
    statements.push_back(pp_sum_hack);
}

void
MHO_FringeControlInitialization::add_dpar_sign_correction_operator(mho_json& statements)
{
    mho_json dpar_corr_hack =
    {
       {"name", "dpar_corr"},
       {"statement_type", "operator"},
       {"operator_category" , "calibration"}
    };
    statements.push_back(dpar_corr_hack);
}

bool MHO_FringeControlInitialization::is_linear_polprod(std::string pp)
{
    if(pp == "XX"){return true;}
    if(pp == "XY"){return true;}
    if(pp == "YY"){return true;}
    if(pp == "YX"){return true;}

    if(pp == "HH"){return true;}
    if(pp == "HV"){return true;}
    if(pp == "VV"){return true;}
    if(pp == "VH"){return true;}
    return false;
}

}//end namespace
