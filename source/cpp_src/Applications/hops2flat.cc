#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerFileInterface.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_Message.hh"

#include "MHO_DirectoryInterface.hh"

#include <utility>

using namespace hops;

//option parsing and help text library
#include "CLI11.hpp"

//unspecialized template doesn't do much
template< typename XCheckType >
void
ExtractData(const MHO_Serializable* obj, std::pair< void*, std::size_t>& ptr_bsize )
{
    ptr_bsize.first = nullptr;
    ptr_bsize.second = 0;
};



//specialization for tables...TODO do we need specialization for vector/scalar containers?
template< typename XCheckType = XContainerType >
typename std::enable_if< std::is_base_of< MHO_TableContainerBase, XCheckType >::value, void >::type
ExtractData(const MHO_Serializable* obj, std::pair< void*, std::size_t>& ptr_bsize )
{
    ptr_bsize.first = nullptr;
    ptr_bsize.second = 0;
    XContainerType* ptr = std::dynamic_cast< XContainerType* >(obj);
    if(obj == nullptr){return;}
    ptr_bsize.first = std::reinterpret_cast< void* >( ptr->GetData() );
    if(ptr_bsize.first == nullptr ){return;}
    std::size_t element_bsize = sizeof(XContainerType::value_type);
    std::size_t nelem = ptr->GetSize();
    ptr_bsize.second = nelem*element_bsize;
};


int main(int argc, char** argv)
{
    std::string input_file = "";
    std::string output_dir = "";
    //meta data detail level
    int detail = eJSONAxesWithLabelsLevel;
    unsigned int nspaces = 0;
    int message_level = 0;

    CLI::App app{"hops2flat"};

    // app.add_option("-d,--detail", detail,
    //                "level of detail to be used when generating the output, range: 0 (low) to 4 (high), default (4)");
    app.add_option("-m,--message-level", message_level, "message level to be used, range: -2 (debug) to 5 (silent)");
    app.add_option("-p,--pretty-print", nspaces,
                   "generates the json meta-data ouput with indentations (soft tabs) consisting of the number of spaces specified, "
                   "default (disabled)");
    // app.add_option("-u,--uuid", uuid_string, "specify and extract a single object by UUID");
    // app.add_option("-s,--shortname", shortname,
    //                "specify and extract a single object by shortname (returns first matching object) ")
    //     ->excludes("--uuid");
    app.add_option("input,-i,--input-file", input_file, "name of the input (hops) file to be converted")->required();
    app.add_option("output,-o,--output-dir", output_dir,
                   "name of the output directory, if not given the result will be stored in <input-file>.flat");

    CLI11_PARSE(app, argc, argv);

    //clamp message level
    if(message_level > 5)
    {
        message_level = 5;
    }
    if(message_level < -2)
    {
        message_level = -2;
    }
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    if(input_file == "")
    {
        msg_fatal("main", "input_file not set" << eom);
        return 1;
    }

    //set default output name, if not passed
    if(output_dir == "")
    {
        output_dir = input_file + ".flat";
    }


    //create the output directory if it doesn't exist
    bool dir_ok = false;
    MHO_DirectoryInterface dirInterface;
    dir_ok = dirInterface.DoesDirectoryExist(output_dir);
    if( !dir_ok )
    {
        dir_ok = dirInterface.CreateDirectory(output_dir);
    }
    
    if(!dir_ok)
    {
        msg_fatal("main", "could not create output directory: "<< output_dir << eom);
        return 1;
    }

    //read in all the objects in a file
    MHO_ContainerStore conStore;
    MHO_ContainerFileInterface conInter;
    conInter.SetFilename(input_file);
    conInter.PopulateStoreFromFile(conStore);
    
    //loop over all the objects in the container, and convert them 
    //into a meta-data .json file, and a flat binary file for the data table
    
    //get all the types in the container
    std::vector< MHO_UUID > type_ids;
    conStore.GetAllTypeUUIDs(type_ids);
    for(std::size_t tid=0; tid<type_ids.size(); tid++)
    {
        //get all the object uuids of this type
        std::vector< MHO_UUID > obj_ids;
        conStore.GetAllObjectUUIDsOfType(type_ids[tid], obj_ids);
        for(std::size_t oid=0; oid<obj_ids.size(); oid++)
        {
            MHO_UUID obj_uuid = obj_ids[oid];
            //convert the selected object to json at the current detail level
            mho_json obj_json;
            conInter.ConvertObjectInStoreToJSON(conStore, obj_uuid, obj_json, detail);
            
            //construct the file name for this object 
            std::string meta_file = output_dir + "/" + obj_uuid.as_string() + ".meta.json";

            //open and dump the meta data to file
            std::ofstream outFile(meta_file.c_str(), std::ofstream::out);
            if(outFile.is_open())
            {
                if(nspaces == 0)
                {
                    outFile << obj_json;
                }
                else
                {
                    outFile << obj_json.dump(nspaces);
                }
            }
            else
            {
                msg_error("main", "could not open file: " << meta_file << eom);
            }
            outFile.close();
            
            //now we want to extract the table data and write it out as a flat binary file 
            MHO_Serializable* obj_ptr = conStore.GetObject(obj_uuid);
            
            //TODO -- we want to write the numpy dtype code into the name
            std::string bin_file = output_dir + "/" + obj_uuid.as_string() + ".bin";
            std::pair< void*, std::size_t> ptr_bsize;
            ExtractData(obj_ptr, ptr_bsize );
            
            if(ptr_bsize.first != nullptr && ptr_bsize.second != 0)
            {
            
                if(ptr_bsize.first != nullptr)
                {
                    std::cout<<"data ptr = "<<ptr_bsize.first<<std::endl;
                }
                if(ptr_bsize.second != 0)
                {
                    std::cout<<"data size (bytes) = "<<ptr_bsize.second<<std::endl;
                }
            
            }

        }
    }



    msg_info("main", "done" << eom);

    return 0;
}
