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


void DetermineScans(MHO_ParameterStore& param, std::vector< std::string >& scans)
{
    scans.clear();
    std::string initial_dir = param.GetAs<std::string>("directory");
    scans.push_back(initial_dir);

    //TODO FIXME...for now we only treat the single directory case
    //if no root file was located, then we might be running over a whole experiment directory
    //we need to loop over all the sub-dirs and determine if they are scans directories
}

void DetermineBaselines(std::string dir, std::vector< std::string >& baselines)
{
    baselines.clear();
    std::vector< std::string > corFiles;
    MHO_DirectoryInterface dirInterface;
    dirInterface.SetCurrentDirectory(dir);
    dirInterface.ReadCurrentDirectory();
    dirInterface.GetFilesMatchingExtention(corFiles, "cor");

    //loop over 'cor' files and extract the 2-character baseline code
    //TODO...eventually we want to eliminate the need to single-char station codes
    //(so that things like 'Gs-Wf.ABCDEF.cor' are also possible)
    MHO_Tokenizer tokenizer;
    tokenizer.SetDelimiter(".");
    tokenizer.SetIncludeEmptyTokensFalse();
    for(std::size_t i=0; i<corFiles.size(); i++)
    {
        tokenizer.SetString( &(corFiles[i]) );
        std::vector< std::string > tok;
        tokenizer.GetTokens(&tok);
        if(tok.size() == 3)
        {
            if(tok[0].size() == 2)
            {
                std::cout<<"got a baseline: "<<tok[0]<<std::endl;
                baselines.push_back(tok[0]);
            }
        }
    }
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

    MHO_ParameterStore initial_param_store;
    int parse_status = MHO_BasicFringeDataConfiguration::parse_fourfit_command_line(argc, argv, &initial_param_store );
    if(parse_status != 0){msg_fatal("main", "could not parse command line options." << eom); std::exit(1);}

    //data loop order is scans, then baselines, then fgroups, then pol-producs
    std::vector< std::string > scans; //list of scan directories
    std::vector< std::string > baselines;
    std::vector< std::string > fgroups;
    std::vector< std::string > polproducts;

    DetermineScans(initial_param_store, scans);
    initial_param_store.Dump(); //TODO REMOVE

    for(auto sc = scans.begin(); sc != scans.end(); sc++)
    {
        std::string scan_dir = *sc;
        std::string root_file = MHO_BasicFringeDataConfiguration::find_associated_root_file(scan_dir);
        if(root_file == ""){continue;} //TODO FIXME get rid of this!!

        MHO_FringeData fringeData;
        fringeData.GetParameterStore()->CopyFrom(initial_param_store); //copy in command line info
        fringeData.GetParameterStore()->Set("directory", scan_dir); //point to current scan
        fringeData.GetParameterStore()->Set("root_file", root_file); //point to current root file

        //populate a few necessary parameters and  initialize the scan data store
        MHO_BasicFringeDataConfiguration::initialize_scan_data(fringeData.GetParameterStore(), fringeData.GetScanDataStore());
        MHO_BasicFringeDataConfiguration::populate_initial_parameters(fringeData.GetParameterStore(), fringeData.GetScanDataStore());
        DetermineBaselines(scan_dir, baselines);

        //loop over all baselines
        for(auto bl = baselines.begin(); bl != baselines.end(); bl++)
        {
            if( !(fringeData.GetScanDataStore()->IsBaselinePresent(*bl) ) )
            {
                msg_error("fringe", "cannot find the specified baseline: " << *bl << " in " << scan_dir << eom);
                continue;
            }

            DetermineFGroupsAndPolProducts(initial_param_store, fgroups, polproducts);

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
