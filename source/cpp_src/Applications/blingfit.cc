#include <fstream>
#include <getopt.h>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

//option parsing and help text library
#include "CLI11.hpp"

#include "MHO_Message.hh"
#include "MHO_Snapshot.hh"
#include "MHO_Timer.hh"

//fringe finding
#include "MHO_FringeFitter.hh"
#include "MHO_FringePlotVisitor.hh"
#include "MHO_FringeFitterFactory.hh"
#include "MHO_FringePlotVisitorFactory.hh"

//for control intialization
#include "MHO_BasicFringeDataConfiguration.hh"
#include "MHO_FringeControlInitialization.hh"

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

//needed to export to mark4 fringe files
#include "MHO_MK4FringeExport.hh"

//wraps the MPI interface (in case it is not enabled)
#include "MHO_MPIInterfaceWrapper.hh"

//set build timestamp, for fourfit plots (legacy behavior)
#ifdef HOPS_BUILD_TIME
    #define HOPS_BUILD_TIMESTAMP STRING(HOPS_BUILD_TIME)
#else
    //no build time defined...default
    #define HOPS_BUILD_TIMESTAMP "2000-01-01T00:00:00.0Z"
#endif


#include "MHO_LinearAlgebraUtilities.hh"


#include <cmath>
#include <cstddef>
#include <vector>

#include <string>
#include <limits>
#include <sstream>
#include <functional>

using namespace hops;

