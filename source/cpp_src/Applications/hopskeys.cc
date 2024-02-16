#include "MHO_Message.hh"

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_FileKey.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerDefinitions.hh"

using namespace hops;

//option parsing and help text library
#include "CLI11.hpp"


mho_json ConvertKey(const MHO_FileKey& key, const MHO_ContainerDictionary& cdict)
{
    mho_json obj;
    obj["sync"] = key.fSync;
    obj["label"] = key.fLabel;
    obj["object_uuid"] = key.fObjectId.as_string();
    obj["type_uuid"] = key.fTypeId.as_string();
    std::string class_name = cdict.GetClassNameFromUUID(key.fTypeId);
    obj["class_name"] = class_name;
    obj["object_name"] = std::string(key.fName, MHO_FileKeyNameLength).c_str();
    obj["object_size"] = key.fSize;
    return obj;
}

int main(int argc, char** argv)
{
    std::string input_file = "";
    std::string output_file = "";
    unsigned int nspaces = 0;
    int message_level = 0;
    bool disable_dump = false;
    bool binary_output = false;

    CLI::App app{"hopskeys"};

    app.add_option("input,-i,--input-file", input_file, "name of the input (hops) file to be inspected")->required();
    app.add_option("output,-o,--output-file", output_file, "optional name of the output file (default format: json), if not given, no output file will be created");
    // app.add_flag("-b,--binary", binary_output, "indicate that output file format should be binary data (concatenates binary key data)");
    app.add_flag("-d,--disable-dump", disable_dump, "do not dump the keys to stdout as semi-formatted ascii");
    app.add_option("-m,--message-level", message_level, "message level to be used, range: -2 (debug) to 5 (silent)");
    app.add_option("-p,--pretty-print", nspaces, "generates the json with indentations (soft tabs) consisting of the number of spaces specified, default (disabled)");

    CLI11_PARSE(app, argc, argv);

    //clamp message level
    if(message_level > 5){message_level = 5;}
    if(message_level < -2){message_level = -2;}
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);


    if(input_file == ""){msg_fatal("main", "input_file not set" << eom);}

    //the container dictionay lets us look the names of (known) file objects
    MHO_ContainerDictionary cdict;

    //now lets extract all of the object keys in the file for inspection
    std::vector< MHO_FileKey > ikeys;
    MHO_BinaryFileInterface inter;
    bool result = false;
    bool is_index_file = false;
    std::string index_ext = ".index";
    if(input_file.size() > 6 )
    {
        auto pos = input_file.find_last_of(index_ext) - (index_ext.size()-1);
        std::string tail = input_file.substr(pos);
        if(tail == index_ext){is_index_file = true;}
    }

    if(is_index_file )
    {
        //key extraction from an index file is different b/c there are no objects to skip over
        result = inter.ExtractIndexFileObjectKeys(input_file, ikeys);
    }
    else
    {
        //regular key extraction skips over the objects, just pulling keys
        result = inter.ExtractFileObjectKeys(input_file, ikeys);
    }

    if(output_file != "" && !binary_output) //dump keys to json file
    {
        //convert the entire store to json
        mho_json root;
        mho_json key_list;
        for(auto it = ikeys.begin(); it != ikeys.end(); it++)
        {
            mho_json obj = ConvertKey(*it, cdict);
            key_list.push_back(obj);
        }
        root["keys"] = key_list;

        //open and dump to file
        std::ofstream outFile(output_file.c_str(), std::ofstream::out);

        if(outFile.is_open())
        {
            if(nspaces == 0){outFile << root;}
            else{ outFile << root.dump(nspaces); }
        }
        else
        {
            msg_error("main", "could not open file: " << output_file << eom);
        }
        outFile.close();
    }

    //do this last (separate from output file) so it doesn't get mixed up with any messages
    for(auto it = ikeys.begin(); it != ikeys.end(); it++)
    {
        if(!disable_dump)
        {
            std::cout<<"key:"<<std::endl;
            std::stringstream ss1;
            ss1 << std::hex << it->fSync;
            std::cout<<"    sync: "<<ss1.str()<<std::endl;
            std::stringstream ss2;
            ss2 << std::hex << it->fLabel;
            std::cout<<"    label: "<<ss2.str()<<std::endl;
            std::cout<<"    object uuid: "<<it->fObjectId.as_string()<<std::endl;
            std::cout<<"    type uuid: "<<it->fTypeId.as_string()<<std::endl;
            std::string class_name = cdict.GetClassNameFromUUID(it->fTypeId);
            std::cout<<"    class name: "<<class_name<<std::endl;
            std::cout<<"    object name: "<<it->fName<<std::endl;
            std::cout<<"    size (bytes): "<<it->fSize<<std::endl;
            std::cout<<std::endl;
        }
    }

    return 0;
}
