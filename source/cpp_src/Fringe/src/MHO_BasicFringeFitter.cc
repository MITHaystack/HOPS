#include "MHO_BasicFringeFitter.hh"

//snapshot utility lib
#include "MHO_Snapshot.hh"

//control
#include "MHO_ControlFileParser.hh"
#include "MHO_ControlConditionEvaluator.hh"

//initialization
#include "MHO_OperatorBuilderManager.hh"
#include "MHO_ParameterConfigurator.hh"
#include "MHO_ParameterManager.hh"

//fringe finding library helper functions
#include "MHO_BasicFringeDataConfiguration.hh"
#include "MHO_BasicFringeInfo.hh"
#include "MHO_InitialFringeInfo.hh"
#include "MHO_BasicFringeUtilities.hh"
#include "MHO_FringePlotInfo.hh"
#include "MHO_VexInfoExtractor.hh"

namespace hops
{


MHO_BasicFringeFitter::MHO_BasicFringeFitter():MHO_FringeFitter(){ count =0;};

MHO_BasicFringeFitter::~MHO_BasicFringeFitter(){};

void MHO_BasicFringeFitter::Configure()
{
    //initialize by setting "is_finished" to false, and 'skipped' to false
    //these parameters must always be present
    fParameterStore.Set("/status/is_finished", false);
    fParameterStore.Set("/status/skipped", false);

    std::string directory = fParameterStore.GetAs<std::string>("/cmdline/directory");
    std::string control_file = fParameterStore.GetAs<std::string>("/cmdline/control_file");
    std::string baseline = fParameterStore.GetAs<std::string>("/cmdline/baseline");
    std::string polprod = fParameterStore.GetAs<std::string>("/cmdline/polprod");

    //if any of these are empty, fail out for now
    if(directory == "" || control_file == "" || baseline == "" || polprod == "")
    {
        msg_fatal("fringe", "missing command line information (directory, control file, baseline, or pol-product)" << eom );
        std::exit(1);
    }

    ////////////////////////////////////////////////////////////////////////////
    //INITIALIZE PARAMETERS
    ////////////////////////////////////////////////////////////////////////////

    //set up the file section of the parameter store to record the directory, root file, and control file
    fParameterStore.Set("/files/control_file", control_file);
    fParameterStore.Set("/files/directory", directory);
    fParameterStore.Set("/files/output_file", fParameterStore.GetAs<std::string>("/cmdline/output_file"));

    //put the baseline and pol product selection into the parameter store
    fParameterStore.Set("/config/polprod", polprod);
    fParameterStore.Set("/config/baseline", baseline);

    //parse the polprod string in order to determine which pol-products are needed (if more than one)
    std::vector< std::string > pp_vec = DetermineRequiredPolProducts(polprod);
    fParameterStore.Set("/config/polprod_set", pp_vec);

    ////////////////////////////////////////////////////////////////////////////
    //INITIALIZE SCAN DIRECTORY
    ////////////////////////////////////////////////////////////////////////////

    //initialize the scan store from this directory
    fScanStore.SetDirectory(directory);
    fScanStore.Initialize();
    if( !fScanStore.IsValid() )
    {
        msg_fatal("fringe", "cannot initialize a valid scan store from this directory: " << directory << eom);
        std::exit(1);
    }

    //set the root file name
    fParameterStore.Set("/files/root_file", fScanStore.GetRootFileBasename() );

    //load root file and extract useful vex info
    fVexInfo = fScanStore.GetRootFileData();
    MHO_VexInfoExtractor::extract_vex_info(fVexInfo, &fParameterStore);

    ////////////////////////////////////////////////////////////////////////////
    //CONTROL CONSTRUCTION
    ////////////////////////////////////////////////////////////////////////////
    // std::string control_file = fParameterStore.GetAs<std::string>("/cmdline/control_file");
    // std::string baseline = fParameterStore.GetAs<std::string>("/cmdline/baseline");
    MHO_ControlFileParser cparser;
    MHO_ControlConditionEvaluator ceval;

    //specify the control format
    fControlFormat = MHO_ControlDefinitions::GetControlFormat();
    
    //add default operations to the control format, so we can later trigger them
    AddDefaultOperatorFormatDef(fControlFormat);

    //now parse the control file and collect the applicable statements 
    cparser.SetControlFile(control_file);
    auto control_contents = cparser.ParseControl();
    //TODO -- where should frequency group information get stashed/retrieved?
    std::string srcName = fParameterStore.GetAs<std::string>("/vex/scan/source/name");
    std::string scnName = fParameterStore.GetAs<std::string>("/vex/scan/name");
    ceval.SetPassInformation(baseline, srcName, "?", scnName);//baseline, source, fgroup, scan
    fControlStatements = ceval.GetApplicableStatements(control_contents);

    //tack on default-operations to the control statements, so we can trigger
    //the build of these operators at the proper step (e.g. coarse selection, multitone pcal etc.)
    AddDefaultOperators( (*(fControlStatements.begin()))["statements"] );

    std::cout<<fControlStatements.dump(2)<<std::endl;
    std::cout<<"*****************************************************************************"<<std::endl;

    //set some intiail/default parameters (polprod, ref_freq)
    MHO_InitialFringeInfo::set_default_parameters_minimal(&fParameterStore);
    //configure parameter store from control statements
    MHO_ParameterManager paramManager(&fParameterStore, fControlFormat);
    paramManager.SetControlStatements(&fControlStatements);
    paramManager.ConfigureAll();

    //the control statement 'skip' is special because we want to bail out
    //as soon as possible (before reading in data) in order to save time
    if( fParameterStore.IsPresent("skip") )
    {
        bool do_skip = fParameterStore.GetAs<bool>("skip");
        if(do_skip)
        {
            //set "is_finished" to true, since we are skipping this data
            fParameterStore.Set("/status/skipped", true);
            fParameterStore.Set("/status/is_finished", true);
        }
    }
    
    //fParameterStore.Dump();
    
    //now build the operator build manager
    fOperatorBuildManager = new MHO_OperatorBuilderManager(&fOperatorToolbox, &fContainerStore, &fParameterStore, fControlFormat);
}

void MHO_BasicFringeFitter::Initialize()
{
    bool skipped = fParameterStore.GetAs<bool>("/status/skipped");
    if( !skipped )
    {
        std::string baseline = fParameterStore.GetAs<std::string>("/config/baseline");
        std::string polprod = fParameterStore.GetAs<std::string>("/config/polprod");

        ////////////////////////////////////////////////////////////////////////////
        //LOAD DATA AND ASSEMBLE THE DATA STORE
        ////////////////////////////////////////////////////////////////////////////

        //load baseline data
        fScanStore.LoadBaseline(baseline, &fContainerStore);
        fParameterStore.Set("/files/baseline_input_file", fScanStore.GetBaselineFilename(baseline));

        //loads visibility data and performs float -> double cast
        MHO_BasicFringeDataConfiguration::configure_visibility_data(&fContainerStore);

        visibility_type* vis_data = fContainerStore.GetObject<visibility_type>(std::string("vis"));
        weight_type* wt_data = fContainerStore.GetObject<weight_type>(std::string("weight"));
        if( vis_data == nullptr || wt_data == nullptr )
        {
            msg_fatal("fringe", "could not find visibility or weight objects with names (vis, weight)." << eom);
            std::exit(1);
        }
        std::string vis_uuid = vis_data->GetObjectUUID().as_string();
        std::string wt_uuid = wt_data->GetObjectUUID().as_string();
        fParameterStore.Set("/uuid/visibilities", vis_uuid);
        fParameterStore.Set("/uuid/weights", wt_uuid);

        //load and rename station data according to reference/remote
        //also load pcal data if it is present
        std::string ref_station_mk4id = std::string(1,baseline[0]);
        std::string rem_station_mk4id = std::string(1,baseline[1]);
        MHO_BasicFringeDataConfiguration::configure_station_data(&fScanStore, &fContainerStore, ref_station_mk4id, rem_station_mk4id);
        fParameterStore.Set("/files/ref_station_input_file", fScanStore.GetStationFilename(ref_station_mk4id));
        fParameterStore.Set("/files/rem_station_input_file", fScanStore.GetStationFilename(rem_station_mk4id));

        station_coord_type* ref_data = fContainerStore.GetObject<station_coord_type>(std::string("ref_sta"));
        station_coord_type* rem_data = fContainerStore.GetObject<station_coord_type>(std::string("rem_sta"));
        if( ref_data == nullptr || rem_data == nullptr )
        {
            msg_fatal("fringe", "could not find station coordinate data with names (ref_sta, rem_sta)." << eom);
            std::exit(1);
        }
        std::string ref_uuid = ref_data->GetObjectUUID().as_string();
        std::string rem_uuid = rem_data->GetObjectUUID().as_string();
        fParameterStore.Set("/uuid/ref_coord", ref_uuid);
        fParameterStore.Set("/uuid/rem_coord", rem_uuid);

        multitone_pcal_type* ref_pcal_data = fContainerStore.GetObject<multitone_pcal_type>(std::string("ref_pcal"));
        multitone_pcal_type* rem_pcal_data = fContainerStore.GetObject<multitone_pcal_type>(std::string("rem_pcal"));
        if( ref_pcal_data != nullptr)
        {
            std::string ref_pcal_uuid = ref_pcal_data->GetObjectUUID().as_string();
            fParameterStore.Set("/uuid/ref_pcal", ref_pcal_uuid);
        }
        if( rem_pcal_data != nullptr )
        {
            std::string rem_pcal_uuid = rem_pcal_data->GetObjectUUID().as_string();
            fParameterStore.Set("/uuid/rem_pcal", rem_pcal_uuid);
        }

        ////////////////////////////////////////////////////////////////////////////
        //PARAMETER SETTING
        ////////////////////////////////////////////////////////////////////////////
        MHO_InitialFringeInfo::configure_reference_frequency(&fContainerStore, &fParameterStore);

        ////////////////////////////////////////////////////////////////////////////
        //CONFIGURE THE OPERATOR BUILD MANAGER
        ////////////////////////////////////////////////////////////////////////////
        fOperatorBuildManager->CreateDefaultBuilders();
        fOperatorBuildManager->SetControlStatements(&fControlStatements);

        //take a snapshot if enabled
        // visibility_type* vis_data = fContainerStore.GetObject<visibility_type>(std::string("vis"));
        // weight_type* wt_data = fContainerStore.GetObject<weight_type>(std::string("weight"));
        take_snapshot_here("test", "visib", __FILE__, __LINE__, vis_data);
        take_snapshot_here("test", "weights", __FILE__, __LINE__,  wt_data);

        ////////////////////////////////////////////////////////////////////////////
        //OPERATOR CONSTRUCTION
        ////////////////////////////////////////////////////////////////////////////

        fOperatorBuildManager->BuildOperatorCategory("default");
        
        
        std::cout<<"Dumping the parameter store: = "<<std::endl;
        fParameterStore.Dump();
        
        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "labeling");
        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "selection");

