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
            std::cout<<sdir<<" =? "<<scan_dir<<std::endl;
            if(sdir == scan_dir)
            {
                scan_pass_vector.push_back(pass);
            }
        }

        msg_info("main", "scan " << scan_dir << " contains "<< scan_pass_vector.size()<< " passes of data" << eom);

        for(std::size_t pass_index = 0; pass_index < scan_pass_vector.size(); pass_index++)
        {
            std::cout<<scan_pass_vector[pass_index].dump()<<std::endl;
        }

        //map to store all the fringe fit data for this scan
        // std::map< std::string, MHO_ParameterStore > scanFringeParameters;
        std::set< std::string > scanStations;

        //baseline quantities (map into 'b' vector)
        std::map< std::string, double > baseline_delay;
        std::map< std::string, double > baseline_snr;
        std::map< std::string, double > baseline_delay_rate;
        std::map< std::string, double > baseline_ion_diff;

        //this loop could be trivially parallelized (with the exception of plotting)
        for(std::size_t pass_index = 0; pass_index < scan_pass_vector.size(); pass_index++)
        {
            if(pass_index % n_processes == process_id) //only do work for this pass if it is assigned to this process
            {
                //profiler_start();

                //populate a few necessary parameters and  initialize the fringe/scan data
                //set the current pass info (directory, root_file, source, baseline, pol-product, frequency-group)
                mho_json pass = scan_pass_vector[pass_index];
                pass["build_time"] = HOPS_BUILD_TIMESTAMP; //set the build time stamp in the pass info
                //construct the scan-pass-key from baseline-polprod-fgroup
                std::string bline = pass["baseline"].get<std::string>();
                std::string pprod = pass["polprod"].get<std::string>();
                std::string fg = pass["frequency_group"].get<std::string>();
                //std::string rootcode = pass["root_file"].get<std::string>();
                std::string pkey = bline + "|" + pprod + "|" + fg; 
                
                // //std::cout<<"pkey = "<<pkey<<std::endl;
                // #ifdef HOPS_USE_MPI
                //     std::stringstream ss;
                //     ss << "process #"<<process_id<<" is working on pass: "<<pkey << "\n";
                //     MHO_MPIInterface::GetInstance()->PrintMessage(ss.str());
                // #endif
                // 
                //fringeData = MHO_FringeData();
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
                
                // //keep track of the station codes
                // std::string ref_station;
                // std::string rem_station;
                // fringeData.GetParameterStore()->Get("/config/reference_station", ref_station);
                // fringeData.GetParameterStore()->Get("/config/remote_station", rem_station);
                // std::cout<<"ref_station = "<<ref_station<<std::endl;
                // std::cout<<"rem_station = "<<rem_station<<std::endl;
                // scanStations.insert(ref_station);
                // scanStations.insert(rem_station);

                // //flush profile events
                // profiler_stop();
                // std::vector< MHO_ProfileEvent > events;
                // MHO_Profiler::GetInstance().GetEvents(events);
                // 
                // //convert and dump the events into the parameter store for now (will be empty unless enabled)
                // mho_json event_list = MHO_BasicFringeDataConfiguration::ConvertProfileEvents(events);
                // fringeData.GetParameterStore()->Set("/profile/events", event_list);
        
                //determine if this pass was skipped or is in test-mode
                bool is_skipped = fringeData.GetParameterStore()->GetAs< bool >("/status/skipped");
                bool test_mode = fringeData.GetParameterStore()->GetAs< bool >("/cmdline/test_mode");
                
                // //open and dump to file -- should we profile this as well?
                // if(!test_mode && !is_skipped)
                // {
                //     bool use_mk4_output = false;
                //     fringeData.GetParameterStore()->Get("/cmdline/mk4format_output", use_mk4_output);
                // 
                //     if(!use_mk4_output)
                //     {
                //         fringeData.WriteOutput();
                //     }
                //     else
                //     {
                //         MHO_MK4FringeExport fexporter;
                //         fexporter.SetParameterStore(fringeData.GetParameterStore());
                //         fexporter.SetPlotData(fringeData.GetPlotData());
                //         fexporter.SetContainerStore(fringeData.GetContainerStore());
                //         fexporter.ExportFringeFile();
                //     }
                // }
                // 
                // //use the plotter factory to construct one of the available plotting backends
                // if(!is_skipped)
                // {
                //     //currently we only have two fringe plotting options (gnuplot or matplotlib)
                //     std::string plot_backend;
                //     fringeData.GetParameterStore()->Get("/control/config/plot_backend", plot_backend);
                //     MHO_FringePlotVisitorFactory plotter_factory;
                //     MHO_FringePlotVisitor* plotter = plotter_factory.ConstructPlotter(plot_backend);
                //     if(plotter != nullptr)
                //     {
                //         ffit->Accept(plotter);
                //     }
                // }

                //extract our quanties of interest from the parameter store 
                // std::string ref_station;
                // std::string rem_station;
                // it->second.Get("/config/reference_station", ref_station);
                // it->second.Get("/config/remote_station", rem_station);
                // int ref_idx = station2int[ref_station];
                // int rem_idx = station2int[rem_station];
                double mbd;
                double drate;
                double snr;
                double dtec;
                fringeData.GetParameterStore()->Get("/fringe/mbdelay", mbd);
                fringeData.GetParameterStore()->Get("/fringe/drate", drate);
                fringeData.GetParameterStore()->Get("/fringe/snr", snr);
                fringeData.GetParameterStore()->Get("/fringe/ion_diff", dtec);
                //std::cout<<ref_station<<":"<<rem_station<<", "<<"pkey: "<<pkey<<", mbd: "<<mbd<<", drate: "<<drate<<" snr: "<<snr<<"dtec: "<<dtec<<std::endl;

                std::cout<<"process_id: "<<process_id<<" pkey: "<<pkey<<", mbd: "<<mbd<<", drate: "<<drate<<" snr: "<<snr<<" dtec: "<<dtec<<std::endl;


                baseline_delay[pkey] = mbd;
                baseline_snr[pkey] = snr;
                baseline_delay_rate[pkey] = drate;
                baseline_ion_diff[pkey] = dtec;

                std::cout<<"finished pass: "<<pkey<<std::endl;
            }


        } //end of pass loop


        //MPI_SINGLE_PROCESS
        {
        std::cout<<"END OF PASS LOOP"<<std::endl;
        }

        #ifdef HOPS_USE_MPI
        MHO_MPIInterface::GetInstance()->GlobalBarrier();
        #endif


        auto bl_delays = baseline_delay;
        auto bl_drates = baseline_delay_rate;
        auto bl_snr = baseline_snr;
        auto bl_ion = baseline_ion_diff;

        #ifdef HOPS_USE_MPI
        //merge the data from all processes so the root process can use it
        bl_delays = MHO_MPIInterface::GetInstance()->MergeMap(baseline_delay);
        bl_drates = MHO_MPIInterface::GetInstance()->MergeMap(baseline_delay_rate);
        bl_snr = MHO_MPIInterface::GetInstance()->MergeMap(baseline_snr);
        bl_ion = MHO_MPIInterface::GetInstance()->MergeMap(baseline_ion_diff);
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

            // //map stations to integers 
            // std::map<std::string, int> station2int;
            // std::map<int, std::string> int2station;
            // int count = 0;
            // for(auto sit = scanStations.begin(); sit != scanStations.end(); sit++)
            // {
            //     std::string st = *sit;
            //     station2int[st] = count;
            //     int2station[count] = st;
            //     std::cout<<count<<": "<<st<<std::endl;
            //     count++;
            // }
            // 
            // std::cout<<"fringe-fitted: "<<scanFringeParameters.size()<<std::endl;
            // int nstations = scanStations.size();
            // std::cout<<"nstations = "<<nstations<<std::endl;
            // MHO_linalg_matrix mbd_delta_mx(nstations, nstations);
            // MHO_linalg_matrix drate_delta_mx(nstations, nstations);
            // 
            // //difference matrix Ax = b, (x is station delays, b is baseline mbd)
            // MHO_linalg_matrix A(scanFringeParameters.size(), nstations);
            // A.zero();
            // 
            // MHO_linalg_vector b(scanFringeParameters.size());
            // b.zero();
            // 
            // MHO_linalg_vector x(nstations);
            // x.zero();
            // 
            // count = 0;
            // for(auto it = scanFringeParameters.begin(); it != scanFringeParameters.end(); it++)
            // {
            //     std::string pkey = it->first;
            // 
            //     std::string ref_station;
            //     std::string rem_station;
            //     it->second.Get("/config/reference_station", ref_station);
            //     it->second.Get("/config/remote_station", rem_station);
            // 
            //     int ref_idx = station2int[ref_station];
            //     int rem_idx = station2int[rem_station];
            // 
            //     if(ref_idx != rem_idx)
            //     {
            //         double mbd;
            //         double drate;
            //         double snr;
            //         double dtec;
            //         it->second.Get("/fringe/mbdelay", mbd);
            //         it->second.Get("/fringe/drate", drate);
            //         it->second.Get("/fringe/snr", snr);
            //         it->second.Get("/fringe/ion_diff", dtec);
            //         std::cout<<ref_station<<":"<<rem_station<<", "<<"pkey: "<<pkey<<", mbd: "<<mbd<<", drate: "<<drate<<" snr: "<<snr<<"dtec: "<<dtec<<std::endl;
            // 
            //         mbd_delta_mx(ref_idx,rem_idx) = mbd;
            //         drate_delta_mx(ref_idx,rem_idx) = drate;
            // 
            //         //make sure we include the opposite entries for skew-symmetric matrix 
            //         mbd_delta_mx(ref_idx, rem_idx) = -1.0*mbd;
            //         drate_delta_mx(rem_idx, ref_idx) = -1.0*drate;
            // 
            //         if(snr > 15.0)
            //         {
            //             //set the output vector elements
            //             b(count) = mbd;
            // 
            //             //set the xform matrix elements
            //             A(count,ref_idx) = -1.0;
            //             A(count, rem_idx) = 1.0;
            //         }
            //         count++;
            //     }
            // }
            // 
            // std::cout<<"MBD deltas: "<<std::endl;
            // MHO_linalg_matrix_print(mbd_delta_mx);
            // 
            // std::cout<<"drate deltas:"<<std::endl;
            // MHO_linalg_matrix_print(drate_delta_mx);
            // 
            // std::cout<<"xform mx = "<<std::endl;
            // MHO_linalg_matrix_print(A);
            // 
            // // this function uses the slower but more accurate one-sided jacobi svd
            // // as defined in the paper:
            // // Jacobi's method is more accurate than QR by J. Demmel and K. Veselic
            // // SIAM. J. Matrix Anal. & Appl., 13(4), 1204-1245.
            // // www.netlib.org/lapack/lawnspdf/lawn15.pdf
            // 
            // // assume that A is n x m
            // // then U is an n x m
            // // V is m x m
            // // S is length m
            // 
            // //now solve for the station delays with SVD decomp;
            // MHO_linalg_matrix U(scanFringeParameters.size(), nstations);
            // MHO_linalg_matrix V(nstations, nstations);
            // MHO_linalg_vector S(nstations);
            // MHO_linalg_matrix_svd(A,U,S,V);
            // 
            // std::cout<<"singular values:"<<std::endl;
            // MHO_linalg_vector_print(S);
            // 
            // MHO_linalg_matrix_svd_solve(U,S,V,b,x);
            // std::cout<<"solution: "<<std::endl;
            // MHO_linalg_vector_print(x);
            // 
            // //check the solution 
            // MHO_linalg_vector bprime = b;
            // bprime.zero();
            // MHO_linalg_matrix_vector_product(A,x,bprime);
            // 
            // std::cout<<"b, bprime:"<<std::endl;
            // MHO_linalg_vector_print(b);
            // MHO_linalg_vector_print(bprime);
            // 
            // MHO_linalg_vector delta = bprime -b;
            // std::cout<<"delta = "<<std::endl;
            // MHO_linalg_vector_print(delta);
            // 
            // std::cout<<"L2 norm = "<<delta.norm()<<std::endl;

        } //end of MPI_SINGLE_PROCESS

        MHO_MPIInterface::GetInstance()->GlobalBarrier();

    }//end of scan loop



#ifdef HOPS_USE_MPI
    MHO_MPIInterface::GetInstance()->GlobalBarrier();
    MHO_MPIInterface::GetInstance()->Finalize();
#endif

    return 0;
}
