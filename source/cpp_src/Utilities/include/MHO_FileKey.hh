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
static constexpr uint32_t MHO_FileKeyNameLength = 16;

//total size 512 bits / 64 bytes
class MHO_FileKey
{
    public:
        MHO_FileKey()
        {
            fSync = 0;
            fLabel = 0;
            for(uint32_t i=0; i<MHO_FileKeyNameLength; i++)
            {
                fName[i] = '\0';
            }
            fSize = 0;
        }

        MHO_FileKey(const MHO_FileKey& copy)
        {
            fSync = copy.fSync;
            fLabel = copy.fLabel;
            fObjectId = copy.fObjectId;
            fTypeId = copy.fTypeId;
            for(uint32_t i=0; i<MHO_FileKeyNameLength; i++)
            {
                fName[i] = copy.fName[i];
            }
            fSize = copy.fSize;
        }

        virtual ~MHO_FileKey(){};

        bool IsEmpty()
        {
            if(fSync){return false;}
            if(fLabel){return false;}
            for(uint32_t i=0; i<MHO_FileKeyNameLength; i++)
            {
                if(fName[i] != '\0'){return false;};
            }
            for(std::size_t i=0; i<MHO_UUID_LENGTH; i++)
            {
                if(fObjectId[i] != 0){return false;}
            }
            for(std::size_t i=0; i<MHO_UUID_LENGTH; i++)
            {
                if(fTypeId[i] != 0){return false;}
            }
            if(fSize){return false;}
            return true;
        }

        bool operator==(const MHO_FileKey& rhs)
        {
            if(fSync != rhs.fSync){return false;}
            if(fLabel != rhs.fLabel){return false;}
            if(fObjectId != rhs.fObjectId){return false;}
            if(fTypeId != rhs.fTypeId){return false;}

            for(uint32_t i=0; i<MHO_FileKeyNameLength; i++)
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


        MHO_FileKey& operator=(const MHO_FileKey& rhs)
        {
            if(this != &rhs)
            {
                fSync = rhs.fSync;
                fLabel = rhs.fLabel;
                fObjectId = rhs.fObjectId;
                fTypeId = rhs.fTypeId;
                for(uint32_t i=0; i<MHO_FileKeyNameLength; i++)
                {
                    fName[i] = rhs.fName[i];
                }
                fSize = rhs.fSize;


            }
            return *this;
        }

        //this is the size of a MHO_FileKey on disk
        //DO NOT USE sizeof(), as that is the size of the object in memory --
        //including compiler dependent padding!!
        static uint64_t ByteSize(){return 64;};

    //public access to members:
    
        uint32_t fSync; //32 bits for sync word for location of object key
        uint32_t fLabel; //32 bits for user/developer assigned labels
        MHO_UUID fObjectId; //128 bits for random (or otherwise determined) unique object ID
        MHO_UUID fTypeId; //128 bits for full MD5 hash of class-type + version
        char     fName[MHO_FileKeyNameLength]; //16 bytes for a (shorthand) name (i.e.probably to replace type_XXX codes)
        uint64_t fSize; //total number of bytes of incoming object (distance in bytes to next key)


};


template<typename XStream> XStream& operator>>(XStream& s, MHO_FileKey& aKey)
{
    s >> aKey.fSync;
    s >> aKey.fLabel;
    s >> aKey.fObjectId;
    s >> aKey.fTypeId;
    for(uint32_t i=0; i<MHO_FileKeyNameLength; i++)
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
    for(uint32_t i=0; i<MHO_FileKeyNameLength; i++)
    {
        s << aKey.fName[i];
    }
    s << aKey.fSize;
    return s;
}


}//end of hops

#endif /* end of include guard: MHO_FileKey */
