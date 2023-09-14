#include <iostream>
#include <iomanip>
#include <cstring>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
namespace py = pybind11;

#include "MHO_Message.hh"
#include "MHO_FileKey.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_PyContainerStoreInterface.hh"

#include "MHO_ParameterStore.hh"
#include "MHO_PyParameterStoreInterface.hh"

using namespace hops;


int main()
{
    py::scoped_interpreter guard{}; // start the interpreter and keep it alive

    std::cout<<"the python path directories: "<<std::endl;

    py::exec(R"(
        import sys
        import numpy
        print(sys.path)
        import pyMHO_Containers
    )");

    MHO_ContainerStore store;

    visibility_type* obj = new visibility_type();
    obj->Resize(2,2,2,2);
    MHO_FileKey key;
    key.fLabel = 0;
    strncpy(key.fName, "vis", 3);

    //stuff something in the container store
    store.AddObject(obj);
    store.SetObjectLabel(obj->GetObjectUUID(), key.fLabel);
    std::string shortname = std::string(key.fName, MHO_FileKeyNameLength ).c_str();
    store.SetShortName(obj->GetObjectUUID(), shortname);

    //now put the object uuid in the parameter store so we can look it up on the python side
    MHO_ParameterStore paramStore;
    std::string obj_uuid_string = obj->GetObjectUUID().as_string();
    paramStore.Set("my_uuid", obj_uuid_string);

    //create the interfaces
    MHO_PyContainerStoreInterface conInter(&store);
    MHO_PyParameterStoreInterface parmInter(&paramStore);

    auto mho_constore_test = py::module::import("mho_container_store_test");
    mho_constore_test.attr("test_inter")(conInter, parmInter);

    return 0;
}
