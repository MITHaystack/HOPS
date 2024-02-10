#include "MHO_BasicFringeDataConfiguration.hh"
#include "MHO_DirectoryInterface.hh"

//snapshot utility lib
#include "MHO_Snapshot.hh"

//configure_data_library
#include "MHO_ElementTypeCaster.hh"

#include "MHO_VexInfoExtractor.hh"

//parse_command_line
#include "MHO_Tokenizer.hh"
#include <getopt.h>

// #include <sys/ioctl.h>
// // #include <stdio.h>
// // #include <unistd.h>
// 
// //retrieve the width of the terminal 
// //(so we know when to line break)
// int get_window_columns()
// {
//     struct winsize w;
//     ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
//     return w.ws_col;
// }

namespace hops
{


int
MHO_BasicFringeDataConfiguration::parse_command_line(int argc, char** argv, MHO_ParameterStore* paramStore)
{
    //TODO update the usage string
    std::string usage = "ffit -c <control file> -b <baseline> -P <pol. product> <directory>";

    //command line parameters
    bool accounting = false; //'-a' perform run-time accounting - not yet enabled
    std::string baseline = ""; //'-b' baseline:frequency_group selection
    std::string freqgrp = ""; //'-b' frequency_group selection
    std::string control_file = ""; //'-c' specifies the control file
    std::string directory = ""; // specifies the data direct (TODO REMOVE ME)
    bool estimate_time = false; //'-e' estimate run time
    int first_plot_chan = 0; //'-n' specifies the first channel displayed in the fringe plot
    int message_level = -1; //'-m' specifies the message verbosity level
    int nplot_chans = 0; //'-n' specifies the number of channels to display in the fringe plot
    bool show_plot = false; //'-p' generates and shows fringe plot
    std::string refringe_alist_file = ""; // '-r' alist file for refringing - not yet enabled
    int ap_per_seg = 0; //'-s' specify the APs to be averaged per plot-segment
    bool test_mode = false; //'-t' if true, then no output is written
    bool update_mode = false; //'-u' not yet enabled
    std::string polprod = ""; //'-P' polarization product argument (e.g XX or I or RR+LL)
    std::string reftime = ""; //'-T' specify the fourfit reference time - not yet enabled
    bool xpower_output = false; //'-x' same as option '-p' we no long use pgplot/xwindows
    //std::string output_file = "fdump.json"; //'-o' specify the output file, for testing
    
    //store the raw arguments in the parameter store
    std::vector<std::string> arglist;
    for(int i=0; i<argc; i++){arglist.push_back( std::string(argv[i]) );}
    paramStore->Set("/cmdline/args", arglist);
    
    //detect and parse the set_string, if it exists, so we can ignore items which may be 
    //valid control parameters but look like options to get opt (e.g. 'set start -3')
    int set_arg_index = -1;
    std::string set_string = parse_set_string(arglist, set_arg_index);



    static struct option longOptions[] = 
    {
        {"help", no_argument, 0, 'h'},
        {"accounting", no_argument, 0, 'a'},
        {"baseline", required_argument, 0, 'b'},
        {"control", required_argument, 0, 'c'},
        {"device", required_argument, 0, 'd'},
        {"estimate-time", no_argument, 0, 'e'},
        {"first-plot-channel", required_argument, 0, 'f'},
        {"message-level", required_argument, 0, 'm'},
        {"nplot-chans", required_argument, 0, 'n'},
        {"plot", no_argument, 0, 'p'},
        {"refringe", required_argument, 0, 'r'},
        {"ap-per-seg", required_argument, 0, 's'},
        {"test-mode", no_argument, 0, 't'},
        {"update-mode", no_argument, 0, 'u'},
        {"xwindow", no_argument, 0, 'x'},
        {"polarization-product", required_argument, 0, 'P'},
        {"time-reference", required_argument, 0, 'T'},
        {"xpower-output", no_argument, 0, 'X'}
    };

    //these are nearly all of the options of the original fourfit
    //However, some are disabled, and the '-d' option has been coopted to point 
    //to the data directory, and '-o' is used to specify the output file name
    static const char* optString = "+hab:c:d:ef:m:n:pr:s:tuxP:T:X";
    //fourfit option string is "+ab:c:d:ef:m:n:pr:s:tuxP:T:X"

    while(true)
    {
        char optId = getopt_long(argc, argv, optString, longOptions, NULL);
        if (optId == -1)
            break;
        switch(optId)
        {
            case 'h':  // help
                print_usage();
                std::exit(0);
            case 'a':  // help
                accounting = true;
                msg_warn("fringe", "accounting option '-a' is not available." << eom);
            case 'b':
                parse_baseline_freqgrp(std::string(optarg), baseline, freqgrp);
                break;
            case 'c':
                control_file = std::string(optarg);
                break;
            case 'd':
                //directory = std::string(optarg);
                msg_warn("fringe", "device (plotting) option '-d' is not available." << eom);
                break;
            case 'e':
                estimate_time = true;
                msg_warn("fringe", "option '-e' is not available." << eom);
                break;
            case 'f':
                first_plot_chan = std::atoi(optarg);
                break;
            case 'p':
                show_plot = true;
                break;
            case 'r':
                refringe_alist_file = std::string(optarg);
                msg_fatal("fringe", "refringe option '-r' is not available." << eom);
                std::exit(1);
                break;
            case 's':
                ap_per_seg = std::atoi(optarg);
                if(ap_per_seg < 0){ap_per_seg = 0; msg_warn("fringe", "invalid ap_per_seg, ignoring." << eom);}
                break;
            case 't':
                test_mode = true;
                break;
            case 'u':
                msg_fatal("fringe", "option '-u' is not available." << eom);
                std::exit(1);
                break;
            case 'x':
                msg_warn("fringe", "option '-x' is unused/deprecated." << eom);
                show_plot = true; //equivalent to '-p', we do not use pgplot/xwindows
                break;
            case 'P':
                polprod = std::string(optarg);
                break;
            case 'T':
                reftime = std::string(optarg);
                msg_fatal("fringe", "alternate reference time option '-T' is not available." << eom);
                std::exit(1);
                break;
            case 'm':
                message_level = std::atoi(optarg);
                if(message_level < -2){message_level = -2;}
                if(message_level > 5){message_level = 5;}
                break;
            case 'n':
                nplot_chans = std::atoi(optarg);
                break;
            case 'X':
                xpower_output = false;
                msg_warn("fringe", "xpower output option '-X' is not available." << eom);
                break;
            // case 'o':
            //     output_file = std::string(optarg);
            //     break;
            case '?':
                if(set_arg_index != -1 && optind < set_arg_index)
                {
                    //invalid option or missing argument
                    msg_fatal("fringe", "invalid option or missing argument, use '-h' for help." << eom);
                    std::exit(1);
                }
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }
    
    //set the message level
    set_message_level(message_level);


    //resolve remaining positional arguments (data directory) and put them in the parameter store
    std::vector< std::string > pargs;
    if(set_arg_index == -1){set_arg_index = argc;}
    for(int i = optind; i < set_arg_index; i++){pargs.push_back( std::string(argv[i]) );}

    paramStore->Set("/cmdline/positional_args", pargs);
    if(pargs.size() != 1)
    {
        msg_fatal("fringe", "the data directory must be passed as the first positional argument." << eom );
        std::exit(1);
    }
    else{ directory = pargs[0]; }

    directory = sanitize_directory(directory);

    //pass the extracted command line info back in the parameter store
    //accounting = false;  //not implemented
    paramStore->Set("/cmdline/baseline", baseline);
    paramStore->Set("/cmdline/frequency_group", freqgrp);

    //TODO set the absolute path for the control file
    paramStore->Set("/cmdline/control_file",control_file);
    //TODO et the absolute path for the data directory 
    paramStore->Set("/cmdline/directory", directory);

    //estimate_time = false; //not implemented
    paramStore->Set("/cmdline/first_plot_channel", first_plot_chan); //TODO
    paramStore->Set("/cmdline/message_level", message_level);
    paramStore->Set("/cmdline/nplot_channels", nplot_chans); //TODO
    paramStore->Set("/cmdline/show_plot", show_plot); //TODO
    //refringe_alist_file = ""; //not implemented
    paramStore->Set("/cmdline/ap_per_seg",ap_per_seg);
    paramStore->Set("/cmdline/test_mode", test_mode); //TODO
    //update_mode = false; //not implemented
    paramStore->Set("/cmdline/polprod", polprod);
    //reftime = ""; //not implemented
    //xpower_output = false; //not implemented
    paramStore->Set("/cmdline/set_string", set_string); //TODO

    int status = sanity_check(paramStore);

    return status; //0 is ok, anything else is an error
}

void 
MHO_BasicFringeDataConfiguration::parse_baseline_freqgrp(std::string baseline_freqgrp, std::string& baseline, std::string& freqgrp)
{
    MHO_Tokenizer tokenizer;
    
    if( baseline_freqgrp.find(':') == std::string::npos )
    {
        baseline = baseline_freqgrp;
        
        if(baseline.size() != 2)
        {
            msg_fatal("fringe", "baseline must be passed as 2-char code."<< eom);
            std::exit(1);
        }
    }
    else 
    {
        //split on ':' into baseline and frequency group
        std::vector< std::string> tokens;
        std::string delim = ":";
        tokenizer.SetDelimiter(delim);
        tokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
        tokenizer.SetIncludeEmptyTokensFalse();
        tokenizer.SetString(&baseline_freqgrp);
        tokenizer.GetTokens(&tokens);
        if(tokens.size() != 2)
        {
            msg_fatal("fringe", "could not parse '-b' command line argument: "<<baseline_freqgrp<< eom);
            std::exit(1);
        }
        baseline = tokens[0];
        freqgrp = tokens[1];
    }
}

void 
MHO_BasicFringeDataConfiguration::set_message_level(int message_level)
{
    //set the message level according to the fourfit style
    //where 3 is least verbose, and '-1' is most verbose
    switch (message_level)
    {
        case -2:
            //NOTE: debug messages must be compiled-in
            #ifndef HOPS_ENABLE_DEBUG_MSG
            MHO_Message::GetInstance().SetMessageLevel(eInfo);
            msg_warn("fringe", "debug messages are toggled via compiler flag, re-compile with ENABLE_DEBUG_MSG=ON to enable." << eom);
            #else
            MHO_Message::GetInstance().SetMessageLevel(eDebug);
            #endif
        break;
        case -1:
            MHO_Message::GetInstance().SetMessageLevel(eInfo);
        break;
        case 0:
            MHO_Message::GetInstance().SetMessageLevel(eStatus);
        break;
        case 1:
            MHO_Message::GetInstance().SetMessageLevel(eWarning);
        break;
        case 2:
            MHO_Message::GetInstance().SetMessageLevel(eError);
        break;
        case 3:
            MHO_Message::GetInstance().SetMessageLevel(eFatal);
        break;
        case 4:
            //silent, but prints out the mk4 fringe file name to stderr if it is created
            //this is for backwards compatiblity with VGOS post-processing scripts
            MHO_Message::GetInstance().SetMessageLevel(eSpecial);
        break;
        case 5:
            //completely silent
            MHO_Message::GetInstance().SetMessageLevel(eSilent);
        break;
        default:
            //for now default is most verbose, eventually will change this to silent
            MHO_Message::GetInstance().SetMessageLevel(eDebug);
    }
}

std::string 
MHO_BasicFringeDataConfiguration::parse_set_string(const std::vector< std::string >& arglist, int& set_arg_index)
{
    set_arg_index = -1;
    int nargs = arglist.size();
    for(int i=0; i<nargs; i++)
    {
        if(arglist[i] == "set")
        {
            set_arg_index = i;
            break;
        }
    }

    //if we've found a 'set' command, assume everything after this is control 
    //file syntax and concatenate everything together with spaces
    std::string set_string = "";
    int start_idx = set_arg_index+1; 
    if(set_arg_index != -1 && start_idx < nargs-1 )
    {
        for(int i=start_idx; i<nargs; i++)
        {
            set_string += arglist[i];
            set_string += " ";
        }
    }
    
    return set_string;
}

std::string 
MHO_BasicFringeDataConfiguration::sanitize_directory(std::string dir)
{
    bool ok;
    std::string fullpath = MHO_DirectoryInterface::GetDirectoryFullPath(dir);
    ok = MHO_DirectoryInterface::DoesDirectoryExist(fullpath);
    if(ok){return fullpath;}

    //check if we have actually been passed a root-file instead (and need to return the prefix)
    std::string basename = MHO_DirectoryInterface::GetBasename(fullpath);
    std::string prefix = MHO_DirectoryInterface::GetPrefix(fullpath);
    std::size_t dot = basename.find('.');
    if(dot != std::string::npos)
    {
        std::string root_code = basename.substr(dot+1);
        if( root_code.size() == 6 )
        {
            //check if is a directory 
            bool ok = MHO_DirectoryInterface::DoesDirectoryExist(fullpath);
            if(!ok)
            {
                //we were actually passed a root file, so return the prefix 
                fullpath = prefix;
            }
        }
    }
    
    return fullpath;

}

//sanity check of parameters after command line parsing
int 
MHO_BasicFringeDataConfiguration::sanity_check(MHO_ParameterStore* paramStore)
{
    //command line parameters
    //bool accounting = false; //'-a' perform run-time accounting - not yet enabled
    std::string baseline = paramStore->GetAs<std::string>("/cmdline/baseline"); 
    std::string freqgrp = paramStore->GetAs<std::string>("/cmdline/frequency_group");
    std::string control_file = paramStore->GetAs<std::string>("/cmdline/control_file");
    std::string directory = paramStore->GetAs<std::string>("/cmdline/directory");
    //bool estimate_time = false; //'-e' estimate run time
    int first_plot_chan = paramStore->GetAs<int>("/cmdline/first_plot_channel");
    int message_level = paramStore->GetAs<int>("/cmdline/message_level");
    int nplot_chans = paramStore->GetAs<int>("/cmdline/nplot_channels");
    bool show_plot = paramStore->GetAs<bool>("/cmdline/show_plot");
    //std::string refringe_alist_file = ""; // '-r' alist file for refringing - not yet enabled
    int ap_per_seg = paramStore->GetAs<int>("/cmdline/ap_per_seg");
    bool test_mode = paramStore->GetAs<bool>("/cmdline/test_mode");
    //bool update_mode = false; //'-u' not yet enabled
    std::string polprod = paramStore->GetAs<std::string>("/cmdline/polprod");
    //std::string reftime = ""; 
    //bool xpower_output = false; 
    //std::string output_file = paramStore->GetAs<std::string>("/cmdline/output_file");

    #pragma message("TODO FIXME - fill out the sanity_check function for command line arguments")
    if( directory == "" || baseline == "" || polprod == "" || control_file == "")
    {
        return 1;
    }
    
    return 0;
}




void MHO_BasicFringeDataConfiguration::populate_initial_parameters(MHO_ParameterStore* paramStore, MHO_ScanDataStore* scanStore)
{
    // //initialize by setting "is_finished" to false, and 'skipped' to false
    // //these parameters must always be present
    // paramStore->Set("/status/is_finished", false);
    // paramStore->Set("/status/skipped", false);

    //these should all be present and ok at this point
    std::string directory = paramStore->GetAs<std::string>("/cmdline/directory");
    std::string control_file = paramStore->GetAs<std::string>("/cmdline/control_file");
    std::string baseline = paramStore->GetAs<std::string>("/cmdline/baseline");
    std::string polprod = paramStore->GetAs<std::string>("/cmdline/polprod");

    ////////////////////////////////////////////////////////////////////////////
    //INITIALIZE PARAMETERS
    ////////////////////////////////////////////////////////////////////////////

    //set up the file section of the parameter store to record the directory, root file, and control file
    paramStore->Set("/files/control_file", control_file);
    paramStore->Set("/files/directory", directory);
    //paramStore->Set("/files/output_file", paramStore->GetAs<std::string>("/cmdline/output_file"));

    //put the baseline and pol product selection into the parameter store
    paramStore->Set("/config/polprod", polprod);
    paramStore->Set("/config/baseline", baseline);

    //parse the polprod string in order to determine which pol-products are needed (if more than one)
    std::vector< std::string > pp_vec = determine_required_pol_products(polprod);
    paramStore->Set("/config/polprod_set", pp_vec);

    ////////////////////////////////////////////////////////////////////////////
    //INITIALIZE SCAN DIRECTORY
    ////////////////////////////////////////////////////////////////////////////

    //initialize the scan store from this directory
    scanStore->SetDirectory(directory);
    scanStore->Initialize();
    if( !scanStore->IsValid() )
    {
        msg_fatal("fringe", "cannot initialize a valid scan store from this directory: " << directory << eom);
        std::exit(1);
    }

    if( !scanStore->IsBaselinePresent(baseline) )
    {
        msg_fatal("fringe", "cannot find the specified baseline: " << baseline << " in " << directory << eom);
        std::exit(1);
    }

    //set the root file name
    paramStore->Set("/files/root_file", scanStore->GetRootFileBasename() );
    
    msg_debug("fringe", "loading root file: "<< scanStore->GetRootFileBasename() << eom);

     // //load root file and extract useful vex info into parameter store
    auto vexInfo = scanStore->GetRootFileData();
    MHO_VexInfoExtractor::extract_vex_info(vexInfo, paramStore);

}

//more helper functions
void
MHO_BasicFringeDataConfiguration::configure_visibility_data(MHO_ContainerStore* store)
{
    //retrieve the (first) visibility and weight objects
    //(currently assuming there is only one object per type)
    visibility_store_type* vis_store_data = nullptr;
    weight_store_type* wt_store_data = nullptr;

    vis_store_data = store->GetObject<visibility_store_type>(0);
    wt_store_data = store->GetObject<weight_store_type>(0);

    if(vis_store_data == nullptr)
    {
        msg_fatal("initialization", "failed to read visibility data from the .cor file." <<eom);
        std::exit(1);
    }

    if(wt_store_data == nullptr)
    {
        msg_fatal("initialization", "failed to read weight data from the .cor file." <<eom);
        std::exit(1);
    }

    std::size_t n_vis = store->GetNObjects<visibility_store_type>();
    std::size_t n_wt = store->GetNObjects<weight_store_type>();

    if(n_vis != 1 || n_wt != 1)
    {
        msg_warn("initialization", "multiple visibility and/or weight types per-baseline not yet supported" << eom);
    }

    auto vis_store_uuid = vis_store_data->GetObjectUUID();
    auto wt_store_uuid = wt_store_data->GetObjectUUID();

    std::string vis_shortname = store->GetShortName(vis_store_uuid);
    std::string wt_shortname = store->GetShortName(wt_store_uuid);

    visibility_type* vis_data = new visibility_type();
    weight_type* wt_data = new weight_type();

    //assign the storage UUID's to their up-casted counter-parts
    //we do this so we can associate them to the file objects (w.r.t to program output, error messages, etc.)
    vis_data->SetObjectUUID(vis_store_uuid);
    wt_data->SetObjectUUID(wt_store_uuid);

    MHO_ElementTypeCaster<visibility_store_type, visibility_type> up_caster;
    up_caster.SetArgs(vis_store_data, vis_data);
    up_caster.Initialize();
    up_caster.Execute();

    MHO_ElementTypeCaster< weight_store_type, weight_type> wt_up_caster;
    wt_up_caster.SetArgs(wt_store_data, wt_data);
    wt_up_caster.Initialize();
    wt_up_caster.Execute();

    //remove the original objects to save space
    store->DeleteObject(vis_store_data);
    store->DeleteObject(wt_store_data);

    //warn on non-standard shortnames
    if(vis_shortname != "vis"){msg_warn("initialization", "visibilities do not use canonical short name 'vis', but are called: "<< vis_shortname << eom);}
    if(wt_shortname != "weight"){msg_warn("initialization", "weights do not use canonical short name 'weight', but are called: "<< wt_shortname << eom);}

    //now shove the double precision data into the container store with the same shortname
    store->AddObject(vis_data);
    store->AddObject(wt_data);
    store->SetShortName(vis_data->GetObjectUUID(), vis_shortname);
    store->SetShortName(wt_data->GetObjectUUID(), wt_shortname);
}

void
MHO_BasicFringeDataConfiguration::configure_station_data(MHO_ScanDataStore* scanStore, MHO_ContainerStore* containerStore,
                                                         std::string ref_station_mk4id, std::string rem_station_mk4id)
{
    //load station data and assign them the names 'ref_sta' or 'rem_sta'
    scanStore->LoadStation(ref_station_mk4id, containerStore);
    containerStore->RenameObject("sta", "ref_sta");
    MHO_UUID pcal_uuid;
    pcal_uuid = containerStore->GetObjectUUID("pcal");
    if( !(pcal_uuid.is_empty()) )
    {
        containerStore->RenameObject("pcal", "ref_pcal");
    }

    scanStore->LoadStation(rem_station_mk4id, containerStore);
    containerStore->RenameObject("sta", "rem_sta");
    pcal_uuid = containerStore->GetObjectUUID("pcal");
    if( !(pcal_uuid.is_empty()) )
    {
        containerStore->RenameObject("pcal", "rem_pcal");
    }
}

void
MHO_BasicFringeDataConfiguration::init_and_exec_operators(MHO_OperatorBuilderManager* build_manager, MHO_OperatorToolbox* opToolbox, const char* category)
{
    std::string cat(category);
    if(build_manager == nullptr || opToolbox == nullptr)
    {
        msg_error("fringe", "cannot initialize or execute operators if builder or toolbox is missing" << eom );
        return;
    }
    
    msg_debug("fringe", "initializing and executing operators in "<<cat<<" category."<<eom);

    build_manager->BuildOperatorCategory(cat);
    auto ops = opToolbox->GetOperatorsByCategory(cat);
    for(auto opIt= ops.begin(); opIt != ops.end(); opIt++)
    {
        msg_debug("fringe", "initializing and executing operator: "<< (*opIt)->GetName() <<", with priority: "<< (*opIt)->Priority() << "." << eom);
        (*opIt)->Initialize();
        (*opIt)->Execute();
    }
}

std::vector< std::string > 
MHO_BasicFringeDataConfiguration::determine_required_pol_products(std::string polprod)
{
    MHO_Tokenizer tokenizer;
    std::set<std::string> pp_set;
    std::vector<std::string> pp_vec;
    //first we parse the polprod string to see what individual pol-products we need 
    if( polprod.find("+") != std::string::npos)
    {
        //we have a pol-product summation like (RR+LL) or XX+YY, or RX+RY
        //so split on all '+' symbols (currently we only support '+' not '-')
        tokenizer.SetDelimiter("+");
        tokenizer.SetString(&polprod);
        //fTokenizer.SetUseMulticharacterDelimiterFalse();
        tokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
        tokenizer.SetIncludeEmptyTokensFalse();
        tokenizer.GetTokens(&pp_vec);
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

    std::stringstream ss;
    for(std::size_t i=0; i<pp_vec.size(); i++)
    {
        ss << pp_vec[i];
        if(i != pp_vec.size() - 1){ss <<", "; }
    }
    msg_debug("fringe", "required pol-products are: {" << ss.str() << "}." << eom );

    return pp_vec;
}

void 
MHO_BasicFringeDataConfiguration::print_usage()
{
    // bool accounting = false; //'-a' perform run-time accounting - not yet enabled
    // std::string baseline = ""; //'-b' baseline:frequency_group selection
    // std::string freqgrp = ""; //'-b' frequency_group selection
    // std::string control_file = ""; //'-c' specifies the control file
    // std::string directory = ""; // specifies the data direct (TODO REMOVE ME)
    // bool estimate_time = false; //'-e' estimate run time
    // int first_plot_chan = 0; //'-n' specifies the first channel displayed in the fringe plot
    // int message_level = -1; //'-m' specifies the message verbosity level
    // int nplot_chans = 0; //'-n' specifies the number of channels to display in the fringe plot
    // bool show_plot = false; //'-p' generates and shows fringe plot
    // std::string refringe_alist_file = ""; // '-r' alist file for refringing - not yet enabled
    // int ap_per_seg = 0; //'-s' specify the APs to be averaged per plot-segment
    // bool test_mode = false; //'-t' if true, then no output is written
    // bool update_mode = false; //'-u' not yet enabled
    // std::string polprod = ""; //'-P' polarization product argument (e.g XX or I or RR+LL)
    // std::string reftime = ""; //'-T' specify the fourfit reference time - not yet enabled
    // bool xpower_output = false; //'-x' same as option '-p' we no long use pgplot/xwindows
    // std::string output_file = "fdump.json"; //'-o' specify the output file, for testing

    MHO_Message::GetInstance().SetMessageLevel(eDebug);
    msg_info("main", "ffit usage is of the form: ffit <options> <data directory> <set control>" << eol);
    msg_info("main", "the data directory argument is required" << eol );
    msg_info("main", "the optional keyword 'set' is used to pass control file syntax" << eol );
    msg_info("main", "ffit supports the following options:" << eol);
    msg_info("main", "(-h, --help) print help message and exit." << eom);

    std::stringstream ss;
    ss << "usage: ffit ";
    ss << "[-h] ";
    ss << "[-a] ";
    ss << "[-b BASELINE or BASELINE:FREQGRP] ";
    ss << "[-c CONTROL_FILE] ";
    ss << "[-d DEVICE]";
    ss << "[-e] ";
    ss << "[-f FIRST_PLOT_CHANNEL] ";
    ss << "[-m MSG_LEVEL] ";
    ss << "[-n N_PLOT_CHANNELS]";
    ss << "[-p] ";
    ss << "[-r ALIST_FILE] ";
    ss << "[-s AP_PER_SEG] ";
    ss << "[-t] ";
    ss << "[-u] ";
    ss << "[-x] ";
    ss << "[-P POLARIZATION_PRODUCT] ";
    ss << "[-T TIME_REFERENCE] ";
    ss << "[-X] ";
    ss << "\n";
    ss << "<data_directory> ";
    ss << "<set control> ";

    msg_info("main", ss.str() << eom );

    // static struct option longOptions[] = 
    // {
    //     {"help", no_argument, 0, 'h'},
    //     {"accounting", no_argument, 0, 'a'},
    //     {"baseline", required_argument, 0, 'b'},
    //     {"control", required_argument, 0, 'c'},
    //     {"device", required_argument, 0, 'd'},
    //     {"estimate-time", no_argument, 0, 'e'},
    //     {"first-plot-channel", required_argument, 0, 'f'},
    //     {"message-level", required_argument, 0, 'm'},
    //     {"nplot-chans", required_argument, 0, 'n'},
    //     {"plot", no_argument, 0, 'p'},
    //     {"refringe", required_argument, 0, 'r'},
    //     {"ap-per-seg", required_argument, 0, 's'},
    //     {"test-mode", no_argument, 0, 't'},
    //     {"update-mode", no_argument, 0, 'u'},
    //     {"xwindow", no_argument, 0, 'x'},
    //     {"polarization-product", required_argument, 0, 'P'},
    //     {"time-reference", required_argument, 0, 'T'},
    //     {"xpower-output", no_argument, 0, 'X'},
    //     {"output", required_argument, 0, 'o'}
    // };

    // usage: vgoscf_generate.py [-h] [-v VERBOSITY] [-n NUM_PROC] [-s SNR_MIN] [-q QUALITY_LOWER_LIMIT] [-d DTEC_THRESH] [-b BEGIN_SCAN_LIMIT] [-e END_SCAN_LIMIT] [-c SIGMA_CUT_FACTOR]
    //                           [-w] [-p] [-o OUTPUT_FILENAME] [-a AVERAGING_SCAN_LIMIT] [-t] [-y NCHANNEL_DISCARD_THRESHOLD] [-z CHANNEL_DISCARD_TOLERANCE]
    //                           control_file network_reference_station stations data_directory
    // 
    // utility for constructing a control file suitable for pseudo-Stokes-I fringe fitting of VGOS sessions
    // 
    // positional arguments:
    //   control_file          the control file to be applied to all scans
    //   network_reference_station
    //                         single character code of station used as network reference
    //   stations              concatenated string of single codes of non-network-reference stations of interest
    //   data_directory        relative path to directory containing experiment or scan data, cannot contain "prepass" or "scratch" and must point to a folder with a 4-digit name
    // 
    // optional arguments:
    //   -h, --help            show this help message and exit
    //   -v VERBOSITY, --verbosity VERBOSITY
    //                         verbosity level: 0 (least verbose) to 3 (most verbose), default=2.
    //   -n NUM_PROC, --num-proc NUM_PROC
    //                         number of concurrent fourfit jobs to run, default=1
    //   -s SNR_MIN, --snr-min SNR_MIN
    //                         set minimum allowed snr threshold, default=30.
    //   -q QUALITY_LOWER_LIMIT, --quality-limit QUALITY_LOWER_LIMIT
    //                         set the lower limit on fringe quality (inclusive), default=6.
    // 


    // static struct option longOptions[] = 
    // {
    //     {"help", no_argument, 0, 'h'},
    //     {"accounting", no_argument, 0, 'a'},
    //     {"baseline", required_argument, 0, 'b'},
    //     {"control", required_argument, 0, 'c'},
    //     {"device", required_argument, 0, 'd'},
    //     {"estimate-time", no_argument, 0, 'e'},
    //     {"first-plot-channel", required_argument, 0, 'f'},
    //     {"message-level", required_argument, 0, 'm'},
    //     {"nplot-chans", required_argument, 0, 'n'},
    //     {"plot", no_argument, 0, 'p'},
    //     {"refringe", required_argument, 0, 'r'},
    //     {"ap-per-seg", required_argument, 0, 's'},
    //     {"test-mode", no_argument, 0, 't'},
    //     {"update-mode", no_argument, 0, 'u'},
    //     {"xwindow", no_argument, 0, 'x'},
    //     {"polarization-product", required_argument, 0, 'P'},
    //     {"time-reference", required_argument, 0, 'T'},
    //     {"xpower-output", no_argument, 0, 'X'},
    //     {"output", required_argument, 0, 'o'}
    // };


    
        //command line parameters

}

}//end namespace
