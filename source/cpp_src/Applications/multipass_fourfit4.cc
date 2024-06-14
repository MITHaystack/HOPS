#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <getopt.h>

//option parsing and help text library
#include "CLI11.hpp"

#define EXTRA_DEBUG

#include "MHO_Message.hh"
#include "MHO_Snapshot.hh"
#include "MHO_Timer.hh"

//fringe finding library helper functions
#include "MHO_BasicFringeFitter.hh"
#include "MHO_IonosphericFringeFitter.hh"

//for control intialization
#include "MHO_BasicFringeDataConfiguration.hh"
#include "MHO_FringeControlInitialization.hh"

//pybind11 stuff to interface with python
#ifdef USE_PYBIND11
    #include <pybind11/pybind11.h>
    #include <pybind11/embed.h>
    #include "pybind11_json/pybind11_json.hpp"
    namespace py = pybind11;
    namespace nl = nlohmann;
    using namespace pybind11::literals;
    #include "MHO_PythonOperatorBuilder.hh"
    #include "MHO_PyConfigurePath.hh"
#endif

//needed to export to mark4 fringe files
#include "MHO_MK4FringeExport.hh"


using namespace hops;

//TODO move this some where else


int parse_command_line(int argc, char** argv, MHO_ParameterStore* paramStore)
{
    //store the raw arguments in the parameter store
    std::vector<std::string> arglist;
    for(int i=0; i<argc; i++){arglist.push_back( std::string(argv[i]) );}
    paramStore->Set("/cmdline/args", arglist);

    //command line parameters
    bool accounting = false; //'-a' perform run-time accounting/profiling
    std::string baseline_opt = ""; //'-b' baseline:frequency_group selection
    std::string baseline = ""; // the baseline
    std::string freqgrp = ""; // the frequency group
    std::string control_file = ""; //'-c' specifies the control file
    bool estimate_time = false; //'-e' estimate run time
    int first_plot_chan = 0; //'-n' specifies the first channel displayed in the fringe plot
    int message_level = -1; //'-m' specifies the message verbosity level
    std::vector< std::string > message_categories;  // -'M' limits the allowed message categories to those the user specifies
    int nplot_chans = 0; //'-n' specifies the number of channels to display in the fringe plot
    bool show_plot = false; //'-p' generates and shows fringe plot
    std::string refringe_alist_file = ""; // '-r' alist file for refringing - not yet enabled
    int ap_per_seg = 0; //'-s' specify the APs to be averaged per plot-segment
    bool test_mode = false; //'-t' if true, then no output is written
    bool update_mode = false; //'-u' not yet enabled
    std::string polprod = ""; //'-P' polarization product argument (e.g XX or I or RR+LL)
    std::string reftime = ""; //'-T' specify the fourfit reference time - not yet enabled
    bool xwindows; //'-x' same as option '-p' we no long use pgplot/xwindows
    bool xpower_output = false; //-X export xpower spectrum
    bool use_mk4_output = false;
    std::string input;

    std::vector< std::string > msg_cats =
    {
        "main", "calibration", "containers", "control",
        "fringe", "file", "initialization", "mk4interface",
        "utilities", "vex", "python_bindings"
    };

    std::stringstream ss;
    ss << "limit the allowed message categories to only those which the user specifies, the available categories are: \n";
    for(auto it = msg_cats.begin(); it != msg_cats.end(); it++)
    {
        ss << "    "<< *it <<"\n";
    }
    ss <<"if the '-M' option is not used, the default is to allow all categories. ";
    std::string msg_cat_help = ss.str();


    CLI::App app{"fourfit"};

    // Remove help flag because it shortcuts all processing
    app.set_help_flag();

    // Add custom flag that activates help
    auto *help = app.add_flag("-h,--help", "print this help message and exit");
    app.add_flag("-a,--accounting", accounting, "perform run-time accounting/profiling");
    app.add_option("-b,--baseline", baseline_opt, "baseline or baseline:frequency_group selection (e.g GE or GE:X)");
    app.add_option("-c,--control-file", control_file, "specify the control file");
    app.add_flag("-e,--estimate", estimate_time, "estimate run time (ignored, not yet implemented)");
    app.add_option("-f,--first-plot-channel", first_plot_chan, "specifies the first channel displayed in the fringe plot");
    app.add_option("-M,--message-categories", message_categories, msg_cat_help.c_str() )->delimiter(',');
    app.add_option("-m,--message-level", message_level, "message level to be used, range: -2 (debug) to 5 (silent)");
    app.add_option("-n,--nplot-channels", nplot_chans, "specifies the number of channels to display in the fringe plot");
    app.add_flag("-p,--plot", show_plot, "generate and shows fringe plot on completion");
    app.add_option("-r,--refringe-alist", refringe_alist_file, "alist file for refringing (ignored, not yet implemented)");
    app.add_option("-s,--ap-per-segment", ap_per_seg, "specify the APs to be averaged per plot-segment");
    app.add_flag("-t,--test-mode", test_mode, "if passed, then no output is written");
    app.add_flag("-u,--update-mode", update_mode, "(ignored, not yet implemented)");
    app.add_option("-P,--polprod", polprod, "polarization product argument (e.g XX or I or RR+LL)");
    app.add_option("-T,--reftime", reftime, "specify the fourfit reference time (ignored, not yet implemented)");
    app.add_flag("-x,--xwindows", xwindows, "display plot using xwindows (ignored, not yet implemented)");
    app.add_flag("-X,--xpower-output", xpower_output, "output spectral cross power data (visibilities with corrections/residual fringe solution applied)");
    app.add_option("input,-i,--input", input, "name of the input directory (scan) or root file")->required();
    app.add_flag("-k,--mark4-output", use_mk4_output, "write output files in mark4 type_2xx format");

    //add the 'set' command for control file parameter overrides
    auto *setcom = app.add_subcommand("set", "pass control file parameters and related syntax on the command line")->prefix_command();
    //setcom->alias("--set");

    try
    {
        app.parse(argc, argv);
        if(*help)
        {
            throw CLI::CallForHelp();
        }
    }
    catch(const CLI::Error &e)
    {
        std::cout << app.help() << std::endl;
        std::exit(1); //just exit don't bother returning to main
    }

    //grab the set string if present
    std::string set_string = " ";
    std::vector< std::string > set_tokens;
    for(const auto &aarg : setcom->remaining())
    {
        set_tokens.push_back(aarg);
        set_string += aarg + " ";
    }

    //clamp message level
    if(message_level > 5){message_level = 5;}
    if(message_level < -2){message_level = -2;}
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    //check if any message categories were passed, if so, we limit the messages
    //to only those categories
    if(message_categories.size() != 0)
    {
        for(std::size_t m=0; m<message_categories.size(); m++)
        {
            MHO_Message::GetInstance().AddKey(message_categories[m]);
        }
        MHO_Message::GetInstance().LimitToKeySet();
    }

    //enable profiling if passed '-a'
    if(accounting){ MHO_Profiler::GetInstance().Enable();}

    //catch no input case
    if(input == ""){msg_fatal("main", "input directory/root file not set" << eom); std::exit(1);}

    //catch no control file case and set to /dev/null
    //TODO detect DEF_CONTROL environmental variable if present and use that
    if(control_file == ""){control_file = "/dev/null";}

    //for now we require these options to be set (may relax this once we allow mult-pass fringe fitting)
    if(baseline_opt != "")
    {
        MHO_BasicFringeDataConfiguration::parse_baseline_freqgrp(baseline_opt, baseline, freqgrp);
    }

    //set the message level
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    std::string directory = MHO_BasicFringeDataConfiguration::sanitize_directory(input);
    std::string root_file = MHO_BasicFringeDataConfiguration::find_associated_root_file(input);

    //pass the extracted command line info back in the parameter store
    paramStore->Set("/cmdline/accounting", accounting);
    paramStore->Set("/cmdline/baseline", baseline);
    paramStore->Set("/cmdline/frequency_group", freqgrp);

    paramStore->Set("/cmdline/control_file",control_file);
    paramStore->Set("/cmdline/directory", directory); //un-modified directory path
    paramStore->Set("/cmdline/root_file", root_file); //fully resolve (symlink free path to the root file)

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
    paramStore->Set("/cmdline/xpower_output", xpower_output);
    paramStore->Set("/cmdline/set_string", set_string); //TODO
    paramStore->Set("/cmdline/mk4format_output", use_mk4_output);

    int status = MHO_BasicFringeDataConfiguration::sanity_check(paramStore);

    return status; //0 is ok, anything else is an error
}



