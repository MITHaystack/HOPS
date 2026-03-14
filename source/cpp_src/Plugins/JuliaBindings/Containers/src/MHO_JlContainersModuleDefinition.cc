#include "jlcxx/jlcxx.hpp"

#include "MHO_ContainerDefinitions.hh"

#include "MHO_JlContainerStoreInterface.hh"
#include "MHO_JlFringeDataInterface.hh"
#include "MHO_JlParameterStoreInterface.hh"
#include "MHO_JlScanStoreInterface.hh"
#include "MHO_JlTableContainer.hh"

using namespace hops;

JLCXX_MODULE define_julia_module(jlcxx::Module& mod)
{
    // STL vector support (std::vector<std::string>, std::vector<int64_t>, etc.)
    // is provided by linking against JlCxx::cxxwrap_julia_stl - no runtime
    // registration call is needed here.

    // ------------------------------------------------------------------
    // 1. Table container wrappers (must be registered BEFORE the store
    //    interfaces that return pointers to them).
    // ------------------------------------------------------------------
    DeclareJlTableContainer< visibility_type >(mod,   "VisibilityType");
    DeclareJlTableContainer< weight_type >(mod,       "WeightType");
    DeclareJlTableContainer< visibility_store_type >(mod, "VisibilityStoreType");
    DeclareJlTableContainer< weight_store_type >(mod, "WeightStoreType");
    DeclareJlTableContainer< station_coord_type >(mod,"StationCoordType");
    DeclareJlTableContainer< phasor_type >(mod,       "PhasorType");
    DeclareJlTableContainer< multitone_pcal_type >(mod,"MultitonePcalType");

    // ------------------------------------------------------------------
    // 2. Store and fringe-data interfaces
    // ------------------------------------------------------------------
    DeclareJlParameterStoreInterface(mod,  "MHO_JlParameterStoreInterface");
    DeclareJlContainerStoreInterface(mod,  "MHO_JlContainerStoreInterface");
    DeclareJlScanStoreInterface(mod,       "MHO_JlScanStoreInterface");
    DeclareJlFringeDataInterface(mod,      "MHO_JlFringeDataInterface");
}
