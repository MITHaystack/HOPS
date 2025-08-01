#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerFileInterface.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_Message.hh"

using namespace hops;

//option parsing and help text library
#include "CLI11.hpp"

int main(int argc, char** argv)
{
    std::string input_file = "";
    std::string output_file = "";
    std::string uuid_string = "";
    std::string shortname = "";
    int detail = eJSONAll;
    unsigned int nspaces = 0;
    int message_level = 0;

    CLI::App app{"hops2json"};

    app.add_option("-d,--detail", detail,
                   "level of detail to be used when generating the output, range: 0 (low) to 4 (high), default (4)");
    app.add_option("-m,--message-level", message_level, "message level to be used, range: -2 (debug) to 5 (silent)");
    app.add_option("-p,--pretty-print", nspaces,
                   "generates the json ouput with indentations (soft tabs) consisting of the number of spaces specified, "
                   "default (disabled)");
    app.add_option("-u,--uuid", uuid_string, "specify and extract a single object by UUID");
    app.add_option("-s,--shortname", shortname,
                   "specify and extract a single object by shortname (returns first matching object) ")
        ->excludes("--uuid");
    app.add_option("input,-i,--input-file", input_file, "name of the input (hops) file to be converted")->required();
    app.add_option("output,-o,--output-file", output_file,
                   "name of the output (json) file, if not given the result will be stored in <input-file>.json");

    CLI11_PARSE(app, argc, argv);

    //clamp detail level
    if(detail > eJSONAll)
    {
        detail = eJSONAll;
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
    }

    //set default output name, if not passed
    if(output_file == "")
    {
        output_file = input_file + ".json";
    }

    //clamp detail level
    if(detail > 4)
    {
        detail = 4;
    }
    if(detail < 0)
    {
        detail = 0;
    }

    MHO_ContainerStore conStore;
    MHO_ContainerFileInterface conInter;
    conInter.SetFilename(input_file);
    conInter.PopulateStoreFromFile(conStore); //reads in all the objects in a file

    if(uuid_string == "" && shortname == "")
    {
        //all file objects are now in memory
        msg_status("main", "converting " << conStore.GetNObjects() << " objects" << eom);

        //convert the entire file/store to json
        mho_json root;
        conInter.ConvertStoreToJSON(conStore, root, detail);

        //open and dump to file
        std::ofstream outFile(output_file.c_str(), std::ofstream::out);

        if(outFile.is_open())
        {
            if(nspaces == 0)
            {
                outFile << root;
            }
            else
            {
                outFile << root.dump(nspaces);
            }
        }
        else
        {
            msg_error("main", "could not open file: " << output_file << eom);
        }
        outFile.close();
    }
    else //we are only converting a single specified object (matching uuid or shortname)
    {
        //convert given uuid string to MHO_UUID object
        MHO_UUID obj_uuid;
        if(shortname != "")
        {
            obj_uuid = conStore.GetObjectUUID(shortname);
        }
        else if(uuid_string != "")
        {
            bool ok = obj_uuid.from_string(uuid_string);
            if(!ok)
            {
                msg_fatal("main", "could not convert given string into UUID key: " << uuid_string << eom);
                return 1;
            }
        }

        if(obj_uuid.is_empty())
        {
            msg_fatal("main", "object uuid could not be determined" << eom);
            return 1;
        }
        msg_debug("main", "looking for object with uuid key: " << obj_uuid.as_string() << eom);

        //convert the selected object to json
        mho_json obj_json;
        conInter.ConvertObjectInStoreToJSON(conStore, obj_uuid, obj_json, detail);

        //open and dump to file
        std::ofstream outFile(output_file.c_str(), std::ofstream::out);
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
            msg_error("main", "could not open file: " << output_file << eom);
        }
        outFile.close();
    }

    msg_info("main", "done" << eom);

    return 0;
}
