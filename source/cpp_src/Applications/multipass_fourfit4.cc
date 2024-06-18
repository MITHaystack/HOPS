#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
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

void DetermineScans(const std::string& initial_dir, std::vector< std::string >& scans)
{
    scans.clear();
    scans.push_back(initial_dir);

    //TODO FIXME...for now we only treat the single directory case
    //if no root file was located, then we might be running over a whole experiment directory
    //we need to loop over all the sub-dirs and determine if they are scans directories
}

void DetermineBaselines(const std::string& dir, const std::string& baseline, std::vector< std::pair< std::string, std::string > >& baseline_files)
{
    baseline_files.clear();
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
        std::string bname = MHO_DirectoryInterface::GetBasename(corFiles[i]);
        tokenizer.SetString( &bname );
        std::vector< std::string > tok;
        tokenizer.GetTokens(&tok);
        if(tok.size() == 3)
        {
            std::cout<<"tok[0] = "<<tok[0]<<std::endl;
            if(tok[0].size() == 2)
            {
                std::cout<<"got a baseline: "<<tok[0]<<" in "<<corFiles[i]<<std::endl;
                std::string bl = tok[0];
                bool keep = false;
                if(baseline == "??"){keep = true;}
                if(baseline[0]  == '?' && baseline[1] == bl[1]){keep = true;}
                if(baseline[1] == '?' && baseline[0] == bl[0]){keep = true;}
                if(baseline == bl){keep = true;}
                if(keep)
                {
                    std::cout<<"keeping a baseline: "<<bl<<" in "<<corFiles[i]<<std::endl;
                    baseline_files.push_back( std::make_pair(bl, corFiles[i]) );
                }
            }
        }
    }
}

void DetermineFGroupsAndPolProducts(const std::string& filename, 
                                    const std::string& cmd_fgroup, 
                                    const std::string& cmd_pprod,
                                    std::vector< std::string >& fgroups, 
                                    std::vector< std::string >& pprods )
{
    fgroups.clear();
    pprods.clear();
    
    std::cout<<"looking at: "<<filename<<std::endl;

    //get uuid for MHO_ObjectTags object
    MHO_ContainerDictionary cdict;
    MHO_UUID tag_uuid = cdict.GetUUIDFor<MHO_ObjectTags>();

    //pull all the keys and byte offsets for each object
    std::vector< MHO_FileKey > ikeys;
    std::vector< std::size_t > byte_offsets;
    MHO_BinaryFileInterface inter;
    inter.ExtractFileObjectKeysAndOffsets(filename, ikeys, byte_offsets);

    //loop over keys and offsets, looking for tags offset
    bool found = false;
    std::size_t offset_bytes = 0;
    for(std::size_t i=0; i<ikeys.size(); i++)
    {
        if(ikeys[i].fTypeId == tag_uuid)
        {
            offset_bytes = byte_offsets[i];
            found = true;
            std::cout<<"found the tag object"<<std::endl;
            break; //only first tag object is used
        }
    }

    std::vector< std::string > tmp_fgroups;
    std::vector< std::string > tmp_pprods;
    if(found)
    {
        inter.OpenToReadAtOffset(filename, offset_bytes);
        MHO_ObjectTags obj;
        MHO_FileKey obj_key;
        //we read the tags object
        //now pull the pol-products and frequency groups info
        //and check them agains the command line arguments
        bool ok = inter.Read(obj, obj_key);
        
        std::cout<<"read the object"<<std::endl;
        // std::cout<< obj.GetMetaDataAsJSON().dump(2)<<std::endl;
        
        if(ok)
        {
            if( obj.IsTagPresent("polarization_product_set") )
            {
                obj.GetTagValue("polarization_product_set", tmp_pprods);
                std::cout<<"size of pol prod set: "<<tmp_pprods.size()<<std::endl;
                for(std::size_t i=0; i<tmp_pprods.size(); i++)
                {
                    std::cout<<"got a pprod: "<<tmp_pprods[i]<<std::endl;
                    //only if pol-product is unspecified, will we do them all
                    //otherwise only the specified pol-product will be used
                    bool keep = false;
                    if(cmd_pprod == "??"){keep = true;}
                    if(keep)
                    {
                        pprods.push_back(tmp_pprods[i]);
                        std::cout<<"keeping a pprod: "<<tmp_pprods[i]<<std::endl;
                    }
                }
            }
            else 
            {
                msg_error("main", "no polarization_product_set present in MHO_ObjectTags" << eom);
            }

            if( obj.IsTagPresent("frequency_band_set") )
            {
                obj.GetTagValue("frequency_band_set", tmp_fgroups);
                std::cout<<"size of freq band set: "<<tmp_fgroups.size()<<std::endl;
                for(std::size_t i=0; i<tmp_fgroups.size(); i++)
                {
                    std::cout<<"got a fgroup: "<<tmp_fgroups[i]<<std::endl;
                    //if fgroup is unspecified we do them all
                    //otherwise we only keep the one selected
                    bool keep = false;
                    if(cmd_fgroup == "?"){keep = true;}
                    if(cmd_fgroup == tmp_fgroups[i]){keep = true;}
                    if(keep)
                    {
                        fgroups.push_back(tmp_fgroups[i]);
                        std::cout<<"keeping a fgroup: "<<tmp_fgroups[i]<<std::endl;
                    }
                }
            }
            else 
            {
                msg_error("main", "no frequency_band_set present in MHO_ObjectTags" << eom);
            }
        }
        else 
        {
            msg_error("main", "could not determine polarization products or frequency bands from: "<< filename << eom);
        }
        inter.Close();
    }
    else 
    {
        msg_error("main", "no MHO_ObjectTags object found in file: "<< filename << eom);
    }

    //if a pol-product (or a linear combination was specified)
    //make sure it gets through here 
    if(cmd_pprod != "??")
    {
        pprods.clear();
        pprods.push_back(cmd_pprod);
    }
}


