#include <iostream>
#include <map>
#include <string>

#include "MHO_ContainerDictionary.hh"
#include "MHO_Message.hh"
#include "MHO_UUID.hh"

using namespace hops;

//expose the protected factory map so we can enumerate every registered type
class DumpDictionary: public MHO_ContainerDictionary
{
    public:
        DumpDictionary(){};
        ~DumpDictionary(){};

        void Dump()
        {
            //collect (name, uuid) pairs and sort by name for stable output
            std::map< std::string, std::string > entries;
            for(auto it = fFactoryMap.begin(); it != fFactoryMap.end(); ++it)
            {
                MHO_UUID uuid = it->first;
                std::string name = GetClassNameFromUUID(uuid);
                entries[name] = uuid.as_string();
            }

            std::cout << "# registered types: " << entries.size() << std::endl;
            std::cout << "# uuid                                   class_name" << std::endl;
            for(auto it = entries.begin(); it != entries.end(); ++it)
            {
                std::cout << it->second << "  " << it->first << std::endl;
            }
        }
};

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eWarning);

    DumpDictionary dict;
    dict.Dump();
    return 0;
}
