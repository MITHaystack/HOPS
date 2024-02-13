#include "MHO_PyTableContainer.hh"
#include "MHO_PyParameterStoreInterface.hh"
#include "MHO_PyContainerStoreInterface.hh"
#include "MHO_PyScanStoreInterface.hh"

#include "MHO_ContainerDefinitions.hh"


using namespace hops;


PYBIND11_MODULE(pyMHO_Containers, m)
{
        m.doc() = "module to interact with MHO_Containers"; // optional module docstring

        //declare several data container types
        DeclarePyTableContainer< visibility_type >(m, std::string("visibility_type") );
        DeclarePyTableContainer< weight_type >(m, std::string("weight_type") );
        DeclarePyTableContainer< visibility_store_type >(m, std::string("visibility_store_type") );
        DeclarePyTableContainer< weight_store_type >(m, std::string("weight_store_type") );
        DeclarePyTableContainer< station_coord_type >(m, std::string("station_coord_type") );
        DeclarePyTableContainer< phasor_type >(m, std::string("phasor_type") );
        DeclarePyTableContainer< multitone_pcal_type >(m, std::string("multitone_pcal_type") );

        //delcare interfaces to the parameter store and the container store
        DeclarePyParameterStoreInterface(m, "MHO_PyParameterStoreInterface");
        DeclarePyContainerStoreInterface(m, "MHO_PyContainerStoreInterface");
        DeclarePyScanStoreInterface(m, std::string("MHO_PyScanStoreInterface") );

}
