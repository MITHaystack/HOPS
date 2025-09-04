#include <fstream>
#include <getopt.h>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

//option parsing and help text library
#include "CLI11.hpp"

#include "MHO_BasicFringeDataConfiguration.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_DirectoryInterface.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_Tokenizer.hh"
#include "MHO_FringePlotVisitorFactory.hh"

//pybind11 stuff to interface with python
#ifdef USE_PYBIND11
    #include "pybind11_json/pybind11_json.hpp"
    #include <pybind11/embed.h>
    #include <pybind11/pybind11.h>
namespace py = pybind11;
namespace nl = nlohmann;
using namespace pybind11::literals;
    #include "MHO_DefaultPythonPlotVisitor.hh"
    #include "MHO_PyConfigurePath.hh"
    #include "MHO_PyFringeDataInterface.hh"
    #include "MHO_PythonOperatorBuilder.hh"
#endif

using namespace hops;

bool matches_extension(const std::string& filename, const std::string& anExt)
{
    //from the current list of files, locate the ones which match the given extension
    std::string basename = MHO_DirectoryInterface::GetBasename(filename);
    std::size_t index = basename.find(anExt);
    if(index != std::string::npos)
    {
        //make sure the extension is the very end of the string
        std::string sub = basename.substr(index);
        if(sub == anExt)
        {
            return true;
        }
    }
    return false;
}

bool match_baseline(const std::string& baseline, const std::string& obj_baseline)
{
    if(baseline == "??")
    {
        return true;
    }
    if(baseline[0] == '?' && baseline[1] == obj_baseline[1])
    {
        return true;
    }
    if(baseline[1] == '?' && baseline[0] == obj_baseline[0])
    {
        return true;
    }
    return baseline == obj_baseline;
}

bool match_fgroup(const std::string& fgroup, const std::string& obj_fgroup)
{
    if(fgroup == "?")
    {
        return true;
    }
    return fgroup == obj_fgroup;
}

bool match_polprod(const std::string& polprod, const std::string& obj_polprod)
{
    if(polprod == "??")
    {
        return true;
    }
    return polprod == obj_polprod;
}

