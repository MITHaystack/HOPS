#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <utility>
#include <map>
#include <getopt.h>

#include "MHO_Message.hh"

#include "MHO_ScanDataStore.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_PyContainerStoreInterface.hh"
#include "MHO_ElementTypeCaster.hh"

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
namespace py = pybind11;


#include "MHO_PyGenericOperator.hh"


using namespace hops;



void configure_data_library(MHO_ContainerStore* store)
{
    //retrieve the (first) visibility and weight objects
    //(currently assuming there is only one object per type)
    visibility_store_type* vis_store_data = nullptr;
    weight_store_type* wt_store_data = nullptr;

    vis_store_data = store->GetObject<visibility_store_type>(0);
    wt_store_data = store->GetObject<weight_store_type>(0);

    if(vis_store_data == nullptr)
    {
        msg_fatal("main", "failed to read visibility data from the .cor file." <<eom);
        std::exit(1);
    }

    if(wt_store_data == nullptr)
    {
        msg_fatal("main", "failed to read weight data from the .cor file." <<eom);
        std::exit(1);
    }

    std::size_t n_vis = store->GetNObjects<visibility_store_type>();
    std::size_t n_wt = store->GetNObjects<weight_store_type>();

    if(n_vis != 1 || n_wt != 1)
    {
        msg_warn("main", "multiple visibility and/or weight types not yet supported" << eom);
    }

    std::string vis_shortname = store->GetShortName(vis_store_data->GetObjectUUID() );
    std::string wt_shortname = store->GetShortName(wt_store_data->GetObjectUUID() );

    visibility_type* vis_data = new visibility_type();
    weight_type* wt_data = new weight_type();

    MHO_ElementTypeCaster<visibility_store_type, visibility_type> up_caster;
    up_caster.SetArgs(vis_store_data, vis_data);
    up_caster.Initialize();
    up_caster.Execute();

    MHO_ElementTypeCaster< weight_store_type, weight_type> wt_up_caster;
    wt_up_caster.SetArgs(wt_store_data, wt_data);
    wt_up_caster.Initialize();
    wt_up_caster.Execute();

    //remove the original objects
    store->DeleteObject(vis_store_data);
    store->DeleteObject(wt_store_data);

    #pragma message("TODO - if we plan to rely on short-names to identify objects, we need to validate them here")
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

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"directory", required_argument, 0, 'd'},
                                          {"baseline", required_argument, 0, 'b'}};

    static const char* optString = "hd:b:";

    while(true)
    {
        char optId = getopt_long(argc, argv, optString, longOptions, NULL);
        if (optId == -1)
            break;
        switch(optId)
        {
            case ('h'):  // help
                std::cout << usage << std::endl;
                return 0;
            case ('d'):
                directory = std::string(optarg);
                break;
            case ('b'):
                baseline = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    //provide necessary objects for operation
    MHO_ParameterStore paramStore; //stores various parameters using string keys
    MHO_ScanDataStore scanStore; //provides access to data associated with this scan
    MHO_ContainerStore conStore; //stores data containers for in-use data

    ////////////////////////////////////////////////////////////////////////////
    //INITIALIZE SCAN DIRECTORY
    ////////////////////////////////////////////////////////////////////////////

    //initialize the scan store from this directory
    scanStore.SetDirectory(directory);
    scanStore.Initialize();
    if( !scanStore.IsValid() )
    {
        msg_fatal("main", "cannot initialize a valid scan store from this directory: " << directory << eom);
        std::exit(1);
    }

    //pass the directory and baseline into the parameter store
    paramStore.Set("directory", directory);
    paramStore.Set("baseline", baseline);

    //load baseline data
    scanStore.LoadBaseline(baseline, &conStore);
    //configure_data_library(&conStore);//momentarily needed for float -> double cast
    //load and rename station data according to reference/remote
    std::string ref_station_mk4id = std::string(1,baseline[0]);
    std::string rem_station_mk4id = std::string(1,baseline[1]);
    scanStore.LoadStation(ref_station_mk4id, &conStore);
    conStore.RenameObject("sta", "ref_sta");
    scanStore.LoadStation(rem_station_mk4id, &conStore);
    conStore.RenameObject("sta", "rem_sta");

    configure_data_library(&conStore);

    station_coord_type* ref_data = conStore.GetObject<station_coord_type>(std::string("ref_sta"));
    station_coord_type* rem_data = conStore.GetObject<station_coord_type>(std::string("rem_sta"));
    visibility_type* vis_data = conStore.GetObject<visibility_type>(std::string("vis"));
    weight_type* wt_data = conStore.GetObject<weight_type>(std::string("weight"));
    if( vis_data == nullptr)
    {
        msg_fatal("main", "failed to load visibility object." << eom);
        std::exit(1);
    }

    if( wt_data == nullptr )
    {
        msg_fatal("main", "failed to load weight object." << eom);
        std::exit(1);
    }

    if( ref_data == nullptr)
    {
        msg_fatal("main", "failed to load ref station object." << eom);
        std::exit(1);
    }

    if( rem_data == nullptr)
    {
        msg_fatal("main", "failed to load rem station object." << eom);
        std::exit(1);
    }

    //now put the object uuid in the parameter store so we can look it up on the python side
    std::string vis_uuid = vis_data->GetObjectUUID().as_string();
    std::string wt_uuid = wt_data->GetObjectUUID().as_string();
    std::string ref_uuid = ref_data->GetObjectUUID().as_string();
    std::string rem_uuid = rem_data->GetObjectUUID().as_string();

    paramStore.Set("/uuid/visibilities", vis_uuid);
    paramStore.Set("/uuid/weights", wt_uuid);
    paramStore.Set("/uuid/ref_station", ref_uuid);
    paramStore.Set("/uuid/rem_station", rem_uuid);


    MHO_PyGenericOperator pyOper;
    pyOper.SetModuleName("mho_test");
    pyOper.SetFunctionName("test_plot_visibilities");
    pyOper.SetContainerStore(&conStore);
    pyOper.SetParameterStore(&paramStore);

    py::scoped_interpreter guard{}; // start the interpreter and keep it alive

    std::cout<<"*************** executing operation via python **************"<<std::endl;

    pyOper.Initialize();
    pyOper.Execute();

    return 0;
}












// 
// 
// 
// 
// 
// 
// 
// int main(int argc, char** argv)
// {
//     std::string usage = "TestPythonUnaryOperator -d <directory> -b <baseline>";
// 
//     MHO_Message::GetInstance().AcceptAllKeys();
//     MHO_Message::GetInstance().SetMessageLevel(eDebug);
// 
//     std::string directory;
//     std::string baseline;
// 
//     static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
//                                           {"directory", required_argument, 0, 'd'},
//                                           {"baseline", required_argument, 0, 'b'}};
// 
//     static const char* optString = "hd:b:";
// 
//     while(true)
//     {
//         char optId = getopt_long(argc, argv, optString, longOptions, NULL);
//         if (optId == -1)
//             break;
//         switch(optId)
//         {
//             case ('h'):  // help
//                 std::cout << usage << std::endl;
//                 return 0;
//             case ('d'):
//                 directory = std::string(optarg);
//                 break;
//             case ('b'):
//                 baseline = std::string(optarg);
//                 break;
//             default:
//                 std::cout << usage << std::endl;
//                 return 1;
//         }
//     }
// 
//     //read the directory file list
//     std::vector< std::string > allFiles;
//     std::vector< std::string > corFiles;
//     std::vector< std::string > staFiles;
//     std::vector< std::string > jsonFiles;
//     MHO_DirectoryInterface dirInterface;
//     dirInterface.SetCurrentDirectory(directory);
//     dirInterface.ReadCurrentDirectory();
// 
//     std::cout<<"directory = "<<dirInterface.GetCurrentDirectory()<<std::endl;
//     dirInterface.GetFileList(allFiles);
//     dirInterface.GetFilesMatchingExtention(corFiles, "cor");
//     dirInterface.GetFilesMatchingExtention(staFiles, "sta");
//     dirInterface.GetFilesMatchingExtention(jsonFiles, "json");
// 
// 
//     for(auto it = corFiles.begin(); it != corFiles.end(); it++)
//     {
//         std::cout<<"cor: "<< *it <<std::endl;
//     }
// 
//     for(auto it = staFiles.begin(); it != staFiles.end(); it++)
//     {
//         std::cout<<"sta: "<< *it <<std::endl;
//     }
// 
// 
//     for(auto it = jsonFiles.begin(); it != jsonFiles.end(); it++)
//     {
//         std::cout<<"json: "<< *it <<std::endl;
//     }
// 
//     //check that there is only one json file
//     std::string root_file = "";
//     if(jsonFiles.size() != 1)
//     {
//         msg_fatal("main", "There are: "<<jsonFiles.size()<<" root files." << eom);
//         std::exit(1);
//     }
//     else
//     {
//         root_file = jsonFiles[0];
//     }
// 
//     //locate the corel file that contains the baseline of interest
//     std::string corel_file = "";
//     bool found_baseline = false;
//     for(auto it = corFiles.begin(); it != corFiles.end(); it++)
//     {
//         std::size_t index = it->find(baseline);
//         if(index != std::string::npos)
//         {
//             corel_file = *it;
//             found_baseline = true;
//         }
//     }
// 
//     if(!found_baseline)
//     {
//         msg_fatal("main", "Could not find a file for baseline: "<< baseline << eom);
//         std::exit(1);
//     }
// 
//     std::cout<<"Will use root file: "<<root_file<<std::endl;
//     std::cout<<"Will use corel file: "<<corel_file<<std::endl;
// 
//     //now open and read the (channelized) baseline visibility data
//     visibility_type* bl_data = new visibility_type();
//     MHO_BinaryFileInterface inter;
//     bool status = inter.OpenToRead(corel_file);
//     if(status)
//     {
//         MHO_FileKey key;
//         inter.Read(*bl_data, key);
//         //std::cout<<"baseline object label = "<<blabel<<std::endl;
//         std::cout<<"Total size of baseline data = "<<bl_data->GetSerializedSize()<<std::endl;
//     }
//     else
//     {
//         std::cout<<" error opening file to read"<<std::endl;
//         inter.Close();
//         std::exit(1);
//     }
//     inter.Close();
// 
//     std::size_t bl_dim[VIS_NDIM];
//     bl_data->GetDimensions(bl_dim);
// 
//     //now we are going to pass the visibility data to python, and plot
//     //the amp/phase of the visibilities for a particular channel
// 
// 
//     MHO_PyContainerInterface myInterface;
//     myInterface.SetVisibilities(bl_data);
// 
//     // start the interpreter and keep it alive the whole program!
//     //TODO FIXME -- we have to do this because some modules (numpy) cannot be re-initialized
//     //if the interpreter goes out of scope...to work around this we are probably going to
//     //need a singleton interface class to handle the python interpreter start-up and shutdown
//     //see issue: https://github.com/pybind/pybind11/issues/3112
//     py::scoped_interpreter guard{};
// 
//     std::cout<<"*************** 1st view of visibilities python **************"<<std::endl;
//     {
//         //load our interface module
//         auto mho_test = py::module::import("mho_test");
//         //call a python functioin on the interface class instance
//         mho_test.attr("test_plot_visibilities")(myInterface);
//     } //interpreter goes out of scope here
// 
//     std::cout<<"*************** calling python operator **************"<<std::endl;
// 
//     MHO_PyUnaryTableOperator test_op;
//     std::string mod_name = "mho_operators";
//     std::string op_name = "TestThreshold";
//     test_op.SetModuleFunctionName(mod_name, op_name);
//     test_op.SetInput(bl_data);
//     test_op.Initialize();
//     test_op.Execute();
// 
//     std::cout<<"*************** contents of array now **************"<<std::endl;
//     for(size_t i=0; i<bl_data->GetDimension(TIME_AXIS); i++)
//     {
//         for(size_t j=0; j<bl_data->GetDimension(FREQ_AXIS); j++)
//         {
//             std::cout<<"vis(0,0,"<<i<<","<<j<<") = "<<(*bl_data)(0,0,i,j)<<std::endl;
//         }
//     }
// 
//     std::cout<<"*************** 2nd view of visibilities python **************"<<std::endl;
//     {
//         //py::scoped_interpreter guard{}; // start the interpreter and keep it alive
//         //load our interface module
//         auto mho_test = py::module::import("mho_test");
//         //call a python functioin on the interface class instance
//         mho_test.attr("test_plot_visibilities")(myInterface);
//     } //interpreter goes out of scope here
// 
// 
// 
// 
// 
//     delete bl_data;
// 
//     return 0;
// }
