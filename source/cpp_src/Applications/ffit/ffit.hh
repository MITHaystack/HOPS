#ifndef FFIT_UTILS_H__
#define FFIT_UTILS_H__

//global messaging util
#include "MHO_Message.hh"

//snapshot utility lib
#include "MHO_Snapshot.hh"

//data/config passing classes
#include "MHO_ParameterStore.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_JSONHeaderWrapper.hh"

//initialization
#include "MHO_OperatorBuilderManager.hh"

//pybind11 stuff to interface with python
#ifdef USE_PYBIND11
    #include <pybind11/pybind11.h>
    #include <pybind11/embed.h>
    #include "pybind11_json/pybind11_json.hpp"
    namespace py = pybind11;
    namespace nl = nlohmann;
    using namespace pybind11::literals;
#endif

using namespace hops;

//helper functions 
void configure_data_library(MHO_ContainerStore* store);
void extract_vex_info(const mho_json& vexInfo, MHO_ParameterStore* paramStore);
void calculate_fringe_info(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore, const mho_json& vexInfo);
void fill_plot_data(MHO_ParameterStore* paramStore, mho_json& plot_dict);
void precalculate_quantities(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);
void init_and_exec_operators(MHO_OperatorBuilderManager& build_manager, MHO_OperatorToolbox* opToolbox, const char* category);
int parse_command_line(int argc, char** argv, MHO_ParameterStore* paramStore);
void basic_fringe_search(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);
mho_json construct_plot_data(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore, mho_json& vexInfo);

#endif
