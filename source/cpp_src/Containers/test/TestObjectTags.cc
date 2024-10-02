#include <iostream>
#include <string>

#include "MHO_BinaryFileInterface.hh"
#include "MHO_ObjectTags.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    MHO_ObjectTags test;

    MHO_UUIDGenerator ugen;
    MHO_UUID a_test_uuid = ugen.GenerateUUID();
    MHO_UUID a_test_uuid2 = ugen.GenerateUUID();

    test.AddObjectUUID(a_test_uuid);
    test.AddObjectUUID(a_test_uuid2);

    test.SetTagValue("tag1", std::string("val1"));
    test.SetTagValue("tag2", 1);
    test.SetTagValue("tag3", 3.14);
    test.SetTagValue("tag4", 'U');
    test.SetTagValue("tag5", true);
    test.SetTagValue("tag1", std::string("val1-1")); //reset tag1

    std::string filename = "./test-tags.bin";

    std::cout << " number of bytes of this object: " << test.GetSerializedSize() << std::endl;

    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(filename);

    if(status)
    {
        std::string shortname = "tags";
        inter.Write(test, shortname);
        inter.Close();
    }
    else
    {
        std::cout << "error opening file" << std::endl;
    }

    inter.Close();

    std::cout << "-------- Now testing read back of object --------- " << std::endl;

    MHO_ObjectTags test2;
    status = inter.OpenToRead(filename);
    if(status)
    {
        MHO_FileKey key;
        inter.Read(test2, key);

        std::vector< std::string > keys;
        test2.DumpTags(keys);
        for(auto it = keys.begin(); it != keys.end(); it++)
        {
            std::cout << "key = " << *it << std::endl;
            // std::cout<<"key value type = "<< test2.GetTagValueType(*it) << std::endl;
            std::cout << "key value = " << test2.GetTagValueAsString(*it) << std::endl;
        }
    }
    else
    {
        std::cout << " error opening file to read" << std::endl;
    }

    inter.Close();

    return 0;
}