        //safety check
        if(vis_data->GetSize() == 0){msg_fatal("fringe", "no visibility data left after cuts." << eom); std::exit(1);}
        if(wt_data->GetSize() == 0){msg_fatal("fringe", "no weight data left after cuts." << eom); std::exit(1);}

        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "flagging");
        MHO_BasicFringeDataConfiguration::init_and_exec_operators(fOperatorBuildManager, &fOperatorToolbox, "calibration");

        //calulate useful quantities to stash in the parameter store
        MHO_InitialFringeInfo::precalculate_quantities(&fContainerStore, &fParameterStore);
    }
}

void MHO_BasicFringeFitter::PreRun()
{
    bool skipped = fParameterStore.GetAs<bool>("/status/skipped");
    if( !skipped) //execute if we are not finished and are not skipping
    {
        //TODO FILL ME IN -- need to call specified user-scripts here
    }
}

void MHO_BasicFringeFitter::Run()
{
    bool status_is_finished = fParameterStore.GetAs<bool>("/status/is_finished");
    bool skipped = fParameterStore.GetAs<bool>("/status/skipped");
    if( !status_is_finished  && !skipped) //execute if we are not finished and are not skipping
    {
        //execute the basic fringe search algorithm
        MHO_BasicFringeUtilities::basic_fringe_search(&fContainerStore, &fParameterStore);
        //calculate the fringe properties
        MHO_BasicFringeUtilities::calculate_fringe_solution_info(&fContainerStore, &fParameterStore, fVexInfo);

        fParameterStore.Set("/status/is_finished", true);

        fParameterStore.Dump();
    }
}