int main(int argc, char** argv)
{
    profiler_start();

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().SetExecutableName(std::string("fourfit"));

    MHO_ParameterStore cmdline_params;
    int parse_status = MHO_BasicFringeDataConfiguration::parse_fourfit_command_line(argc, argv, &cmdline_params );
    if(parse_status != 0){msg_fatal("main", "could not parse command line options." << eom); std::exit(1);}

    //data-loop order is scans, then baselines, then fgroups, then pol-products
    std::vector< std::string > scans; //list of scan directories
    std::vector< std::pair< std::string, std::string > > baseline_files;
    std::vector< std::string > fgroups;
    std::vector< std::string > polproducts;

    //determine which directories contain scans to process
    std::string initial_dir = cmdline_params.GetAs<std::string>("/cmdline/directory");
    DetermineScans(initial_dir, scans);
    cmdline_params.Dump(); //TODO REMOVE

    for(auto sc = scans.begin(); sc != scans.end(); sc++)
    {
        std::string scan_dir = *sc;
        std::cout<<"scan directory = "<<scan_dir<<std::endl;
        std::string root_file = MHO_BasicFringeDataConfiguration::find_associated_root_file(scan_dir);
        std::cout<<"root file = "<<root_file<<std::endl;
        if(root_file == ""){continue;} //TODO FIXME get rid of this!!

        std::string cmd_bl = cmdline_params.GetAs<std::string>("/cmdline/baseline"); //if not passed, will be "??"
        DetermineBaselines(scan_dir, cmd_bl, baseline_files);

        //loop over all baselines
        for(auto bl = baseline_files.begin(); bl != baseline_files.end(); bl++)
        {

            std::string baseline = bl->first;
            std::string corFile = bl->second;

            std::cout<<"working on baseline: "<<baseline<<std::endl;
            std::string cmd_fg = cmdline_params.GetAs<std::string>("/cmdline/frequency_group"); //if not passed, this will be "?"
            std::string cmd_pp = cmdline_params.GetAs<std::string>("/cmdline/polprod"); //if not passed, this will be "??"
            DetermineFGroupsAndPolProducts(corFile, cmd_fg, cmd_pp, fgroups, polproducts);

            for(auto fg = fgroups.begin(); fg != fgroups.end(); fg++)
            {
                std::string fgroup = *fg;
                //loop over all pol-products
                for(auto pprod = polproducts.begin(); pprod != polproducts.end(); pprod++)
                {
                    std::string polprod = *pprod;
                    
                    //populate a few necessary parameters and  initialize the fringe/scan data
                    MHO_FringeData fringeData;
                    fringeData.GetParameterStore()->CopyFrom(cmdline_params); //copy in command line info
                    //set the current pass info (directory, root_file, source, baseline, pol-product, frequency-group)
                    fringeData.GetParameterStore()->Set("/pass/directory", scan_dir);
                    fringeData.GetParameterStore()->Set("/pass/root_file", root_file);
                    fringeData.GetParameterStore()->Set("/pass/baseline", baseline);
                    fringeData.GetParameterStore()->Set("/pass/polprod", polprod);
                    fringeData.GetParameterStore()->Set("/pass/frequency_group", fgroup);
                    //initializes the scan data store, reads the ovex file and sets the value of '/pass/source'
                    bool scan_dir_ok = MHO_BasicFringeDataConfiguration::initialize_scan_data(fringeData.GetParameterStore(), fringeData.GetScanDataStore());
                    
                    MHO_BasicFringeDataConfiguration::populate_initial_parameters(fringeData.GetParameterStore(), fringeData.GetScanDataStore());

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
