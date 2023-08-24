#ifndef FFIT_UTILS_H__
#define FFIT_UTILS_H__

//global messaging util
#include "MHO_Message.hh"

//snapshot utility lib
#include "MHO_Snapshot.hh"

//needed to read hops files and extract objects from scan dir
#include "MHO_ScanDataStore.hh"

//control
#include "MHO_ControlFileParser.hh"
#include "MHO_ControlConditionEvaluator.hh"

//operators

#include "MHO_NormFX.hh"
#include "MHO_SelectRepack.hh"
#include "MHO_UniformGridPointsCalculator.hh"
#include "MHO_FringeRotation.hh"
#include "MHO_Reducer.hh"

#include "MHO_AbsoluteValue.hh"
#include "MHO_FunctorBroadcaster.hh"
#include "MHO_ExtremaSearch.hh"
#include "MHO_ManualChannelPhaseCorrection.hh"
#include "MHO_DelayRate.hh"
#include "MHO_MBDelaySearch.hh"
#include "MHO_InterpolateFringePeak.hh"

//initialization
#include "MHO_OperatorBuilderManager.hh"
#include "MHO_ParameterConfigurator.hh"
#include "MHO_ParameterManager.hh"

#include "MHO_ComputePlotData.hh"
#include "MHO_DelayModel.hh"

#include "MHO_Clock.hh"

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
void fill_output_info(const MHO_ParameterStore* paramStore, const mho_json& vexInfo, mho_json& plot_dict);
void precalculate_quantities(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);
void build_and_exec_operators(MHO_OperatorBuilderManager& build_manager, MHO_OperatorToolbox* opToolbox, const char* category);
int parse_command_line(int argc, char** argv, MHO_ParameterStore* paramStore);
void basic_fringe_search(MHO_ContainerStore* conStore, MHO_ParameterStore* paramStore);

#endif