int main(int argc, char** argv)
{
    int process_id = 0;
    int local_id = 0;
    int n_processes = 1;

#ifdef HOPS_USE_MPI
    MHO_MPIInterface::GetInstance()->Initialize(&argc, &argv, true); //true -> run with no local even/odd split
    process_id = MHO_MPIInterface::GetInstance()->GetGlobalProcessID();
    local_id = MHO_MPIInterface::GetInstance()->GetLocalProcessID();
    n_processes = MHO_MPIInterface::GetInstance()->GetNProcesses();
#endif

#ifdef USE_PYBIND11
    //start the interpreter and keep it alive, need this or we segfault
    //each process has its own interpreter
    py::scoped_interpreter guard{};
    configure_pypath();
#endif

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().SetExecutableName(std::string("blingfit"));

    MHO_ParameterStore cmdline_params;
    int parse_status = MHO_BasicFringeDataConfiguration::parse_fourfit_command_line(argc, argv, &cmdline_params);
    if(parse_status != 0)
    {
        msg_fatal("main", "could not parse command line options." << eom);
        std::exit(1);
    }

    //flattened pass-info parameters (these are flattened into a single string primarily for MPI)
    std::string concat_delim = ",";
    std::string cscans, croots, cbaselines, cfgroups, cpolprods;

    MPI_SINGLE_PROCESS
    {
        msg_debug("main", "determining the data passes" << eom);
        MHO_BasicFringeDataConfiguration::determine_passes(&cmdline_params, cscans, croots, cbaselines, cfgroups, cpolprods);
        msg_debug("main", "done determining the data passes" << eom);
    }

//use MPI bcast to send all of the pass information to the worker processes
#ifdef HOPS_USE_MPI
    MHO_MPIInterface::GetInstance()->BroadcastString(cscans);
    MHO_MPIInterface::GetInstance()->BroadcastString(croots);
    MHO_MPIInterface::GetInstance()->BroadcastString(cbaselines);
    MHO_MPIInterface::GetInstance()->BroadcastString(cfgroups);
    MHO_MPIInterface::GetInstance()->BroadcastString(cpolprods);
#endif

    std::vector< mho_json > pass_vector;
    MHO_BasicFringeDataConfiguration::split_passes(pass_vector, cscans, croots, cbaselines, cfgroups, cpolprods);
    
    //construct the scan/directory set
    std::set< std::string > scan_set;
    for(std::size_t pass_index = 0; pass_index < pass_vector.size(); pass_index++)
    {
        mho_json pass = pass_vector[pass_index];
        std::string sdir = pass["directory"].get<std::string>();
        scan_set.insert(sdir);
    }

    std::size_t n_scans = scan_set.size();
    MPI_SINGLE_PROCESS
    {
        msg_info("main", "blingfit will fringe " << n_scans << " n_scans of data" << eom);
    }


    //loop over all scans
    for(auto scan_dir_it = scan_set.begin(); scan_dir_it != scan_set.end(); scan_dir_it++)
    {
        //collect all of the passes (baseline, fgroup, pol-prod) associated with a specific scan
        std::string scan_dir = *scan_dir_it;
        std::vector< mho_json > scan_pass_vector;
        for(std::size_t pass_index = 0; pass_index < pass_vector.size(); pass_index++)
        {
            mho_json pass = pass_vector[pass_index];
            std::string sdir = pass["directory"].get<std::string>();
            if(sdir == scan_dir)
            {
                scan_pass_vector.push_back(pass);
            }
        }

        msg_info("main", "scan " << scan_dir << " contains "<< scan_pass_vector.size()<< " passes of data" << eom);

        //keep track of all the stations encountered
        std::set< std::string > scanStations;

        //baseline quantities (map's into 'b' vector, for later Ax=b solve step)
        std::map< std::string, double > baseline_delay;
        std::map< std::string, double > baseline_snr;
        std::map< std::string, double > baseline_delay_rate;
        std::map< std::string, double > baseline_ion_diff;

        //this loop could be trivially parallelized (with the exception of plotting)
        for(std::size_t pass_index = 0; pass_index < scan_pass_vector.size(); pass_index++)
        {
            if(pass_index % n_processes == process_id) //only do work for this pass if it is assigned to this process
            {
                profiler_start();

                //populate a few necessary parameters and  initialize the fringe/scan data
                //set the current pass info (directory, root_file, source, baseline, pol-product, frequency-group)
                mho_json pass = scan_pass_vector[pass_index];
                pass["build_time"] = HOPS_BUILD_TIMESTAMP; //set the build time stamp in the pass info
        
                MHO_FringeData fringeData;
                fringeData.GetParameterStore()->CopyFrom(cmdline_params); //copy in command line info
                fringeData.GetParameterStore()->Set("/pass", pass);

                //initializes the scan data store, reads the ovex file and sets the value of '/pass/source'
                bool scan_dir_ok = MHO_BasicFringeDataConfiguration::initialize_scan_data(fringeData.GetParameterStore(),
                                                                                          fringeData.GetScanDataStore());
                MHO_BasicFringeDataConfiguration::populate_initial_parameters(fringeData.GetParameterStore(),
                                                                              fringeData.GetScanDataStore());

                //parse the control file and form the control statements
                MHO_FringeControlInitialization::process_control_file(fringeData.GetParameterStore(), 
                                                                      fringeData.GetControlFormat(),
                                                                      fringeData.GetControlStatements());
        
                //build the fringe fitter based on the input (only 2 choices currently -- basic and ionospheric)
                MHO_FringeFitterFactory ff_factory(&(fringeData));
                MHO_FringeFitter* ffit = ff_factory.ConstructFringeFitter(); //configuration is done here
        
                //initialize and perform run loop
                while(!ffit->IsFinished())
                {
                    ffit->Initialize();
                    ffit->PreRun();
                    ffit->Run();
                    ffit->PostRun();
                }
                ffit->Finalize();
                
                //keep track of the stations
                std::string ref_station;
                std::string rem_station;
                fringeData.GetParameterStore()->Get("/config/reference_station", ref_station);
                fringeData.GetParameterStore()->Get("/config/remote_station", rem_station);
                scanStations.insert(ref_station);
                scanStations.insert(rem_station);

                //construct the scan-pass-key from baseline-polprod-fgroup
                std::string pprod = pass["polprod"].get<std::string>();
                std::string fg = pass["frequency_group"].get<std::string>();
                //std::string rootcode = pass["root_file"].get<std::string>();
                std::string pkey = ref_station + "|" + rem_station  + "|" + pprod + "|" + fg; 

                //flush profile events
                profiler_stop();
                std::vector< MHO_ProfileEvent > events;
                MHO_Profiler::GetInstance().GetEvents(events);
                
                //convert and dump the events into the parameter store for now (will be empty unless enabled)
                mho_json event_list = MHO_BasicFringeDataConfiguration::ConvertProfileEvents(events);
                fringeData.GetParameterStore()->Set("/profile/events", event_list);
        
                //determine if this pass was skipped or is in test-mode
                bool is_skipped = fringeData.GetParameterStore()->GetAs< bool >("/status/skipped");
                bool test_mode = fringeData.GetParameterStore()->GetAs< bool >("/cmdline/test_mode");
                
                //open and dump to file -- should we profile this as well?
                if(!test_mode && !is_skipped)
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
                        fexporter.SetPlotData(fringeData.GetPlotData());
                        fexporter.SetContainerStore(fringeData.GetContainerStore());
                        fexporter.ExportFringeFile();
                    }
                }
                
                //use the plotter factory to construct one of the available plotting backends
                if(!is_skipped)
                {
                    //currently we only have two fringe plotting options (gnuplot or matplotlib)
                    std::string plot_backend;
                    fringeData.GetParameterStore()->Get("/control/config/plot_backend", plot_backend);
                    MHO_FringePlotVisitorFactory plotter_factory;
                    MHO_FringePlotVisitor* plotter = plotter_factory.ConstructPlotter(plot_backend);
                    if(plotter != nullptr)
                    {
                        ffit->Accept(plotter);
                    }
                }

                //extract our quanties of interest from the parameter store 
                double mbd;
                double drate;
                double snr;
                double dtec;
                fringeData.GetParameterStore()->Get("/fringe/mbdelay", mbd);
                fringeData.GetParameterStore()->Get("/fringe/drate", drate);
                fringeData.GetParameterStore()->Get("/fringe/snr", snr);
                fringeData.GetParameterStore()->Get("/fringe/ion_diff", dtec);
                std::cout<<"process_id: "<<process_id<<" pkey: "<<pkey<<", mbd: "<<mbd<<", drate: "<<drate<<" snr: "<<snr<<" dtec: "<<dtec<<std::endl;

                //stash these quantites in the map
                baseline_delay[pkey] = mbd;
                baseline_snr[pkey] = snr;
                baseline_delay_rate[pkey] = drate;
                baseline_ion_diff[pkey] = dtec;
            }
        } //end of pass loop


        //wait for every process to finish
        #ifdef HOPS_USE_MPI
        MHO_MPIInterface::GetInstance()->GlobalBarrier();
        #endif


        //now collect all map data across all processes
        auto bl_delays = baseline_delay;
        auto bl_drates = baseline_delay_rate;
        auto bl_ion = baseline_ion_diff;
        auto bl_snr = baseline_snr;
        auto station_set = scanStations;
        #ifdef HOPS_USE_MPI
        //merge the data from all processes so the root process can use it
        bl_delays = MHO_MPIInterface::GetInstance()->MergeMap(baseline_delay);
        bl_drates = MHO_MPIInterface::GetInstance()->MergeMap(baseline_delay_rate);
        bl_snr = MHO_MPIInterface::GetInstance()->MergeMap(baseline_snr);
        bl_ion = MHO_MPIInterface::GetInstance()->MergeMap(baseline_ion_diff);
        station_set = MHO_MPIInterface::GetInstance()->MergeStringSet(scanStations);
        #endif

        MPI_SINGLE_PROCESS
        {
            for(const auto& kv : bl_delays) 
            {
                std::cout<<"mbd: "<< kv.first <<", "<< kv.second <<std::endl;
            }

            for(const auto& kv : bl_drates) 
            {
                std::cout<<"delay rate: "<< kv.first <<", "<< kv.second <<std::endl;
            }

            for(const auto& kv : bl_snr) 
            {
                std::cout<<"snr: "<< kv.first <<", "<< kv.second <<std::endl;
            }

            for(const auto& kv : bl_ion) 
            {
                std::cout<<"dtec: "<< kv.first <<", "<< kv.second <<std::endl;
            }

            for(const auto& k : station_set) 
            {
                std::cout<<"station: "<< k <<std::endl;
            }

            //map stations to integers 
            std::map<std::string, int> station2int;
            std::map<int, std::string> int2station;
            int count = 0;
            for(auto sit = station_set.begin(); sit != station_set.end(); sit++)
            {
                std::string st = *sit;
                station2int[st] = count;
                int2station[count] = st;
                std::cout<<count<<": "<<st<<std::endl;
                count++;
            }
            int nstations = station_set.size();

            //construct the b-vector
            std::size_t n_baseline_parameters = bl_delays.size();// + baseline_delay_rate.size() + baseline_ion_diff.size();
            MHO_linalg_vector b(n_baseline_parameters);
            b.zero();

            //construct the x-vector
            std::size_t NQTY = 1;
            std::size_t n_station_parameters = NQTY*nstations;
            MHO_linalg_vector x(n_station_parameters);
            x.zero();
            
            //construct the network difference matrix 
            MHO_linalg_matrix A(n_baseline_parameters, n_station_parameters);
            A.zero();

            std::cout<<"fringe-fitted: "<<baseline_delay.size()<<std::endl;
            std::cout<<"nstations = "<<nstations<<std::endl;
            
            MHO_Tokenizer tokenizer;
            tokenizer.SetDelimiter("|");

            for(auto it = bl_delays.begin(); it != bl_delays.end(); it++)
            {
                //get the pass key, and map stations to indexes
                std::string pkey = it->first;
                std::vector< std::string > tokens;
                tokenizer.SetString(&pkey);
                tokenizer.GetTokens(&tokens);
                std::string ref_station = tokens[0];
                std::string rem_station = tokens[1];
                int ref_idx = station2int[ref_station];
                int rem_idx = station2int[rem_station];
            
                if(ref_idx != rem_idx)
                {
                    double mbd = bl_delays[pkey];
                    double drate = bl_drates[pkey];
                    double dtec = bl_ion[pkey];
                    double snr = bl_snr[pkey];
                    std::cout<<ref_station<<":"<<rem_station<<", "<<"pkey: "<<pkey<<", mbd: "<<mbd<<", drate: "<<drate<<" snr: "<<snr<<"dtec: "<<dtec<<std::endl;
            
                    if(snr > 15.0)
                    {
                        //set the output vector elements
                        b(count) = mbd;
                        // b(count+1) = drate;
                        // b(count+2) = dtec;
            
                        //set the station network difference matrix elements
                        A(count, ref_idx) = -1.0;
                        A(count, rem_idx) = 1.0;

                        // A(count, ref_idx) = -1.0;
                        // A(count, rem_idx) = 1.0;
                        // 
                        // A(count, ref_idx) = -1.0;
                        // A(count, rem_idx) = 1.0;
                    }
                }
                count++;
            }

            
            std::cout<<"xform mx = "<<std::endl;
            MHO_linalg_matrix_print(A);
            
            // this function uses the slower but more accurate one-sided jacobi svd
            // as defined in the paper:
            // Jacobi's method is more accurate than QR by J. Demmel and K. Veselic
            // SIAM. J. Matrix Anal. & Appl., 13(4), 1204-1245.
            // www.netlib.org/lapack/lawnspdf/lawn15.pdf
            
            // assume that A is n x m
            // then U is an n x m
            // V is m x m
            // S is length m
            
            //now solve for the station delays with SVD decomp;
            MHO_linalg_matrix U(n_baseline_parameters, n_station_parameters);
            MHO_linalg_matrix V(n_station_parameters, n_station_parameters);
            MHO_linalg_vector S(n_station_parameters);
            MHO_linalg_matrix_svd(A,U,S,V);
            
            std::cout<<"singular values:"<<std::endl;
            MHO_linalg_vector_print(S);
            
            MHO_linalg_matrix_svd_solve(U,S,V,b,x);
            std::cout<<"solution: "<<std::endl;
            MHO_linalg_vector_print(x);
            
            //check the solution 
            MHO_linalg_vector bprime = b;
            bprime.zero();
            MHO_linalg_matrix_vector_product(A,x,bprime);
            
            std::cout<<"b:"<<std::endl;
            MHO_linalg_vector_print(b);
            std::cout<<"bprime:"<<std::endl;
            MHO_linalg_vector_print(bprime);
            
            MHO_linalg_vector delta = bprime -b;
            std::cout<<"delta = "<<std::endl;
            MHO_linalg_vector_print(delta);
            
            std::cout<<"L2 norm = "<<delta.norm()<<std::endl;

        } //end of MPI_SINGLE_PROCESS

        MHO_MPIInterface::GetInstance()->GlobalBarrier();

    }//end of scan loop



#ifdef HOPS_USE_MPI
    MHO_MPIInterface::GetInstance()->GlobalBarrier();
    MHO_MPIInterface::GetInstance()->Finalize();
#endif

    return 0;
}
