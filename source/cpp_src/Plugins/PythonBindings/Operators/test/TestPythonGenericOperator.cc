#include <algorithm>
#include <getopt.h>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "MHO_Message.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_ElementTypeCaster.hh"
#include "MHO_PyContainerStoreInterface.hh"
#include "MHO_ScanDataStore.hh"

#include "MHO_FringeData.hh"

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
namespace py = pybind11;

#include "MHO_PyConfigurePath.hh"
#include "MHO_PyGenericOperator.hh"

using namespace hops;

void configure_data_library(MHO_ContainerStore* store)
{
    //retrieve the (first) visibility and weight objects
    //(currently assuming there is only one object per type)
    visibility_store_type* vis_store_data = nullptr;
    weight_store_type* wt_store_data = nullptr;

    vis_store_data = store->GetObject< visibility_store_type >(0);
    wt_store_data = store->GetObject< weight_store_type >(0);

    if(vis_store_data == nullptr)
    {
        msg_fatal("main", "failed to read visibility data from the .cor file." << eom);
        std::exit(1);
    }

    if(wt_store_data == nullptr)
    {
        msg_fatal("main", "failed to read weight data from the .cor file." << eom);
        std::exit(1);
    }

    std::size_t n_vis = store->GetNObjects< visibility_store_type >();
    std::size_t n_wt = store->GetNObjects< weight_store_type >();

    if(n_vis != 1 || n_wt != 1)
    {
        msg_warn("main", "multiple visibility and/or weight types not yet supported" << eom);
    }

    std::string vis_shortname = store->GetShortName(vis_store_data->GetObjectUUID());
    std::string wt_shortname = store->GetShortName(wt_store_data->GetObjectUUID());

    visibility_type* vis_data = new visibility_type();
    weight_type* wt_data = new weight_type();

    MHO_ElementTypeCaster< visibility_store_type, visibility_type > up_caster;
    up_caster.SetArgs(vis_store_data, vis_data);
    up_caster.Initialize();
    up_caster.Execute();

    MHO_ElementTypeCaster< weight_store_type, weight_type > wt_up_caster;
    wt_up_caster.SetArgs(wt_store_data, wt_data);
    wt_up_caster.Initialize();
    wt_up_caster.Execute();

    //remove the original objects
    store->DeleteObject(vis_store_data);
    store->DeleteObject(wt_store_data);

    TODO_FIXME_MSG("TODO - if we plan to rely on short-names to identify objects, we need to validate them here")
    //TODO make sure that the visibility object is called 'vis' and weights are called 'weights', etc.
    //TODO also validate the station data

    //now shove the double precision data into the container store with the same shortname
    store->AddObject(vis_data);
    store->AddObject(wt_data);
    store->SetShortName(vis_data->GetObjectUUID(), vis_shortname);
    store->SetShortName(wt_data->GetObjectUUID(), wt_shortname);
}

int main(int argc, char** argv)
{
    std::string usage = "TestPythonGenericOperator -d <directory> -b <baseline>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string directory;
    std::string baseline;

    static struct option longOptions[] = {
        {"help",      no_argument,       0, 'h'},
        {"directory", required_argument, 0, 'd'},
        {"baseline",  required_argument, 0, 'b'}
    };

    static const char* optString = "hd:b:";

    while(true)
    {
        char optId = getopt_long(argc, argv, optString, longOptions, NULL);
        if(optId == -1)
            break;
        switch(optId)
        {
            case('h'): // help
                std::cout << usage << std::endl;
                return 0;
            case('d'):
                directory = std::string(optarg);
                break;
            case('b'):
                baseline = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    MHO_FringeData fdata;
    //provide necessary objects for operation
    MHO_ParameterStore* paramStore = fdata.GetParameterStore(); //stores various parameters using string keys
    MHO_ScanDataStore* scanStore = fdata.GetScanDataStore();    //provides access to data associated with this scan
    MHO_ContainerStore* conStore = fdata.GetContainerStore();   //stores data containers for in-use data

    ////////////////////////////////////////////////////////////////////////////
    //INITIALIZE SCAN DIRECTORY
    ////////////////////////////////////////////////////////////////////////////

    //initialize the scan store from this directory
    scanStore->SetDirectory(directory);
    scanStore->Initialize();
    if(!scanStore->IsValid())
    {
        msg_fatal("main", "cannot initialize a valid scan store from this directory: " << directory << eom);
        std::exit(1);
    }

    //pass the directory and baseline into the parameter store
    paramStore->Set("directory", directory);
    paramStore->Set("baseline", baseline);

    //load baseline data
    scanStore->LoadBaseline(baseline, conStore);
    //configure_data_library(&conStore->;//momentarily needed for float -> double cast
    //load and rename station data according to reference/remote
    std::string ref_station_mk4id = std::string(1, baseline[0]);
    std::string rem_station_mk4id = std::string(1, baseline[1]);
    scanStore->LoadStation(ref_station_mk4id, conStore);
    conStore->RenameObject("sta", "ref_sta");
    scanStore->LoadStation(rem_station_mk4id, conStore);
    conStore->RenameObject("sta", "rem_sta");

    configure_data_library(conStore);

    station_coord_type* ref_data = conStore->GetObject< station_coord_type >(std::string("ref_sta"));
    station_coord_type* rem_data = conStore->GetObject< station_coord_type >(std::string("rem_sta"));
    visibility_type* vis_data = conStore->GetObject< visibility_type >(std::string("vis"));
    weight_type* wt_data = conStore->GetObject< weight_type >(std::string("weight"));
    if(vis_data == nullptr)
    {
        msg_fatal("main", "failed to load visibility object." << eom);
        std::exit(1);
    }

    if(wt_data == nullptr)
    {
        msg_fatal("main", "failed to load weight object." << eom);
        std::exit(1);
    }

    if(ref_data == nullptr)
    {
        msg_fatal("main", "failed to load ref station object." << eom);
        std::exit(1);
    }

    if(rem_data == nullptr)
    {
        msg_fatal("main", "failed to load rem station object." << eom);
        std::exit(1);
    }

    //now put the object uuid in the parameter store so we can look it up on the python side
    std::string vis_uuid = vis_data->GetObjectUUID().as_string();
    std::string wt_uuid = wt_data->GetObjectUUID().as_string();
    std::string ref_uuid = ref_data->GetObjectUUID().as_string();
    std::string rem_uuid = rem_data->GetObjectUUID().as_string();

    paramStore->Set("/uuid/visibilities", vis_uuid);
    paramStore->Set("/uuid/weights", wt_uuid);
    paramStore->Set("/uuid/ref_station", ref_uuid);
    paramStore->Set("/uuid/rem_station", rem_uuid);

    MHO_PyGenericOperator pyOper;
    pyOper.SetModuleName("mho_test");
    pyOper.SetFunctionName("test_plot_visibilities");
    pyOper.SetFringeData(&fdata);
    // pyOper.SetContainerStore(&conStore->;
    // pyOper.SetParameterStore(&paramStore->;

    py::scoped_interpreter guard{}; // start the interpreter and keep it alive
    configure_pypath();

    std::cout << "*************** executing operation via python **************" << std::endl;

    pyOper.Initialize();
    pyOper.Execute();

    return 0;
}
