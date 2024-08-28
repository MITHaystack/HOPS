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
    MHO_UUID a_test_uuid = ugen.GenerateUUID();
    MHO_UUID a_test_uuid2 = ugen.GenerateUUID();

    test.AddObjectUUID(a_test_uuid);
    test.AddObjectUUID(a_test_uuid2);

    test.SetTagValue("tag1", std::string("val1") );
    test.SetTagValue("tag2", 1);
    test.SetTagValue("tag3", 3.14);
    test.SetTagValue("tag4", 'U');
    test.SetTagValue("tag5", true);
    test.SetTagValue("tag1", std::string("val1-1")); //reset tag1

    std::string filename = "./test-tags.bin";

    std::cout<<" number of bytes of this object: "<< test.GetSerializedSize()<<std::endl;

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
        std::cout<<"error opening file"<<std::endl;
    }

    inter.Close();



    //now open file in append mode and add another tag object

    MHO_ObjectTags test2;

    MHO_UUID a_test_uuid3 = ugen.GenerateUUID();
    MHO_UUID a_test_uuid4 = ugen.GenerateUUID();

    test2.AddObjectUUID(a_test_uuid3);
    test2.AddObjectUUID(a_test_uuid4);

    test2.SetTagValue("tag10", std::string("val1") );
    test2.SetTagValue("tag20", 1);
    test2.SetTagValue("tag30", 3.14);
    test2.SetTagValue("tag40", 'U');
    test2.SetTagValue("tag50", true);
    test2.SetTagValue("tag10", std::string("val1-1")); //reset tag10


    std::cout<<" number of bytes of this object: "<< test2.GetSerializedSize()<<std::endl;

    status = inter.OpenToAppend(filename);
    if(status)
    {
        std::string shortname = "tags2";
        inter.Write(test2, shortname);
        inter.Close();
    }
    else
    {
        std::cout<<"error opening file"<<std::endl;
    }

    inter.Close();


    std::cout<<"-------- Now testing read back of object --------- "<<std::endl;

    MHO_ObjectTags test3;
    MHO_ObjectTags test4;
    status = inter.OpenToRead(filename);
    if(status)
    {
        MHO_FileKey key;
        inter.Read(test3, key);

        std::vector< std::string > keys;
        test3.DumpTags(keys);
        for(auto it = keys.begin(); it != keys.end(); it++)
        {
            std::cout<<"key = "<<*it<<std::endl;
            //std::cout<<"key value type = "<< test3.GetTagValueType(*it) << std::endl;
            std::cout<<"key value = "<<test3.GetTagValueAsString(*it) <<std::endl;
        }

        inter.Read(test4, key);
        keys.clear();
        test4.DumpTags(keys);
        for(auto it = keys.begin(); it != keys.end(); it++)
        {
            std::cout<<"key = "<<*it<<std::endl;
            //std::cout<<"key value type = "<< test4.GetTagValueType(*it) << std::endl;
            std::cout<<"key value = "<<test4.GetTagValueAsString(*it) <<std::endl;
        }

    }
    else{std::cout<<" error opening file to read"<<std::endl; }

    inter.Close();


    return 0;
}
