#include "MHO_Snapshot.hh"

namespace hops
{



//initialization to nullptr
MHO_Snapshot* MHO_Snapshot::fInstance = nullptr;

void
MHO_Snapshot::AddKey(const std::string& key)
{
    fKeys.insert(key);
}

void
MHO_Snapshot::AddKey(const char* key)
{
    fKeys.insert(std::string(key));
}

void
MHO_Snapshot::RemoveKey(const std::string& key)
{
    auto iter = fKeys.find(key);
    if(iter != fKeys.end())
    {
        fKeys.erase(iter);
    }
}


void
MHO_Snapshot::RemoveKey(const char* key)
{
    std::string tmp_key(key);
    auto iter = fKeys.find(tmp_key);
    if(iter != fKeys.end())
    {
        fKeys.erase(iter);
    }
}

void
MHO_Snapshot::RemoveAllKeys()
{
    fKeys.clear();
}


MHO_Snapshot&
MHO_Snapshot::SendSnapshot(const MHO_SnapshotLevel& level, const std::string& key)
{
    fCurrentLevel = level;
    fCurrentKeyIsAllowed = false;
    auto iter = fKeys.find(key);
    if(iter != fKeys.end()){fCurrentKeyIsAllowed = true;}

    if( PassSnapshot() )
    {
        fSnapshotStream << GetCurrentPrefix(level,key);
    }

    return *fInstance;
}

MHO_Snapshot&
MHO_Snapshot::SendSnapshot(const MHO_SnapshotLevel& level, const char* key)
{
    fCurrentLevel = level;
    fCurrentKeyIsAllowed = false;
    std::string tmp_key(key);
    auto iter = fKeys.find(tmp_key);
    if(iter != fKeys.end()){fCurrentKeyIsAllowed = true;}

    if( PassSnapshot() )
    {
        fSnapshotStream << GetCurrentPrefix(level, tmp_key);
    }

    return *fInstance;
}


bool
MHO_Snapshot::PassSnapshot()
{
    return ( fCurrentKeyIsAllowed || fAcceptAllKeys );
}




}