void MHO_BasicFringeFitter::PostRun()
{
    bool skipped = fParameterStore.GetAs<bool>("/status/skipped");
    if( !skipped) //execute if we are not finished and are not skipping
    {
        //TODO FILL ME IN -- need to call specified user-scripts here
    }
}


bool MHO_BasicFringeFitter::IsFinished()
{
    bool is_finished = fParameterStore.GetAs<bool>("/status/is_finished");
    return is_finished;
}


void MHO_BasicFringeFitter::Finalize()
{
    ////////////////////////////////////////////////////////////////////////////
    //PLOTTING/DEBUG
    ////////////////////////////////////////////////////////////////////////////
    //TODO may want to reorg the way this is done

    bool status_is_finished = fParameterStore.GetAs<bool>("/status/is_finished");
    bool skipped = fParameterStore.GetAs<bool>("/status/skipped");
    if( status_is_finished  && !skipped ) //have to be finished and not-skipped
    {
        fPlotData = MHO_FringePlotInfo::construct_plot_data(&fContainerStore, &fParameterStore, fVexInfo);
        MHO_FringePlotInfo::fill_plot_data(&fParameterStore, fPlotData);
    }
}


void 
MHO_BasicFringeFitter::AddDefaultOperatorFormatDef(mho_json& format)
{
    //this is bit of a hack to get these operators
    //(which cannot be triggered via control file statements)
    //into the initialization stream (part 1)
    
    //add the data selection operator
    fDataSelectFormat =
    {
        {"name", "coarse_selection"},
        {"statement_type", "operator"},
        {"operator_category" , "selection"},
        {"type" , "empty"},
        {"priority", 1.01}
    };
    format["coarse_selection"] = fDataSelectFormat;

    mho_json sampler_labeler =
    {
        {"name", "sampler_labeler"},
        {"statement_type", "operator"},
        {"operator_category" , "labeling"},
        {"type" , "empty"},
        {"priority", 0.10}
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
    fControlFormat["ref_multitone_pcal"] = ref_multitone_pcal_format;

    //add a multitone pcal op for the remote station
    mho_json rem_multitone_pcal_format =
    {
        {"name", "rem_multitone_pcal"},
        {"statement_type", "operator"},
        {"operator_category" , "calibration"},
        {"type" , "empty"},
        {"priority", 3.1}
    };
    fControlFormat["rem_multitone_pcal"] = rem_multitone_pcal_format;
}

void 
MHO_BasicFringeFitter::AddDefaultOperators(mho_json& statements)
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

std::vector< std::string > 
MHO_BasicFringeFitter::DetermineRequiredPolProducts(std::string polprod)
{
    std::set<std::string> pp_set;
    std::vector<std::string> pp_vec;
    //first we parse the polprod string to see what individual pol-products we need 
    if( polprod.find("+") != std::string::npos)
    {
        //we have a pol-product summation like (RR+LL) or XX+YY, or RX+RY
        //so split on all '+' symbols (currently we only support '+' not '-')
        fTokenizer.SetDelimiter("+");
        fTokenizer.SetUseMulticharacterDelimiterFalse();
        fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
        fTokenizer.SetIncludeEmptyTokensFalse();
        fTokenizer.GetTokens(&pp_vec);
    }
    else if(polprod == "I") //special pseudo-Stokes-I mode (linear pol only)
    {
        pp_vec.push_back("XX");
        pp_vec.push_back("YY");
        pp_vec.push_back("XY");
        pp_vec.push_back("YX");
    }
    else 
    {
        pp_vec.push_back(polprod); //polprod is just a single value
    }

    //push the values into a set, so we don't have any duplicates
    pp_set.insert( pp_vec.begin(), pp_vec.end() );

    //push the set values into the vector for return
    pp_vec.clear();
    pp_vec.insert(pp_vec.begin(), pp_set.begin(), pp_set.end() );


    std::cout<<"PP VEC:"<<std::endl;
    for(std::size_t i=0; i<pp_vec.size(); i++)
    {
        std::cout<<"ppvec @ "<<i<<" = "<<pp_vec[i]<<std::endl;
    }

    return pp_vec;
}

}//end namespace
