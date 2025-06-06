#include <getopt.h>
#include <vector>

#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerFileInterface.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_Message.hh"

using namespace hops;

class LocalDictionary: public MHO_ContainerDictionary
{
    public:
        LocalDictionary(){};

        virtual ~LocalDictionary()
        {
            for(auto it = fAllObjects.begin(); it != fAllObjects.end(); it++)
            {
                MHO_Serializable* obj = it->second;
                delete obj;
            }
        };

        void MakeAnObjectOfEveryType()
        {
            for(auto it = fFactoryMap.begin(); it != fFactoryMap.end(); it++)
            {
                MHO_UUID uuid = it->first;
                MHO_Serializable* obj = it->second->Build();
                fAllObjects.emplace(uuid, obj);
                fAllObjectTypeNames.emplace(uuid, GetClassNameFromUUID(uuid));
            }
        }

        void PopulateStore(MHO_ContainerStore& lib)
        {
            MHO_UUIDGenerator gen;
            for(auto it = fAllObjects.begin(); it != fAllObjects.end(); it++)
            {
                MHO_UUID type_uuid = it->first;
                MHO_Serializable* obj = it->second;
                MHO_UUID object_uuid = obj->GetObjectUUID(); //gen.GenerateUUID(); //random object id
                std::cout << type_uuid.as_string() << " - " << object_uuid.as_string() << std::endl;
                std::cout << type_uuid.as_string() << " - " << fAllObjectTypeNames[type_uuid] << std::endl;
                std::cout << "object thinks its type uuid = " << obj->GetTypeUUID().as_string() << std::endl;
                std::string shortname = "test";
                uint32_t label = 1;
                // bool ok = lib.AddContainerObject(obj, type_uuid, object_uuid, shortname, label);
                bool ok = lib.AddObject(obj); // type_uuid, object_uuid, shortname, label);
            }
            fAllObjects.clear(); // we've handed ownership of these objects off to the container library
        }

    private:
        std::map< MHO_UUID, MHO_Serializable* > fAllObjects;
        std::map< MHO_UUID, std::string > fAllObjectTypeNames;
};

int main(int argc, char** argv)
{
    std::string usage = "TestContainerNames -f <output_file>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string filename = "./test-container-names.bin";

    static struct option longOptions[] = {
        {"help", no_argument,       0, 'h'},
        {"file", required_argument, 0, 'f'}
    };

    static const char* optString = "hf:";

    while(true)
    {
        char optId = getopt_long(argc, argv, optString, longOptions, NULL);
        if(optId == -1)
            break;
        switch(optId)
        {
            case('h'): // help
                std::cout << usage << std::endl;
                return 0;
            case('f'):
                filename = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    LocalDictionary conDict;
    MHO_ContainerStore conStore;

    conDict.MakeAnObjectOfEveryType();
    conDict.PopulateStore(conStore);

    //all file objects are now in memory
    std::size_t n_objs_start = conStore.GetNObjects();
    std::cout << "store has: " << n_objs_start << " objects." << std::endl;

    MHO_ContainerFileInterface conInter;
    conInter.SetFilename(filename);
    conInter.WriteStoreToFile(conStore);

    MHO_ContainerStore conStore2;
    conInter.SetFilename(filename);
    conInter.PopulateStoreFromFile(conStore2);

    //all file objects are now in memory
    std::size_t n_objs_end = conStore2.GetNObjects();
    std::cout << "store has: " << n_objs_end << " objects." << std::endl;

    //explicitly delete the visibility type object;
    std::vector< MHO_UUID > obj_ids;
    MHO_UUID vis_type_id = conStore2.GetTypeUUID< visibility_type >();
    conStore2.GetAllObjectUUIDsOfType(vis_type_id, obj_ids);
    for(auto it = obj_ids.begin(); it != obj_ids.end(); it++)
    {
        std::cout << "deleting object of visibility_type with object uuid: " << (*it).as_string() << std::endl;
        conStore2.DeleteObject(*it);
    }

    conStore.Clear();
    conStore2.Clear();

    if(n_objs_start == n_objs_end)
    {
        return 0;
    }         //test passes
    return 1; //test fails
}
