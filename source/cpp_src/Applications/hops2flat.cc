#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerFileInterface.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_Message.hh"

#include "MHO_DirectoryInterface.hh"

#include <fstream>
#include <utility>

using namespace hops;

//option parsing and help text library
#include "CLI11.hpp"

void write_to_binfile(const char* data, std::size_t size, const std::string& filename)
{
    std::ofstream outFile(filename, std::ios::binary);
    if(!outFile)
    {
        msg_error("main", "could not open file: " + filename << eom);
    }
    outFile.write(data, size);
    outFile.close();
}

void write_to_jsonfile(mho_json& obj, const std::string& meta_file, int nspaces)
{
    //open and dump the meta data to file
    std::ofstream outFile(meta_file.c_str(), std::ofstream::out);
    if(outFile.is_open())
    {
        if(nspaces == 0)
        {
            outFile << obj;
        }
        else
        {
            outFile << obj.dump(nspaces);
        }
    }
    else
    {
        msg_error("main", "could not open file: " << meta_file << eom);
    }
    outFile.close();
}

void write_to_cborfile(mho_json& obj, const std::string& meta_file)
{
    //open and dump the meta data to file
    std::ofstream outFile(meta_file.c_str(), std::ofstream::out);
    if(outFile.is_open())
    {
        //serialize to CBOR and write
        std::vector< std::uint8_t > cbor_data = mho_json::to_cbor(obj);
        outFile.write(reinterpret_cast< const char* >(cbor_data.data()), cbor_data.size());
    }
    else
    {
        msg_error("main", "could not open file: " << meta_file << eom);
    }
    outFile.close();
}

int main(int argc, char** argv)
{
    std::string input_file = "";
    std::string output_dir = "";
    //meta data detail level
    int detail = eJSONAxesWithLabelsLevel;
    std::string uuid_string = "";
    std::string shortname = "";
    unsigned int nspaces = 0;
    int message_level = 0;
    int trim_nchars = 0;
    bool cbor_output = false;

    CLI::App app{"hops2flat"};

    app.add_option("-d,--detail", detail,
                   "level of detail to be used when generating the output, range: 0 (low) to 3 (high), default (3)");
    app.add_option("-m,--message-level", message_level, "message level to be used, range: -2 (debug) to 5 (silent)");
    app.add_option(
        "-p,--pretty-print", nspaces,
        "generates the json meta-data ouput with indentations (soft tabs) consisting of the number of spaces specified, "
        "default (disabled)");
    app.add_option("-u,--uuid", uuid_string, "specify and extract a single object by UUID");
    app.add_option("-s,--shortname", shortname,
                   "specify and extract a single object by shortname (returns first matching object) ")
        ->excludes("--uuid");
    app.add_option("-t,--trim-uuid", trim_nchars,
                   "specify the number of characters of the object UUID to keep in the file name");
    app.add_flag("-c,--cbor", cbor_output, "export the json meta data in CBOR binary representation")
        ->excludes("--pretty-print");

    app.add_option("input,-i,--input-file", input_file, "name of the input (hops) file to be converted")->required();
    app.add_option("output,-o,--output-dir", output_dir,
                   "name of the output directory, if not given the result will be stored in <input-file>.flat");

    CLI11_PARSE(app, argc, argv);

    //clamp detail level
    if(detail > eJSONAxesWithLabelsLevel)
    {
        detail = eJSONAxesWithLabelsLevel;
    }
    if(detail < eJSONBasicLevel)
    {
        detail = eJSONBasicLevel;
    }

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
    if(!dir_ok)
    {
        dir_ok = dirInterface.CreateDirectory(output_dir);
    }

    if(!dir_ok)
    {
        msg_fatal("main", "could not create output directory: " << output_dir << eom);
        return 1;
    }

    //read in all the objects in a file
    MHO_ContainerStore conStore;
    MHO_ContainerFileInterface conInter;
    conInter.SetFilename(input_file);
    conInter.PopulateStoreFromFile(conStore);

    //if a specific object was requested -- convert given uuid string to MHO_UUID object
    bool single_object = false;
    MHO_UUID single_obj_uuid;
    if(shortname != "")
    {
        single_obj_uuid = conStore.GetObjectUUID(shortname);
        if(single_obj_uuid.is_empty())
        {
            msg_fatal("main", "object uuid for: " << shortname << ", could not be determined" << eom);
            return 1;
        }
        single_object = true;
    }
    else if(uuid_string != "")
    {
        bool ok = single_obj_uuid.from_string(uuid_string);
        if(!ok)
        {
            msg_fatal("main", "could not convert given string into UUID key: " << uuid_string << eom);
            return 1;
        }
        single_object = true;
    }

    //loop over all the objects in the container, and convert them
    //into a meta-data .json file, and a flat binary file for the data table
    //get all the types in the container
    std::vector< MHO_UUID > type_ids;
    conStore.GetAllTypeUUIDs(type_ids);
    for(std::size_t tid = 0; tid < type_ids.size(); tid++)
    {
        //get all the object uuids of this type
        std::vector< MHO_UUID > obj_ids;
        conStore.GetAllObjectUUIDsOfType(type_ids[tid], obj_ids);
        for(std::size_t oid = 0; oid < obj_ids.size(); oid++)
        {
            MHO_UUID obj_uuid = obj_ids[oid];
            //bail out on this one if only a single object requested, and this is not a match
            if(single_object && obj_uuid != single_obj_uuid)
            {
                continue;
            }

            std::string shortname = conStore.GetShortName(obj_uuid);
            std::string obj_ident = obj_uuid.as_string();
            //trim the object ident as needed
            if(trim_nchars != 0)
            {
                if(obj_ident.size() > trim_nchars)
                {
                    obj_ident.resize(trim_nchars);
                }
            }

            //convert the selected object to json
            mho_json obj_json;

            //needed to extract raw table data (if we can)
            std::size_t rank;
            const char* raw_data;
            std::size_t raw_data_byte_size;
            std::string raw_data_descriptor;

            conInter.ConvertObjectInStoreToJSONAndRaw(conStore, obj_uuid, obj_json, rank, raw_data, raw_data_byte_size,
                                                      raw_data_descriptor, detail);

            if(!cbor_output)
            {
                //construct the file name for the meta data of this object
                std::string meta_file = output_dir + "/" + shortname + "." + obj_ident + ".meta.json";
                write_to_jsonfile(obj_json, meta_file, nspaces);
            }
            else
            {
                //construct the file name for the meta data of this object
                std::string meta_file = output_dir + "/" + shortname + "." + obj_ident + ".meta.cbor";
                write_to_cborfile(obj_json, meta_file);
            }

            //now we can write out the raw table data (if it exists) in one big chunk
            if(raw_data != nullptr && raw_data_byte_size != 0 && raw_data_descriptor != "")
            {
                std::string bin_file = output_dir + "/" + shortname + "." + obj_ident + "." + raw_data_descriptor + ".bin";
                write_to_binfile(raw_data, raw_data_byte_size, bin_file);
            }
        }
    }

    msg_info("main", "done" << eom);

    return 0;
}