int parse_fplot_command_line(int argc, char** argv, MHO_ParameterStore* paramStore)
{
    //store the raw arguments in the parameter store
    std::vector< std::string > arglist;
    for(int i = 0; i < argc; i++)
    {
        arglist.push_back(std::string(argv[i]));
    }
    paramStore->Set("/cmdline/args", arglist);

    //command line parameters
    std::string disk_file = "";
    std::string baseline_opt = "??:?";             //'-b' baseline:frequency_group selection
    std::string baseline = "??";                   // the baseline
    std::string freqgrp = "?";                     // the frequency group
    int message_level = -1;                        //'-m' specifies the message verbosity level
    std::vector< std::string > message_categories; // -'M' limits the allowed message categories to those the user specifies
    bool no_plot = false;                          //'-n' disable display of fringe plot (default is to show it)
    std::string polprod = "??";                    //'-P' polarization product argument (e.g XX or I or RR+LL)
    std::vector< std::string > input;              //either directory, individual file, or list of files
    std::vector< std::string > fringe_file_list;
    std::string backend = "gnuplot";

    std::vector< std::string > msg_cats = {"main",           "calibration",  "containers", "control", "fringe",         "file",
                                           "initialization", "mk4interface", "utilities",  "vex",     "python_bindings"};

    std::stringstream ss;
    ss << "limit the allowed message categories to only those which the user specifies, the available categories are: \n";
    for(auto it = msg_cats.begin(); it != msg_cats.end(); it++)
    {
        ss << "    " << *it << "\n";
    }
    ss << "if the '-M' option is not used, the default is to allow all categories. ";
    std::string msg_cat_help = ss.str();

    CLI::App app{"fplot"};

    // Remove help flag because it shortcuts all processing
    app.set_help_flag();

    // Add custom flag that activates help
    auto* help = app.add_flag("-h,--help", "print this help message and exit");
    app.add_option("-d,--disk_file", disk_file, "name of the file in which to save the fringe plot");
    app.add_option("-b,--baseline", baseline_opt, "baseline or baseline:frequency_group selection (e.g GE or GE:X)");
    app.add_option("-M,--message-categories", message_categories, msg_cat_help.c_str())->delimiter(',');
    app.add_option("-m,--message-level", message_level, "message level to be used, range: -2 (debug) to 5 (silent)");
    app.add_option("-B,--backend", backend, "plotting backend to be used (gnuplot (default) or matplotlib)");
    app.add_flag("-n,--no-plot", no_plot, "do not display the fringe plot");
    app.add_option("-P,--polprod", polprod, "plot only files matching this polarization product (e.g XX or I or RR+LL)");
    app.add_option("input,-i,--input", input, "name of the input directory, fringe file, or list of fringe files")->required();

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
    //set the message level
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    //catch no input case
    if(input.size() == 0)
    {
        msg_fatal("main", "input directory/fringe file not set" << eom);
        std::exit(1);
    }
    std::size_t n_input = input.size();
    if(n_input == 1)
    {
        //either we have been passed a single fringe file or a directory
        if(matches_extension(input[0], "frng"))
        {
            fringe_file_list.push_back(input[0]);
        }
        else
        {
            // //assume this is a directory
            MHO_DirectoryInterface dirInterface;
            dirInterface.SetCurrentDirectory(input[0]);
            dirInterface.ReadCurrentDirectory();
            //get file list in this directory
            std::vector< std::string > flist;
            dirInterface.GetFilesMatchingExtention(flist, "frng");
            if(flist.size() == 0)
            {
                //no fringe files here, so recurse 1-level (only)
                //and collect all fringe files found
                std::vector< std::string > sdlist;
                dirInterface.GetSubDirectoryList(sdlist);
                for(std::size_t i = 0; i < sdlist.size(); i++)
                {
                    dirInterface.SetCurrentDirectory(sdlist[i]);
                    dirInterface.ReadCurrentDirectory();
                    std::vector< std::string > tmp_flist;
                    dirInterface.GetFilesMatchingExtention(tmp_flist, "frng");
                    flist.insert(flist.end(), tmp_flist.begin(), tmp_flist.end());
                }
            }
            fringe_file_list = flist;
        }
    }
    else
    {
        //we have a list of fringe files, just copy them
        for(std::size_t i = 0; i < n_input; i++)
        {
            if(matches_extension(input[i], "frng"))
            {
                fringe_file_list.push_back(input[i]);
            }
        }
    }

    //for now we require these options to be set (may relax this once we allow mult-pass fringe fitting)
    MHO_BasicFringeDataConfiguration::parse_baseline_freqgrp(baseline_opt, baseline, freqgrp);

    //pass the extracted command line info back in the parameter store
    bool show_plot = !no_plot;
    paramStore->Set("/cmdline/baseline", baseline);
    paramStore->Set("/cmdline/frequency_group", freqgrp);
    paramStore->Set("/cmdline/polprod", polprod);
    paramStore->Set("/cmdline/disk_file", disk_file);
    paramStore->Set("/cmdline/message_level", message_level);
    paramStore->Set("/cmdline/show_plot", show_plot);
    paramStore->Set("/cmdline/fringe_files", fringe_file_list);
    paramStore->Set("/cmdline/backend", backend);

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool extract_plot_data(mho_json& plot_data, mho_json& param_data, std::string filename)
{
    //split the filename (not strictly necessary, but used to extract the extent number)
    //for example: 23 in 'GE.X.XX.345F47.23.frng'
    // MHO_Tokenizer tokenizer;
    // tokenizer.SetDelimiter(".");
    // tokenizer.SetIncludeEmptyTokensFalse();
    // tokenizer.SetString(&filename);
    // std::vector< std::string > tokens;
    // tokenizer.GetTokens(&tokens);
    if(filename.find("frng") == std::string::npos)
    {
        //not a fringe file, skip this
        msg_error("fringe", "could not parse the file name: " << filename << eom);
        return false;
    }

    //to pull out fringe data, we are primarily interested in the 'MHO_ObjectTags' object
    //get uuid for MHO_ObjectTags object
    MHO_ContainerDictionary cdict;
    MHO_UUID tag_uuid = cdict.GetUUIDFor< MHO_ObjectTags >();

    //pull all the keys and byte offsets for each object
    std::vector< MHO_FileKey > ikeys;
    std::vector< std::size_t > byte_offsets;
    MHO_BinaryFileInterface inter;
    inter.ExtractFileObjectKeysAndOffsets(filename, ikeys, byte_offsets);

    //loop over keys and offsets, looking for tags offset
    bool found = false;
    std::size_t offset_bytes = 0;
    for(std::size_t i = 0; i < ikeys.size(); i++)
    {
        if(ikeys[i].fTypeId == tag_uuid)
        {
            offset_bytes = byte_offsets[i];
            found = true;
            break; //only first tag object is used
        }
    }

    if(found)
    {
        inter.OpenToReadAtOffset(filename, offset_bytes);
        MHO_ObjectTags obj;
        MHO_FileKey obj_key;
        //we read the tags object
        bool ok = inter.Read(obj, obj_key);
        if(ok)
        {
            //pull the plot data
            bool plot_ok = obj.GetTagValue("plot_data", plot_data);
            bool param_ok = obj.GetTagValue("parameters", param_data);
            inter.Close();
            if(plot_ok && param_ok)
            {
                return true;
            }
            return false;
        }
        else
        {
            msg_error("fringe", "could not read MHO_ObjectTags from: " << filename << eom);
            inter.Close();
            return false;
        }
    }
    else
    {
        msg_error("fringe", "no MHO_ObjectTags object found in file: " << filename << eom);
        return false;
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

#ifdef USE_PYBIND11
    //start the interpreter and keep it alive, need this or we segfault
    //each process has its own interpreter
    py::scoped_interpreter guard{};
    configure_pypath();
#endif 

    MHO_ParameterStore paramStore;
    parse_fplot_command_line(argc, argv, &paramStore);

    //determine if we are saving/showing the plot
    std::string disk_file = paramStore.GetAs< std::string >("/cmdline/disk_file");
    bool show_plot = paramStore.GetAs< bool >("/cmdline/show_plot");

    std::vector< std::string > ffiles = paramStore.GetAs< std::vector< std::string > >("/cmdline/fringe_files");
    std::size_t n_files = ffiles.size();

    for(std::size_t i = 0; i < n_files; i++)
    {
        std::string filename = ffiles[i];

        //read the plot data from the fringe file
        MHO_FringeData fdata;
        mho_json param_data;
        bool ok = extract_plot_data(fdata.GetPlotData(), param_data, filename);
        fdata.GetParameterStore()->FillData(param_data);


        //check if this file matches any of the selection criteria (if passed)
        std::string baseline = paramStore.GetAs< std::string >("/cmdline/baseline");
        std::string fgroup = paramStore.GetAs< std::string >("/cmdline/frequency_group");
        std::string polprod = paramStore.GetAs< std::string >("/cmdline/polprod");
        //call the plotting mechanism
        if(ok)
        {
            //msg_debug("main", "python plot generation enabled." << eom);
            //py::dict plot_obj = plot_data; //convert to dictionary object

            //grab the selection info
            std::string obj_baseline = fdata.GetParameterStore()->GetAs<std::string>("/pass/baseline"); 
            std::string obj_fgroup = fdata.GetParameterStore()->GetAs<std::string>("pass/frequency_group");
            std::string obj_polprod = fdata.GetParameterStore()->GetAs<std::string>("pass/polprod");

            if(match_baseline(baseline, obj_baseline) && match_fgroup(fgroup, obj_fgroup) &&
               match_polprod(polprod, obj_polprod))
            {
                
                //specify what plotting operation is needed in the parameter store
                fdata.GetParameterStore()->Set("/cmdline/disk_file", disk_file);
                fdata.GetParameterStore()->Set("/cmdline/show_plot", show_plot);
                
                //use the plotter factory to construct one of the available plotting backends
                //currently we only have two fringe plotting options (gnuplot or matplotlib)
                //default is gnuplot
                std::string plot_backend = paramStore.GetAs<std::string>("/cmdline/backend");
                msg_debug("main", "plotting backend is: "<< plot_backend << eom);
                //fringeData.GetParameterStore()->Get("/control/config/plot_backend", plot_backend);
                MHO_FringePlotVisitorFactory plotter_factory;
                MHO_FringePlotVisitor* plotter = plotter_factory.ConstructPlotter(plot_backend);
                if(plotter != nullptr)
                {
                    plotter->Plot(&fdata);
                }

                //plotter is deleted by the factory
                
                // 
                // 
                // #ifdef USE_PYBIND11
                // //load our interface module -- this is extremely slow!
                // auto vis_module = py::module::import("hops_visualization");
                // auto plot_lib = vis_module.attr("fourfit_plot");
                // ////////////////////////////////////////////////////////////////////////
                // //load our interface module -- this is extremely slow!
                // //TODO..allow for custom plot method to be called
                // try
                // {
                //     //required modules pyMHO_Containers and pyMHO_Operators are imported by configure_pypath()
                //     auto vis_module = py::module::import("hops_visualization");
                //     auto plot_lib = vis_module.attr("fourfit_plot");
                //     //call a python function on the interface class instance
                //     plot_lib.attr("make_fourfit_plot")(plot_obj, show_plot, disk_file);
                // }
                // catch(py::error_already_set& excep)
                // {
                //     if(std::string(excep.what()).find("SystemExit") != std::string::npos)
                //     {
                //         msg_debug("python_bindings", "sys.exit() called from within python, exiting" << eom);
                //         std::exit(0); //ok to exit program entirely
                //     }
                //     else
                //     {
                //         msg_error("python_bindings", "python exception when calling subroutine ("
                //                                          << "fourfit_plot"
                //                                          << ","
                //                                          << "make_fourfit_plot"
                //                                          << ")" << eom);
                //         msg_error("python_bindings", "python error message: " << excep.what() << eom);
                //         PyErr_Clear(); //clear the error and attempt to continue
                //     }
                // }
                // #else //USE_PYBIND11
                //     msg_warn("main", "fplot is not enabled since HOPS was built without pybind11 support." << eom);
                // #endif

            }
        }
    }



    return 0;
}
