#ifndef MHO_FileKey_HH__
#define MHO_FileKey_HH__

/*
*File: MHO_FileKey.hh
*Class: MHO_FileKey
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <cstdint>
#include "MHO_UUID.hh"

namespace hops
{

static constexpr uint32_t MHO_FileKeySyncWord = 0xEFBEADDE; //DEADBEEF
static constexpr uint32_t MHO_FikeKeyNameLength = 16;

//total size 384 bits / 64 bytes
struct MHO_FileKey
{
    uint32_t fSync; //32 bits for sync word for location of object key
    uint32_t fLabel; //32 bits for user/developer assigned labels
    MHO_UUID fObjectId; //128 bits for random (or otherwise determined) unique object ID
    MHO_UUID fTypeId; //128 bits for full MD5 hash of class-type + version
    char     fName[MHO_FikeKeyNameLength]; //16 bytes for a (shorthand) name (i.e.probably to replace type_XXX codes)
    uint64_t fSize; //total number of bytes of incoming object (distance in bytes to next key)

    bool operator==(const MHO_FileKey& rhs)
    {
        if(fSync != rhs.fSync){return false;}
        if(fLabel != rhs.fLabel){return false;}
        if(fObjectId != rhs.fObjectId){return false;}
        if(fTypeId != rhs.fTypeId){return false;}

        for(uint32_t i=0; i<MHO_FikeKeyNameLength; i++)
        {
            if(fName[i] != rhs.fName[i]){return false;}
        }

        if(fSize != rhs.fSize){return false;}
        return true;
    }

    bool operator!=(const MHO_FileKey& rhs)
    {
        return !(*this == rhs);
    }

};


template<typename XStream> XStream& operator>>(XStream& s, MHO_FileKey& aKey)
{
    s >> aKey.fSync;
    s >> aKey.fLabel;
    s >> aKey.fObjectId;
    s >> aKey.fTypeId;
    for(uint32_t i=0; i<MHO_FikeKeyNameLength; i++)
    {
        s >> aKey.fName[i];
    }
    s >> aKey.fSize;
    return s;
}

template<typename XStream> XStream& operator<<(XStream& s, const MHO_FileKey& aKey)
{
    s << aKey.fSync;
    s << aKey.fLabel;
    s << aKey.fObjectId;
    s << aKey.fTypeId;
    for(uint32_t i=0; i<MHO_FikeKeyNameLength; i++)
    {
        s << aKey.fName[i];
    }
    s << aKey.fSize;
    return s;
}


}//end of hops

#endif /* end of include guard: MHO_FileKey */
