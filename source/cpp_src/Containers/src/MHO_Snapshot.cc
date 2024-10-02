#include "MHO_Snapshot.hh"

namespace hops
{

//initialization to nullptr
MHO_Snapshot* MHO_Snapshot::fInstance = nullptr;

void MHO_Snapshot::AddKey(const std::string& key)
{
    fKeys.insert(key);
}

void MHO_Snapshot::AddKey(const char* key)
{
    fKeys.insert(std::string(key));
}

void MHO_Snapshot::RemoveKey(const std::string& key)
{
    auto iter = fKeys.find(key);
    if(iter != fKeys.end())
    {
        fKeys.erase(iter);
    }
}

void MHO_Snapshot::RemoveKey(const char* key)
{
    std::string tmp_key(key);
    auto iter = fKeys.find(tmp_key);
    if(iter != fKeys.end())
    {
        fKeys.erase(iter);
    }
}

void MHO_Snapshot::RemoveAllKeys()
{
    fKeys.clear();
}

bool MHO_Snapshot::PassSnapshot(std::string key)
{
    if(fAcceptAllKeys)
    {
        return true;
    }
    else
    {
        fCurrentKeyIsAllowed = false;
        std::string tmp_key(key);
        auto iter = fKeys.find(tmp_key);
        if(iter != fKeys.end())
        {
            fCurrentKeyIsAllowed = true;
        }
        return fCurrentKeyIsAllowed;
    }
}

} // namespace hops
