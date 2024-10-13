#ifndef MHO_FileKey_HH__
#define MHO_FileKey_HH__

#include "MHO_Types.hh"
#include "MHO_UUID.hh"

namespace hops
{

/*!
 *@file MHO_FileKey.hh
 *@class MHO_FileKey
 *@author J. Barrett - barrettj@mit.edu
 *@date Wed Apr 21 13:40:18 2021 -0400
 *@brief class for file object keys to locate and describe objects in a hops file
 */

//fixed values
static constexpr uint32_t MHO_FileKeySyncWord = 0xEFBEADDE; //DEADBEEF
static constexpr uint32_t MHO_FileKeyNameLength = 16;

//v0 parameters
static constexpr uint16_t MHO_FileKeyVersion_v0 = 0;
static constexpr uint16_t MHO_FileKeySize_v0 = 64;

//points to the current version
static constexpr uint16_t MHO_FileKeyVersion = MHO_FileKeyVersion_v0;
static constexpr uint16_t MHO_FileKeySize = MHO_FileKeySize_v0;

//reserved label
static constexpr uint32_t MHO_FileKeyVersionReserved = 0xFFFFFFFF;

//this union allows us to store the file key version and size info into a 4 byte integer
union MHO_FileKeyVersionInfo
{
        uint32_t fLabel;
        uint16_t fVersionSize[2];
        //fVersionSize format is:
        //1st uint16_t is the file key version
        //2nd uint16_t is the size of the file key, (cannot exceed UINT16_MAX)
};

//the version-0 size of the file key is 64bytes, and all of the version-0 data fields must
//be present and structure unchanged in any future versions.
//A future version X of MHO_FileKey may extend the file key by N bytes of arbitrarily structured data to
//be inserted (after the end of the V0 key and before the file object it describes,
//so long as N+64 < UINT16_MAX, and the appropriate StreamInData_VX/StreamOutData_VX and ByteSize functions are defined

//total size 512 bits / 64 bytes
class MHO_FileKey
{
    public:
        MHO_FileKey()
        {
            //sync word is always the same
            fSync = MHO_FileKeySyncWord;
            //define the current file key version/size
            MHO_FileKeyVersionInfo u;
            u.fVersionSize[0] = MHO_FileKeyVersion;
            u.fVersionSize[1] = MHO_FileKeySize;
            fLabel = u.fLabel;

            for(uint32_t i = 0; i < MHO_FileKeyNameLength; i++)
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
            for(uint32_t i = 0; i < MHO_FileKeyNameLength; i++)
            {
                fName[i] = copy.fName[i];
            }
            fSize = copy.fSize;
        }

        virtual ~MHO_FileKey(){};

        bool operator==(const MHO_FileKey& rhs)
        {
            if(fSync != rhs.fSync)
            {
                return false;
            }
            if(fLabel != rhs.fLabel)
            {
                return false;
            }
            if(fObjectId != rhs.fObjectId)
            {
                return false;
            }
            if(fTypeId != rhs.fTypeId)
            {
                return false;
            }

            for(uint32_t i = 0; i < MHO_FileKeyNameLength; i++)
            {
                if(fName[i] != rhs.fName[i])
                {
                    return false;
                }
            }

            if(fSize != rhs.fSize)
            {
                return false;
            }
            return true;
        }

        bool operator!=(const MHO_FileKey& rhs) { return !(*this == rhs); }

        MHO_FileKey& operator=(const MHO_FileKey& rhs)
        {
            if(this != &rhs)
            {
                fSync = rhs.fSync;
                fLabel = rhs.fLabel;
                fObjectId = rhs.fObjectId;
                fTypeId = rhs.fTypeId;
                for(uint32_t i = 0; i < MHO_FileKeyNameLength; i++)
                {
                    fName[i] = rhs.fName[i];
                }
                fSize = rhs.fSize;
            }
            return *this;
        }

        //this is the size of a MHO_FileKey on disk
        //DO NOT USE sizeof(MHO_FileKey), as that is the size of the object in memory
        //which includes compiler dependent padding!!
        static uint64_t ByteSize() { return MHO_FileKeySize_v0; };

