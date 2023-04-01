#include <string>
#include <sstream>

//global messaging util
#include "MHO_Message.hh"

//handles reading directories, listing files etc.
#include "MHO_DirectoryInterface.hh"

//needed to read hops files and extract objects
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerFileInterface.hh"

namespace hops
{

template< typename XObjType >
void
dump_object(XObjType* obj, std::string name, std::string filename, std::string dir)
{
    //dump bl_data into a file for later inspection
    std::stringstream ss;
    ss << dir;
    ss << "/";
    ss << filename;

    std::string output_file = ss.str();
    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(output_file);
    if(status)
    {
        uint32_t label = 0xFFFFFFFF; //placeholder
        inter.Write(*obj, name, label);
    }
    else
    {
        msg_error("file", "error writing object "<< name << " to file: " << output_file << eom);
    }
    inter.Close();
}


}
