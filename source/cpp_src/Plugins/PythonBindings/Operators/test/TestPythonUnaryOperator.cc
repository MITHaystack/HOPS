#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <utility>
#include <map>
#include <getopt.h>

#include "MHO_Message.hh"

#include "MHO_DirectoryInterface.hh"
#include "MHO_BinaryFileInterface.hh"

#include "MHO_ContainerDefinitions.hh"

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
namespace py = pybind11;

#include "MHO_PyContainerInterface.hh"
#include "MHO_PyUnaryTableOperator.hh"
#include "MHO_PyConfigurePath.hh"


using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "TestPythonUnaryOperator -d <directory> -b <baseline>";

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

    //read the directory file list
    std::vector< std::string > allFiles;
    std::vector< std::string > corFiles;
    std::vector< std::string > staFiles;
    std::vector< std::string > jsonFiles;
    MHO_DirectoryInterface dirInterface;
    dirInterface.SetCurrentDirectory(directory);
    dirInterface.ReadCurrentDirectory();

    std::cout<<"directory = "<<dirInterface.GetCurrentDirectory()<<std::endl;
    dirInterface.GetFileList(allFiles);
    dirInterface.GetFilesMatchingExtention(corFiles, "cor");
    dirInterface.GetFilesMatchingExtention(staFiles, "sta");
    dirInterface.GetFilesMatchingExtention(jsonFiles, "json");


    for(auto it = corFiles.begin(); it != corFiles.end(); it++)
    {
        std::cout<<"cor: "<< *it <<std::endl;
    }

    for(auto it = staFiles.begin(); it != staFiles.end(); it++)
    {
        std::cout<<"sta: "<< *it <<std::endl;
    }


    for(auto it = jsonFiles.begin(); it != jsonFiles.end(); it++)
    {
        std::cout<<"json: "<< *it <<std::endl;
    }

    //check that there is only one json file
    std::string root_file = "";
    if(jsonFiles.size() != 1)
    {
        msg_fatal("main", "There are: "<<jsonFiles.size()<<" root files." << eom);
        std::exit(1);
    }
    else
    {
        root_file = jsonFiles[0];
    }

    //locate the corel file that contains the baseline of interest
    std::string corel_file = "";
    bool found_baseline = false;
    for(auto it = corFiles.begin(); it != corFiles.end(); it++)
    {
        std::size_t index = it->find(baseline);
        if(index != std::string::npos)
        {
            corel_file = *it;
            found_baseline = true;
        }
    }

    if(!found_baseline)
    {
        msg_fatal("main", "Could not find a file for baseline: "<< baseline << eom);
        std::exit(1);
    }

    std::cout<<"Will use root file: "<<root_file<<std::endl;
    std::cout<<"Will use corel file: "<<corel_file<<std::endl;

    //now open and read the (channelized) baseline visibility data
    visibility_type* bl_data = new visibility_type();
    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToRead(corel_file);
    if(status)
    {
        MHO_FileKey key;
        inter.Read(*bl_data, key);
        //std::cout<<"baseline object label = "<<blabel<<std::endl;
        std::cout<<"Total size of baseline data = "<<bl_data->GetSerializedSize()<<std::endl;
    }
    else
    {
        std::cout<<" error opening file to read"<<std::endl;
        inter.Close();
        std::exit(1);
    }
    inter.Close();

    std::size_t bl_dim[VIS_NDIM];
    bl_data->GetDimensions(bl_dim);

    //now we are going to pass the visibility data to python, and plot
    //the amp/phase of the visibilities for a particular channel


    MHO_PyContainerInterface myInterface;
    myInterface.SetVisibilities(bl_data);

    // start the interpreter and keep it alive the whole program!
    //TODO FIXME -- we have to do this because some modules (numpy) cannot be re-initialized
    //if the interpreter goes out of scope...to work around this we are probably going to
    //need a singleton interface class to handle the python interpreter start-up and shutdown
    //see issue: https://github.com/pybind/pybind11/issues/3112
    py::scoped_interpreter guard{};
    configure_pypath();

    std::cout<<"*************** 1st view of visibilities python **************"<<std::endl;
    {
        //load our interface module
        auto mho_test = py::module::import("mho_test");
        //call a python functioin on the interface class instance
        mho_test.attr("test_plot_visibilities")(myInterface);
    } //interpreter goes out of scope here

    std::cout<<"*************** calling python operator **************"<<std::endl;

    MHO_PyUnaryTableOperator test_op;
    std::string mod_name = "mho_operators";
    std::string op_name = "TestThreshold";
    test_op.SetModuleFunctionName(mod_name, op_name);
    test_op.SetInput(bl_data);
    test_op.Initialize();
    test_op.Execute();

    std::cout<<"*************** contents of array now **************"<<std::endl;
    for(size_t i=0; i<bl_data->GetDimension(TIME_AXIS); i++)
    {
        for(size_t j=0; j<bl_data->GetDimension(FREQ_AXIS); j++)
        {
            std::cout<<"vis(0,0,"<<i<<","<<j<<") = "<<(*bl_data)(0,0,i,j)<<std::endl;
        }
    }

    std::cout<<"*************** 2nd view of visibilities python **************"<<std::endl;
    {
        //py::scoped_interpreter guard{}; // start the interpreter and keep it alive
        //load our interface module
        auto mho_test = py::module::import("mho_test");
        //call a python functioin on the interface class instance
        mho_test.attr("test_plot_visibilities")(myInterface);
    } //interpreter goes out of scope here





    delete bl_data;

    return 0;
}
