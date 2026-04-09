#include "MHO_FringeCommandLineParser.hh"
#include "MHO_DirectoryInterface.hh"

#include "MHO_FringeDataDiscovery.hh"

//option parsing and help text library
#include "CLI11.hpp"

namespace hops
{

void MHO_FringeCommandLineParser::parse_baseline_freqgrp(std::string baseline_freqgrp, std::string& baseline,
                                                              std::string& freqgrp)
{
    MHO_Tokenizer tokenizer;
    if(baseline_freqgrp.find(':') == std::string::npos)
    {
        //no ':' present, so we must only have a baseline specification
        baseline = baseline_freqgrp;
        freqgrp = "?"; //not passed, set to wildcard
    }
    else
    {
        //split on ':' into baseline and frequency group
        std::vector< std::string > tokens;
        std::string delim = ":";
        tokenizer.SetDelimiter(delim);
        tokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
        tokenizer.SetIncludeEmptyTokensFalse();
        tokenizer.SetString(&baseline_freqgrp);
        tokenizer.GetTokens(&tokens);
        if(tokens.size() != 2)
        {
            msg_fatal("fringe", "could not parse '-b' command line argument: " << baseline_freqgrp << eom);
            std::exit(1);
        }
        baseline = tokens[0];
        freqgrp = tokens[1];
    }

    if(baseline.size() != 2)
    {
        if(baseline.find('-') == std::string::npos)
        {
            //error out if something odd was passed
            msg_fatal(
                "fringe",
                "baseline must be passed as pair of 1-char MK4 ids or station codes delimited by '-' (e.g. GE or Gs-Wf), not: "
                    << baseline << eom);
            std::exit(1);
        }
    }
}

std::string MHO_FringeCommandLineParser::parse_set_string(const std::vector< std::string >& arglist, int& set_arg_index)
{
    set_arg_index = -1;
    int nargs = arglist.size();
    for(int i = 0; i < nargs; i++)
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
    int start_idx = set_arg_index + 1;
    if(set_arg_index != -1 && start_idx < nargs - 1)
    {
        for(int i = start_idx; i < nargs; i++)
        {
            set_string += arglist[i];
            set_string += " ";
        }
    }

    return set_string;
}

std::string MHO_FringeCommandLineParser::sanitize_directory(std::string dir)
{
    //first lets determine if this actually a directory (or if is a file)
    bool is_dir = MHO_DirectoryInterface::IsDirectory(dir);
    std::string path = dir;
    if(is_dir)
    {
        //this is really a directory, so just make sure it ends with a '/'
        if(path.back() != '/')
        {
            path += "/";
        }
    }
    else
    {
        //this is actually a file (probably the root file)
        //so strip the file and return the directory it is in
        std::size_t dir_end = dir.find_last_of("/");
        if(dir_end != std::string::npos)
        {
            path = dir.substr(0, dir_end) + "/";
        }
    }

    msg_debug("fringe", "sanitized directory path: " << path << eom);

    //check that this directory exists
    bool ok;
    std::string fullpath = MHO_DirectoryInterface::GetDirectoryFullPath(path);
    ok = MHO_DirectoryInterface::DoesDirectoryExist(fullpath);
    if(!ok)
    {
        msg_error("fringe", "could not find the directory: " << fullpath << eom);
        std::exit(1);
    }
    return path; //we do not return the full-path in order to preserve symlinks
}


//sanity check of parameters after command line parsing
int MHO_FringeCommandLineParser::sanity_check(MHO_ParameterStore* paramStore)
{
    //command line parameters
    std::string baseline = paramStore->GetAs< std::string >("/cmdline/baseline");
    std::string freqgrp = paramStore->GetAs< std::string >("/cmdline/frequency_group");
    std::string control_file = paramStore->GetAs< std::string >("/cmdline/control_file");
    std::string input_directory = paramStore->GetAs< std::string >("/cmdline/input_directory");
    //bool estimate_time = false; //'-e' estimate run time
    int first_plot_chan = paramStore->GetAs< int >("/cmdline/first_plot_channel");
    int message_level = paramStore->GetAs< int >("/cmdline/message_level");
    int nplot_chans = paramStore->GetAs< int >("/cmdline/nplot_channels");
    bool show_plot = paramStore->GetAs< bool >("/cmdline/show_plot");
    //std::string refringe_alist_file = ""; // '-r' alist file for refringing - not yet enabled
    int ap_per_seg = paramStore->GetAs< int >("/cmdline/ap_per_seg");
    bool test_mode = paramStore->GetAs< bool >("/cmdline/test_mode");
    std::string polprod = paramStore->GetAs< std::string >("/cmdline/polprod");
    //std::string reftime = "";
    //bool xpower_output = false;
    //std::string output_file = paramStore->GetAs<std::string>("/cmdline/output_file");

    TODO_FIXME_MSG("TODO FIXME - fill out the sanity_check function for command line arguments")
    if(input_directory == "")
    {
        return 1;
    }

    return 0;
}




int MHO_FringeCommandLineParser::parse_fourfit_command_line(int argc, char** argv, MHO_ParameterStore* paramStore)
{
    //store the raw arguments in the parameter store
    std::vector< std::string > arglist;
    for(int i = 0; i < argc; i++)
    {
        arglist.push_back(std::string(argv[i]));
    }
    paramStore->Set("/cmdline/args", arglist);

    //command line parameters
    bool accounting = false;                       //'-a' perform run-time accounting/profiling
    std::string baseline_opt = "??:?";             //'-b' baseline:frequency_group selection
    std::string baseline = "??";                   // the baseline
    std::string freqgrp = "?";                     // the frequency group
    std::string control_file = "";                 //'-c' specifies the control file
    std::string disk_file = "";                    //'-d' specifies the name of the plot file
    bool exclude_autos = false;                    //'-e' exclude auto-corrs
    int first_plot_chan = 0;                       //'-n' specifies the first channel displayed in the fringe plot
    std::string output_directory = "";             //'-o' specify the output directory
    int omp_threads = -1;                          //'-O' set the number of threads used by OpenMP (if enabled at compile time)
    int message_level = -1;                        //'-m' specifies the message verbosity level
    std::vector< std::string > message_categories; // -'M' limits the allowed message categories to those the user specifies
    int nplot_chans = 0;                           //'-n' specifies the number of channels to display in the fringe plot
    bool show_plot = false;                        //'-p' generates and shows fringe plot
    std::string refringe_alist_file = "";          // '-r' alist file for refringing - not yet enabled
    int ap_per_seg = 0;                            //'-s' specify the APs to be averaged per plot-segment
    bool test_mode = false;                        //'-t' if true, then no output is written
    std::string polprod = "??";                    //'-P' polarization product argument (e.g XX or I or RR+LL)
    std::string reftime = "";                      //'-T' specify the fourfit reference time - not yet enabled
    //bool xwindows; //'-x' same as option '-p' we no long use pgplot/xwindows
    int xpower_output = -1; //export xpower spectrum (default '-1' is to not export this)
    bool use_mk4_output = false;
    bool use_mk4_input = false;
    std::string input;

    std::vector< std::string > msg_cats = {"main",      "calibration", "containers",     "control",
                                           "fringe",    "file",        "initialization", "mk4interface",
                                           "utilities", "vex",         "plot",           "python_bindings"};

    std::stringstream ss;
    ss << "limit the allowed message categories to only those which the user specifies, the available categories are: \n";
    for(auto it = msg_cats.begin(); it != msg_cats.end(); it++)
    {
        ss << "    " << *it << "\n";
    }
    ss << "if the '-M' option is not used, the default is to allow all categories. ";
    std::string msg_cat_help = ss.str();

    CLI::App app{"fourfit"};

    // Remove help flag because it shortcuts all processing
    app.set_help_flag();

    // Add custom flag that activates help
    auto* help = app.add_flag("-h,--help", "print this help message and exit");
    app.add_flag("-a,--accounting", accounting, "perform run-time accounting/profiling");
    app.add_option("-b,--baseline", baseline_opt, "baseline or baseline:frequency_group selection (e.g GE or GE:X)");
    app.add_option("-c,--control-file", control_file, "specify the control file");
    app.add_option("-d,--disk-file", disk_file, "specify the file name where the plot will be saved");
    app.add_flag("-e,--exclude-autocorrs", exclude_autos, "exclude auto-correlations from fringe-fitting");
    app.add_option("-f,--first-plot-channel", first_plot_chan,
                   "specifies the first channel displayed in the fringe plot (ignored, not yet implemented)");
    app.add_option("-M,--message-categories", message_categories, msg_cat_help.c_str())->delimiter(',');
    app.add_option("-m,--message-level", message_level, "message level to be used, range: -2 (debug) to 5 (silent)");
    app.add_option("-n,--nplot-channels", nplot_chans,
                   "specifies the number of channels to display in the fringe plot (ignored, not yet implemented)");
    app.add_option("-o,--output-directory", output_directory, "set the directory under which to write output files (default is input)");
    app.add_option("-O,--openmp-threads", omp_threads, "set the number of threads used by OpenMP (if enabled, default=max)");
    app.add_flag("-p,--plot", show_plot, "generate and shows fringe plot on completion");
    app.add_option("-r,--refringe-alist", refringe_alist_file, "alist file for refringing (ignored, not yet implemented)");
    app.add_option("-s,--ap-per-segment", ap_per_seg, "specify the APs to be averaged per plot-segment");
    app.add_flag("-t,--test-mode", test_mode, "if passed, then no output is written");
    app.add_option("-P,--polprod", polprod, "polarization product argument (e.g XX or I or RR+LL, etc.)");
    app.add_option("-T,--reftime", reftime, "specify the fourfit reference time (ignored, not yet implemented)");
    //app.add_flag("-x,--xwindows", xwindows, "display plot using xwindows (ignored, deprecated)");
    app.add_option("-X,--xpower-output", xpower_output,
                   "append cross power data with fringe solution applied, specifying the axis along which data should be "
                   "summed (-1=no-export, 0=none, 1=channel, 2=time/AP, 3=sub-channel), default: -1");
    app.add_option("input,-i,--input", input, "name of the input directory (scan) or root file")->required();
    app.add_flag("-k,--mark4-output", use_mk4_output, "write output files in mark4 type_2xx format");
    app.add_flag("-K,--mark4-input", use_mk4_input, "accept mark4 format input directory (converted to HOPS format via temp dir at runtime)");

    //add the 'set' command for control file parameter overrides
    auto* setcom =
        app.add_subcommand("set", "pass control file parameters and related syntax on the command line")->prefix_command();

    try
    {
        app.parse(argc, argv);
        if(*help)
        {
            throw CLI::CallForHelp();
        }
    }
    catch(const CLI::Error& e)
    {
        std::cout << app.help() << std::endl;
        std::exit(1); //just exit don't bother returning to main
    }

    //grab the set string if present
    std::string set_string = " ";
    std::vector< std::string > set_tokens;
    for(const auto& aarg : setcom->remaining())
    {
        set_tokens.push_back(aarg);
        set_string += aarg + " ";
    }

    //configure messaging
    MHO_FringeCommandLineParser::initialize_messaging(message_level, message_categories);

    //enable profiling if passed '-a'
    if(accounting)
    {
        MHO_Profiler::GetInstance().Enable();
    }

    //catch no input case
    if(input == "")
    {
        msg_fatal("main", "input directory/root file not set" << eom);
        std::exit(1);
    }

    //catch no control file case and set to /dev/null
    //TODO detect DEF_CONTROL environmental variable if present and use that
    if(control_file == "")
    {
        control_file = "/dev/null";
    }

    //for now we require these options to be set (may relax this once we allow mult-pass fringe fitting)
    MHO_FringeCommandLineParser::parse_baseline_freqgrp(baseline_opt, baseline, freqgrp);

    //set the message level
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    //clean the directory string
    std::string input_directory = MHO_FringeCommandLineParser::sanitize_directory(input);

    //if there is no root file (i.e. we were passed and experiment directory, we return an empty string)
    std::string root_file = MHO_FringeDataDiscovery::find_associated_root_file(input);

    //pass the extracted command line info back in the parameter store
    paramStore->Set("/cmdline/accounting", accounting);
    paramStore->Set("/cmdline/baseline", baseline);
    paramStore->Set("/cmdline/frequency_group", freqgrp);

    paramStore->Set("/cmdline/control_file", control_file);
    paramStore->Set("/cmdline/disk_file", disk_file); //default is empty string -> no plot file
    paramStore->Set("/cmdline/input_directory", input_directory); //sanitized input directory path

    if(output_directory == ""){output_directory = input_directory;}
    paramStore->Set("/cmdline/output_directory", output_directory); //output_directory path

    paramStore->Set("/cmdline/root_file", root_file); //fully resolved (symlink free path to the root file)...or empty
    paramStore->Set("/cmdline/exclude_autos", exclude_autos);
    paramStore->Set("/cmdline/first_plot_channel", first_plot_chan); //TODO
    paramStore->Set("/cmdline/message_level", message_level);
    paramStore->Set("/cmdline/nplot_channels", nplot_chans); //TODO
    paramStore->Set("/cmdline/omp_threads", omp_threads);    //does nothing if OpenMP has not been enabled
    paramStore->Set("/cmdline/show_plot", show_plot);        //TODO
    //refringe_alist_file = ""; //not implemented
    paramStore->Set("/cmdline/ap_per_seg", ap_per_seg);
    paramStore->Set("/cmdline/test_mode", test_mode); //TODO
    paramStore->Set("/cmdline/polprod", polprod);
    //reftime = ""; //not implemented
    paramStore->Set("/cmdline/xpower_output", xpower_output);
    paramStore->Set("/cmdline/set_string", set_string); //TODO
    paramStore->Set("/cmdline/mk4format_output", use_mk4_output);
    paramStore->Set("/cmdline/mk4format_input", use_mk4_input);

    int status = MHO_FringeCommandLineParser::sanity_check(paramStore);

    return status; //0 is ok, anything else is an error
}

void MHO_FringeCommandLineParser::initialize_messaging(int message_level, const std::vector< std::string>& message_categories)
{
    //clamp message level
    if(message_level > 5)
    {
        message_level = 5;
    }
    if(message_level < -2)
    {
        message_level = -2;
    }
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    //check if any message categories were passed, if so, we limit the messages
    //to only those categories
    if(message_categories.size() != 0)
    {
        for(std::size_t m = 0; m < message_categories.size(); m++)
        {
            MHO_Message::GetInstance().AddKey(message_categories[m]);
        }
        MHO_Message::GetInstance().LimitToKeySet();
    }
}


}//end namespace
