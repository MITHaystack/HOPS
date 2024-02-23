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
#include "MHO_PyConfigurePath.hh"

using namespace hops;


int main()
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    py::scoped_interpreter guard{}; // start the interpreter and keep it alive
    configure_pypath();

    std::cout<<"the python path directories: "<<std::endl;

    py::exec(R"(
        import sys
        import numpy
        print(sys.path)
        import pyMHO_Containers
    )");

    MHO_ContainerStore store;

    visibility_type* obj = new visibility_type();
    weight_type* wobj = new weight_type();
    obj->Resize(1,1,2,2);
    obj->ZeroArray();
    wobj->Resize(1,1,2,2);
    wobj->ZeroArray();

    MHO_FileKey key;
    key.fLabel = 0;
    strncpy(key.fName, "vis", 3);

    MHO_FileKey wkey;
    wkey.fLabel = 0;
    strncpy(wkey.fName, "weight", 6);

    //stuff something in the container store
    store.AddObject(obj);
    std::string shortname = std::string(key.fName, MHO_FileKeyNameLength ).c_str();
    store.SetShortName(obj->GetObjectUUID(), shortname);

    store.AddObject(wobj);
    std::string wshortname = std::string(wkey.fName, MHO_FileKeyNameLength ).c_str();
    store.SetShortName(wobj->GetObjectUUID(), wshortname);

    //now put the object uuid in the parameter store so we can look it up on the python side
    MHO_ParameterStore paramStore;
    std::string obj_uuid_string = obj->GetObjectUUID().as_string();
    std::string wobj_uuid_string = wobj->GetObjectUUID().as_string();

    paramStore.Set("vis_uuid", obj_uuid_string);
    paramStore.Set("weight_uuid", wobj_uuid_string);

    //create the interfaces
    MHO_PyContainerStoreInterface conInter(&store);
    MHO_PyParameterStoreInterface parmInter(&paramStore);

    auto mho_constore_test = py::module::import("mho_container_store_test");
    mho_constore_test.attr("test_inter")(conInter, parmInter);

    //dump out the visibility array
    for(std::size_t i=0; i<obj->GetSize(); i++)
    {
        std::cout<<"vis value @ "<<i<<" = "<<(*obj)[i]<<std::endl;
    }

    //dump out the weight array
    for(std::size_t i=0; i<obj->GetSize(); i++)
    {
        std::cout<<"weight value @ "<<i<<" = "<<(*wobj)[i]<<std::endl;
    }

    return 0;
}
