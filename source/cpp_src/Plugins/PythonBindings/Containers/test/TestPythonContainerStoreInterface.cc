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

    store.AddObject(obj);
    store.SetObjectLabel(obj->GetObjectUUID(), key.fLabel);
    std::string shortname = std::string(key.fName, MHO_FileKeyNameLength ).c_str();
    store.SetShortName(obj->GetObjectUUID(), shortname);

    //stuff something in the container store

    MHO_PyContainerStoreInterface conInter(&store);

    auto mho_constore_test = py::module::import("mho_container_store_test");
    mho_constore_test.attr("test_inter")(conInter);

    return 0;
}
