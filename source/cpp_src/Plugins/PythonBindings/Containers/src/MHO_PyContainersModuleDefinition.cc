#include "MHO_PyContainerStoreInterface.hh"
#include "MHO_PyFringeDataInterface.hh"
#include "MHO_PyParameterStoreInterface.hh"
#include "MHO_PyScanStoreInterface.hh"
#include "MHO_PyTableContainer.hh"

#include "MHO_ContainerDefinitions.hh"

using namespace hops;

PYBIND11_MODULE(pyMHO_Containers, m)
{
    m.doc() = "module to interact with MHO_Containers"; // optional module docstring

    //declare several data container types
    DeclarePyTableContainer< visibility_type >(m, "visibility_type");
    DeclarePyTableContainer< weight_type >(m, "weight_type");
    DeclarePyTableContainer< visibility_store_type >(m, "visibility_store_type");
    DeclarePyTableContainer< weight_store_type >(m, "weight_store_type");
    DeclarePyTableContainer< station_coord_type >(m, "station_coord_type");
    DeclarePyTableContainer< phasor_type >(m, "phasor_type");
    DeclarePyTableContainer< multitone_pcal_type >(m, "multitone_pcal_type");

    //declare interfaces to the parameter, container, and scan store
    DeclarePyParameterStoreInterface(m, "MHO_PyParameterStoreInterface");
    DeclarePyContainerStoreInterface(m, "MHO_PyContainerStoreInterface");
    DeclarePyScanStoreInterface(m, "MHO_PyScanStoreInterface");

    //declare interface to the fringe data (e.g current pass's data as parameter, container, and scan stores)
    DeclarePyFringeDataInterface(m, "MHO_PyFringeDataInterface");
}
