#ifndef MHO_BinaryFileInterface_HH__
#define MHO_BinaryFileInterface_HH__

/*
*File: MHO_BinaryFileInterface.hh
*Class: MHO_BinaryFileInterface
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include "MHO_Message.hh"
#include "MHO_ClassIdentity.hh"
#include "MHO_Serializable.hh"
#include "MHO_FileKey.hh"
#include "MHO_BinaryFileStreamer.hh"
#include "MHO_MD5HashGenerator.hh"
#include "MHO_UUIDGenerator.hh"

namespace hops
{

class MHO_BinaryFileInterface
{
    public:

        MHO_BinaryFileInterface(){};
        virtual ~MHO_BinaryFileInterface(){};

        bool OpenToWrite(const std::string filename)
        {
            fStreamer.SetFilename(filename);
            fStreamer.OpenToWrite();
            return fStreamer.IsOpenForWrite();
        }

        bool OpenToRead(const std::string filename)
        {
            fStreamer.SetFilename(filename);
            fStreamer.OpenToRead();
            return fStreamer.IsOpenForRead();
        }

        void Close()
        {
            fStreamer.Close();
            if(!(fStreamer.IsClosed()))
            {
                msg_error("file", "Failed to close file." << eom);
            }
        }


        template<class XWriteType> bool Write(const XWriteType& obj, const uint32_t label = 0)
        {
            if(fStreamer.IsOpenForWrite())
            {
                MHO_FileKey key = GenerateObjectFileKey(obj, label);
                fStreamer << key;
                fStreamer << obj;
                if( fStreamer.GetStream().rdstate() & std::ifstream::goodbit ){return true;}
                else{return false;}
            }
            else
            {
                return false;
                msg_error("file", "Failed to write object, file not open for writing." << eom);
            }
        }


        template<class XWriteType> bool Read(XWriteType& obj, uint32_t& label)
        {
            if(fStreamer.IsOpenForRead())
            {
                MHO_FileKey key;
                fStreamer >> key;

                bool key_ok = true;
                if( key.fSync != MHO_FileKeySyncWord ){key_ok = false;}

                //TODO determine what we want to do with the labels
                label = key.fLabel;

                fMD5Generator.Initialize();
                std::string name = MHO_ClassIdentity::ClassName(obj);
                fMD5Generator << name;
                fMD5Generator.Finalize();
                MHO_UUID type_uuid = fMD5Generator.GetDigestAsUUID();
                if(key.fTypeId != type_uuid){key_ok = false;}

                if(key_ok)
                {
                    fStreamer >> obj;
                    if( fStreamer.IsObjectUnknown() )
                    {
                        //object version was not recognized, skip this object's data
                        if(key.fSize > sizeof(MHO_ClassVersion) )
                        {
                            uint64_t skip_size = key.fSize - sizeof(MHO_ClassVersion);
                            fStreamer.GetStream().seekg(skip_size, std::ios_base::cur);
                            msg_warn("file", "Encountered and skipped an unrecognized version of object with class name: " << name << "." << eom);
                            fStreamer.ResetObjectState(); //reset streamer state for next object
                            return true; //recoverable error
                        }
                        else
                        {
                            msg_error("file", "Encountered object with wrong/corrupt object size");
                            return false; //non-recoverable error
                        }

                    }
                    return true;
                }
                else
                {
                    msg_error("file", "Failed to read object, object type/key mismatch for object of type: " << name << "." << eom);
                    return false; //non-recoverable error
                }
            }
            else
            {
                msg_error("file", "Failed to read object, file not open for reading." << eom);
                return false; //non-recoverable error
            }
        }

    private:

        //the file streamer
        MHO_BinaryFileStreamer fStreamer;
        MHO_UUIDGenerator fUUIDGenerator;
        MHO_MD5HashGenerator fMD5Generator;

        //generate the file object key
        template<class XWriteType>
        MHO_FileKey GenerateObjectFileKey(const XWriteType& obj, const uint32_t label)
        {
            MHO_FileKey key;
            //set sync word and user label
            key.fSync = MHO_FileKeySyncWord;
            key.fLabel = label;

            fMD5Generator.Initialize();
            std::string name = MHO_ClassIdentity::ClassName(obj);
            fMD5Generator << name;
            fMD5Generator.Finalize();
            key.fTypeId = fMD5Generator.GetDigestAsUUID(); //type uuid
            key.fObjectId = fUUIDGenerator.GenerateUUID(); //random uuid of object id
            key.fSize = obj.GetSerializedSize();

            return key;
        }

        //file object keys, collected as we go
        std::vector< MHO_FileKey > fKeys;

};


}


#endif /* end of include guard: MHO_BinaryFileInterface */
