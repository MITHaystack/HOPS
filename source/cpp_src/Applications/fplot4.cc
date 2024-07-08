#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <getopt.h>

//option parsing and help text library
#include "CLI11.hpp"

#include "MHO_Message.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Tokenizer.hh"

//pybind11 stuff to interface with python
#ifdef USE_PYBIND11
    #include <pybind11/pybind11.h>
    #include <pybind11/embed.h>
    #include "pybind11_json/pybind11_json.hpp"
    namespace py = pybind11;
    namespace nl = nlohmann;
    using namespace pybind11::literals;
    #include "MHO_PythonOperatorBuilder.hh"
    #include "MHO_PyConfigurePath.hh"
#endif



using namespace hops;


bool extract_plot_data(mho_json& plot_data, std::string filename)
{
    mho_json fsum;
    if(filename.find("frng") == std::string::npos)
    {
        //not a fringe file, skip this
        msg_error("fringe", "the file: "<< filename<<" is not a fringe file"<<eom);
        return fsum;
    }

    //split the filename (not strictly necessary, but used to extract the extent number)
    //for example: 23 in 'GE.X.XX.345F47.23.frng'
    MHO_Tokenizer tokenizer;
    tokenizer.SetDelimiter(".");
    tokenizer.SetIncludeEmptyTokensFalse();
    tokenizer.SetString(&filename);
    std::vector< std::string > tokens;
    tokenizer.GetTokens(&tokens);
    if(tokens.size() != 6)
    {
        //not a fringe file, skip this
        msg_error("fringe", "could not parse the file name: "<< filename<<eom);
        return fsum;
    }


    //to pull out fringe data, we are primarily interested in the 'MHO_ObjectTags' object
    //get uuid for MHO_ObjectTags object
    MHO_ContainerDictionary cdict;
    MHO_UUID tag_uuid = cdict.GetUUIDFor<MHO_ObjectTags>();

    //pull all the keys and byte offsets for each object
    std::vector< MHO_FileKey > ikeys;
    std::vector< std::size_t > byte_offsets;
    MHO_BinaryFileInterface inter;
    inter.ExtractFileObjectKeysAndOffsets(filename, ikeys, byte_offsets);

    //loop over keys and offsets, looking for tags offset
    bool found = false;
    std::size_t offset_bytes = 0;
    for(std::size_t i=0; i<ikeys.size(); i++)
    {
        if(ikeys[i].fTypeId == tag_uuid)
        {
            offset_bytes = byte_offsets[i];
            found = true;
            break; //only first tag object is used
        }
    }

    if(found)
    {
        inter.OpenToReadAtOffset(filename, offset_bytes);
        MHO_ObjectTags obj;
        MHO_FileKey obj_key;
        //we read the tags object
        bool ok = inter.Read(obj, obj_key);
        if(ok)
        {
            //pull the plot data
            bool plot_ok = obj.GetTagValue("plot_data", plot_data);
            inter.Close();
            if(plot_ok){return true;}
            return false;
        }
        else
        {
            msg_error("fringe", "could not read MHO_ObjectTags from: "<< filename << eom);
            inter.Close();
            return false;
        }

    }
    else
    {
        msg_error("fringe", "no MHO_ObjectTags object found in file: "<< filename << eom);
        return false;
    }

}


int main(int argc, char** argv)
{
    #ifdef USE_PYBIND11
    //start the interpreter and keep it alive, need this or we segfault
    //each process has its own interpreter
    py::scoped_interpreter guard{};
    configure_pypath();
    #endif

    if(argc != 2){std::cout<<"filename argument missing"<<std::endl; std::exit(1);}

    //temporary...just one file for now
    std::string filename(argv[1]);

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    mho_json plot_data;
    bool ok = extract_plot_data(plot_data, filename);

    //call the plotting mechanism
    if(ok)
    {

        #ifdef USE_PYBIND11
        msg_debug("main", "python plot generation enabled." << eom );
        py::dict plot_obj = plot_data;

        ////////////////////////////////////////////////////////////////////////
        //load our interface module -- this is extremely slow!
        auto vis_module = py::module::import("hops_visualization");
        auto plot_lib = vis_module.attr("fourfit_plot");
        //call a python function on the interface class instance
        //TODO, pass filename to save plot if needed
        plot_lib.attr("make_fourfit_plot")(plot_obj, true, "");

        #else //USE_PYBIND11
            msg_warn("main", "fplot is not enabled since HOPS was built without pybind11 support." << eom);
        #endif
    }


    return 0;
}