void DetermineScans(MHO_ParameterStore& param, std::vector< std::string >& scans)
{

}



void DetermineBaselines(MHO_ParameterStore& param, std::vector< std::string >& baselines)
{

}

void DetermineFGroupsAndPolProducts(MHO_ParameterStore& param, std::vector< std::string >& fgroups, std::vector< std::string >& pprods )
{

}


int main(int argc, char** argv)
{
    profiler_start();

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().SetExecutableName(std::string("fourfit"));

    MHO_ParameterStore basic_param_store;
    int parse_status = parse_command_line(argc, argv, &basic_param_store );
    if(parse_status != 0){msg_fatal("main", "could not parse command line options." << eom); std::exit(1);}

    //data loop order is scans, then baselines, then fgroups, then pol-producs
    std::vector< std::string > scans;
    std::vector< std::string > baselines;
    std::vector< std::string > fgroups;
    std::vector< std::string > polproducts;

    DetermineScans(basic_param_store, scans);

    for(auto sc = scans.begin(); sc != scans.end(); sc++)
    {
        MHO_FringeData fringeData;

        //populate a few necessary parameters and  initialize the scan data store
        MHO_BasicFringeDataConfiguration::populate_initial_parameters(fringeData.GetParameterStore(), fringeData.GetScanDataStore());
        DetermineBaselines(basic_param_store, baselines);

        //loop over all baselines
        for(auto bl = baselines.begin(); bl != baselines.end(); bl++)
        {
            DetermineFGroupsAndPolProducts(basic_param_store, fgroups, polproducts);

            for(auto fg = fgroups.begin(); fg != fgroups.end(); fg++)
            {
                //loop over all pol-products
                for(auto pprod = polproducts.begin(); pprod != polproducts.end(); pprod++)
                {


                    //parse the control file and form the control statements
                    MHO_FringeControlInitialization::process_control_file(fringeData.GetParameterStore(), fringeData.GetControlFormat(), fringeData.GetControlStatements() );

                    bool do_ion = false;
                    fringeData.GetParameterStore()->Get("/config/do_ion", do_ion);

                    MHO_FringeFitter* ffit;
                    //TODO FIXME...replace this logic with a factory method based on the
                    //contents of the control and parameter store
                    //but for the time being we only have two choices
                    if(do_ion){ ffit = new MHO_IonosphericFringeFitter(&fringeData);}
                    else{ ffit = new MHO_BasicFringeFitter(&fringeData);}

                    #ifdef USE_PYBIND11
                    // start the interpreter and keep it alive, need this or we segfault
                    py::scoped_interpreter guard{};
                    configure_pypath();

                    #endif

                    ffit->Configure();

                    ////////////////////////////////////////////////////////////////////////////
                    //POST-CONFIGURE FOR COMPILE-TIME EXTENSIONS -- this should be reorganized with visitor pattern
                    ////////////////////////////////////////////////////////////////////////////
                    #ifdef USE_PYBIND11
                    #pragma message("TODO FIXME -- formalize the means by which plugin dependent operator builders are added to the configuration")
                    ffit->GetOperatorBuildManager()->AddBuilderType<MHO_PythonOperatorBuilder>("python_labeling", "python_labeling");
                    ffit->GetOperatorBuildManager()->AddBuilderType<MHO_PythonOperatorBuilder>("python_flagging", "python_flagging");
                    ffit->GetOperatorBuildManager()->AddBuilderType<MHO_PythonOperatorBuilder>("python_calibration", "python_calibration");
                    #endif


                    //initialize and perform run loop
                    ffit->Initialize();
                    while( !ffit->IsFinished() )
                    {
                        ffit->PreRun();
                        ffit->Run();
                        ffit->PostRun();
                    }

                    ffit->Finalize();

                    ////////////////////////////////////////////////////////////////////////////
                    //OUTPUT/PLOTTING -- this should be reorganized with visitor pattern
                    ////////////////////////////////////////////////////////////////////////////
                    bool test_mode = fringeData.GetParameterStore()->GetAs<bool>("/cmdline/test_mode");
                    bool show_plot = fringeData.GetParameterStore()->GetAs<bool>("/cmdline/show_plot");


                    mho_json plot_data = fringeData.GetPlotData();

                    profiler_stop();
                    std::vector< MHO_ProfileEvent > events;
                    MHO_Profiler::GetInstance().GetEvents(events);
                    //convert and dump the events into the parameter store for now (will be empty unless enabled)
                    mho_json event_list = MHO_BasicFringeDataConfiguration::ConvertProfileEvents(events);
                    fringeData.GetParameterStore()->Set("/profile/events", event_list);

                    //open and dump to file -- should we profile this as well?
                    if(!test_mode)
                    {
                        bool use_mk4_output = false;
                        fringeData.GetParameterStore()->Get("/cmdline/mk4format_output", use_mk4_output);

                        if(!use_mk4_output)
                        {
                            fringeData.WriteOutput();
                        }
                        else
                        {
                            MHO_MK4FringeExport fexporter;
                            fexporter.SetParameterStore(fringeData.GetParameterStore());
                            fexporter.SetPlotData(plot_data);
                            fexporter.SetContainerStore(fringeData.GetContainerStore());
                            fexporter.ExportFringeFile();
                        }
                    }



                    #ifdef USE_PYBIND11
                    if(show_plot)
                    {
                        msg_debug("main", "python plot generation enabled." << eom );
                        py::dict plot_obj = plot_data;

                        // //QUICK HACK FOR PCPHASES UNTIL WE GET est_pc_maual working/////////////
                        // try
                        // {
                        //     auto mod = py::module::import("mho_test3");
                        //     mod.attr("generate_pcphases")(plot_obj);
                        // }
                        // catch(py::error_already_set &excep)
                        // {
                        //     msg_error("python_bindings", "python exception when calling subroutine (" << "mho_test3"<< "," << "generate_pcphases" << ")" << eom );
                        //     msg_error("python_bindings", "python error message: "<< excep.what() << eom);
                        //     PyErr_Clear(); //clear the error and attempt to continue
                        // }

                        ////////////////////////////////////////////////////////////////////////

                        //load our interface module -- this is extremely slow!
                        auto vis_module = py::module::import("hops_visualization");
                        auto plot_lib = vis_module.attr("fourfit_plot");
                        //call a python function on the interface class instance
                        //TODO, pass filename to save plot if needed
                        plot_lib.attr("make_fourfit_plot")(plot_obj, true, "");


                    }
                    #else //USE_PYBIND11
                    if(show_plot)
                    {
                        msg_warn("main", "plot output requested, but not enabled since HOPS was built without pybind11 support, ignoring." << eom);
                    }
                    #endif

                    //clean up
                    delete ffit;

                }//end of pol-product loop

            }//end of frequency group loop

        }//end of baseline loop

    }//end of scan loop

    return 0;
}
