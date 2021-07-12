#include <iostream>
#include <string>

#include "MHO_ObjectTags.hh"
#include "MHO_BinaryFileInterface.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    MHO_ObjectTags test;

    MHO_UUIDGenerator ugen;
    MHO_UUID a_test_uuid;
    a_test_uuid = ugen.GenerateUUID();

    test.SetObjectUUID(a_test_uuid);
    test.SetTag("tag1", "val1");
    test.SetTag("tag2", "val2");
    test.SetTag("tag3", "val3");
    test.SetTag("tag4", "val4");
    test.SetTag("tag5", "val5");
    test.SetTag("tag1", "val1-1"); //reset tag1
    test.SetObjectName("test");
    std::string filename = "./test-tags.bin";

    std::cout<<" number of bytes of this object: "<< test.GetSerializedSize()<<std::endl;

    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(filename);

    if(status)
    {
        uint32_t label = 0xFF00FF00;
        std::string shortname = "tags";
        inter.Write(test, shortname, label);
        inter.Close();
    }
    else
    {
        std::cout<<"error opening file"<<std::endl;
    }

    inter.Close();

    std::cout<<"-------- Now testing read back of object --------- "<<std::endl;

    MHO_ObjectTags test2;
    status = inter.OpenToRead(filename);
    if(status)
    {
        MHO_FileKey key;
        inter.Read(test2, key);
        //std::cout<<"object label = "<<blabel<<std::endl;

        std::vector< std::pair<std::string, std::string> > tv_pairs;
        test2.DumpTagValuePairs(tv_pairs);
        for(auto it = tv_pairs.begin(); it != tv_pairs.end(); it++)
        {
            std::cout<<"tag:value = "<<it->first<<" : "<<it->second<<std::endl;
        }
    }
    else
    {
        std::cout<<" error opening file to read"<<std::endl;
    }

    inter.Close();


    return 0;
}