        //public access to members:
    public:
        uint32_t fSync;                    //32 bits for sync word for location of object key
        uint32_t fLabel;                   //32 bits for version/size label
        MHO_UUID fObjectId;                //128 bits for random (or otherwise determined) unique object ID
        MHO_UUID fTypeId;                  //128 bits for full MD5 hash of class-type + version
        char fName[MHO_FileKeyNameLength]; //16 bytes for a (shorthand) name (i.e.probably to replace type_XXX codes)
        uint64_t fSize;                    //total number of bytes of incoming object (distance in bytes to next key)

        //Future version of the file key could include additional information/parameters here
        //However, version/size flag must be defined and additional I/O functionality must be added.
        //For example, we could include space for a checksum of the object data, or additional tags/meta-data.

        ////////////////////////////////////////////////////////////////////////////
        //streaming functions and I/O defined below

        template< typename XStream > friend XStream& operator>>(XStream& s, MHO_FileKey& aData)
        {
            s >> aData.fSync;
            s >> aData.fLabel;

            //the reserved 0xFFFFFFFF label forces us to use the current key version and size
            //regardless of what is on disk, use with caution!
            if(aData.fLabel == MHO_FileKeyVersionReserved)
            {
                MHO_FileKeyVersionInfo r;
                r.fVersionSize[0] = MHO_FileKeyVersion;
                r.fVersionSize[1] = MHO_FileKeySize;
                aData.fLabel = r.fLabel;
            }

            MHO_FileKeyVersionInfo u;
            u.fLabel = aData.fLabel;
            uint16_t version = u.fVersionSize[0];
            uint16_t vsize = u.fVersionSize[1];

            switch(version)
            {
                case 0:
                    aData.StreamInData_V0(s);
                    break;
                default:
                    aData.StreamInData_V0(s); //version-0 data must always be present.
                    //However, we don't understand this file-key version, so increment the stream
                    //data for this unknown portion into the trash, if the object type is know, it will
                    //still get read, but the extended file key data will be lost
                    if(vsize > 64)
                    {
                        int n = vsize - 64;
                        char trash;
                        for(int i = 0; i < n; i++)
                        {
                            s >> trash;
                        }
                    }
                    msg_error("utility", "encountered a MHO_FileKey with unknown version: "
                                             << version << " attempting to continue. " << eom);
            }
            return s;
        }

        template< typename XStream > friend XStream& operator<<(XStream& s, const MHO_FileKey& aData)
        {
            MHO_FileKeyVersionInfo u;
            u.fLabel = aData.fLabel;
            uint16_t version = u.fVersionSize[0];
            uint16_t vsize = u.fVersionSize[1];

            s << aData.fSync;
            s << aData.fLabel;

            switch(version)
            {
                case 0:
                    aData.StreamOutData_V0(s);
                    break;
                case 0xFFFF: //reserved case, use the current streamer version (v0 at the moment)
                    aData.StreamOutData_V0(s);
                    break;
                default:
                    //this should never happen (would require having a file-key version in memory that we don't know about)
                    msg_error("utility",
                              "error, cannot stream out MHO_FileKey object with unknown version: " << version << eom);
            }
            return s;
        }

    private:
        template< typename XStream > void StreamOutData_V0(XStream& s) const
        {
            // s << aKey.fSync;
            // s << aKey.fLabel;
            s << this->fObjectId;
            s << this->fTypeId;
            for(uint32_t i = 0; i < MHO_FileKeyNameLength; i++)
            {
                s << this->fName[i];
            }
            s << this->fSize;
        }

        template< typename XStream > void StreamInData_V0(XStream& s)
        {
            // s >> aKey.fSync;
            // s >> aKey.fLabel;
            s >> this->fObjectId;
            s >> this->fTypeId;
            for(uint32_t i = 0; i < MHO_FileKeyNameLength; i++)
            {
                s >> this->fName[i];
            }
            s >> this->fSize;
        }
};

} // namespace hops

#endif /*! end of include guard: MHO_FileKey */
