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

        MHO_BinaryFileInterface():
            fCollectKeys(false)
        {};

        virtual ~MHO_BinaryFileInterface(){};

        bool IsOpenForWrite()
        {
            if(fCollectKeys){return (fObjectStreamer.IsOpenForWrite() &&  fKeyStreamer.IsOpenForWrite() );}
            else {return fObjectStreamer.IsOpenForWrite();}
        }
        bool IsOpenForRead(){return fObjectStreamer.IsOpenForRead();}

        bool OpenToWrite(const std::string& obj_filename, const std::string& index_filename = "")
        {
            fObjectStreamer.SetFilename(obj_filename);
            fObjectStreamer.OpenToWrite();

            if(index_filename != "") //stream keys to a separate index file
            {
                fCollectKeys = true;
                fKeyStreamer.SetFilename(index_filename);
                fKeyStreamer.OpenToWrite();
                return fObjectStreamer.IsOpenForWrite() && fKeyStreamer.IsOpenForWrite();
            }

            return fObjectStreamer.IsOpenForWrite();
        }


        bool OpenToAppend(const std::string& obj_filename, const std::string& index_filename = "")
        {
            fObjectStreamer.SetFilename(obj_filename);
            fObjectStreamer.OpenToAppend();

            if(index_filename != "") //stream keys to a separate index file
            {
                fCollectKeys = true;
                fKeyStreamer.SetFilename(index_filename);
                fKeyStreamer.OpenToAppend();
                return fObjectStreamer.IsOpenForWrite() && fKeyStreamer.IsOpenForWrite();
            }

            return fObjectStreamer.IsOpenForWrite();
        }

        bool OpenToRead(const std::string& filename)
        {
            fObjectStreamer.SetFilename(filename);
            fObjectStreamer.OpenToRead();

            return fObjectStreamer.IsOpenForRead();
        }

        bool OpenToReadAtOffset(const std::string& filename, uint64_t offset_bytes)
        {
            fObjectStreamer.SetFilename(filename);
            fObjectStreamer.OpenToRead();
            fObjectStreamer.GetStream().seekg(offset_bytes, std::ios_base::cur);
            return fObjectStreamer.IsOpenForRead();
        }


        bool ExtractIndexFileObjectKeys(const std::string& index_filename, std::vector<MHO_FileKey>& keys)
        {
            keys.clear();

            if( fObjectStreamer.IsOpenForRead() || fObjectStreamer.IsOpenForWrite() ||
                fKeyStreamer.IsOpenForRead() || fKeyStreamer.IsOpenForRead() )
            {
                msg_warn("file", "Cannot extract index file keys with active stream. Close open file first." << eom);
                return false;
            }
            else
            {
                fKeyStreamer.SetFilename(index_filename);
                fKeyStreamer.OpenToRead();
                if( fKeyStreamer.IsOpenForRead() )
                {
                    while( fKeyStreamer.GetStream().good() )
                    {
                        MHO_FileKey key;
                        fKeyStreamer >> key;
                        if(fKeyStreamer.GetStream().good())
                        {
                            keys.push_back(key);
                        }
                    }
                    fKeyStreamer.Close();
                    return true;
                }
                else
                {
                    msg_error("file", "Failed to read object keys, file not open for reading." << eom);
                    fKeyStreamer.Close();
                    return false;
                }
            }
        }

        bool ExtractFileObjectKeys(const std::string& filename, std::vector<MHO_FileKey>& keys)
        {
            keys.clear();

            if( fObjectStreamer.IsOpenForRead() || fObjectStreamer.IsOpenForWrite() ||
                fKeyStreamer.IsOpenForRead() || fKeyStreamer.IsOpenForRead() )
            {
                msg_warn("file", "Cannot extract file keys with active stream. Close open file first." << eom);
                return false;
            }
            else
            {
                fKeyStreamer.SetFilename(filename);
                fKeyStreamer.OpenToRead();
                if( fKeyStreamer.IsOpenForRead() )
                {
                    while( fKeyStreamer.GetStream().good() )
                    {
                        MHO_FileKey key;
                        fKeyStreamer >> key;
                        if( fKeyStreamer.GetStream().good() ) //make sure we haven't hit EOF
                        {
                            //first check if the sync word matches, if not then we have
                            //gotten off track and are in unknown territory
                            bool key_ok = true;
                            if( key.fSync != MHO_FileKeySyncWord ){key_ok = false; }
                            if(key_ok)
                            {
                                keys.push_back(key);
                                //now skip ahead by the size of the object
                                fKeyStreamer.SkipAhead(key.fSize);
                            }
                            else
                            {
                                msg_error("file", "Failed to read object key, sync word " << key.fSync << " not recognized." << eom);
                                break;
                            }
                        }
                        else{break;} //EOF
                    }
                    fKeyStreamer.Close();
                    return true;
                }
                else
                {
                    msg_error("file", "Failed to read key, file not open for reading." << eom);
                    fKeyStreamer.Close();
                    return false; //non-recoverable error
                }
            }
            Close();
        }


        void Close()
        {
            fObjectStreamer.Close();
            if( !( fObjectStreamer.IsClosed() ) )
            {
                msg_error("file", "Failed to close file." << eom);
            }

            if(fCollectKeys)
            {
                fKeyStreamer.Close();
                if( !( fKeyStreamer.IsClosed() ) )
                {
                    msg_error("file", "Failed to close key/index file." << eom);
                }
            }
            fCollectKeys = false;
        }

        template<class XWriteType> bool Write(const XWriteType& obj, const std::string& shortname = "")
        {
            if( fObjectStreamer.IsOpenForWrite() )
            {
                MHO_FileKey key = GenerateObjectFileKey(obj, shortname);
                fObjectStreamer.ResetByteCount();
                fObjectStreamer << key;
                msg_debug("file", "wrote object key of size: " << fObjectStreamer.GetNBytesWritten() <<" bytes." << eom);
                fObjectStreamer.ResetByteCount();
                fObjectStreamer << obj;
                msg_debug("file", "wrote object of size: " << fObjectStreamer.GetNBytesWritten() <<" bytes." << eom);
                if( fObjectStreamer.GetStream().good() )
                {
                    if(fCollectKeys && fKeyStreamer.IsOpenForWrite())
                    {
                        fKeyStreamer << key;
                    }
                    return true;
                }
                else
                {
                    msg_error("file", "Failed to write object, stream state returned: " << fObjectStreamer.GetStream().rdstate() << eom);
                    return false;
                }
            }
            else
            {
                return false;
                msg_error("file", "Failed to write object, file not open for writing." << eom);
            }
        }

        //overload for char array name
        template<class XWriteType> bool Write(const XWriteType& obj, const char* shortname)
        {
            std::string sshortname(shortname);
            return this->Write(obj, sshortname);
        }

        template<class XReadType> bool Read(XReadType& obj, MHO_FileKey& obj_key)
        {
            if(fObjectStreamer.IsOpenForRead())
            {
                MHO_FileKey key;
                fObjectStreamer >> key;
                obj_key = key;

                //first check if the sync word matches, if not then we have
                //gotten off track and are in unknown territory
                bool key_ok = true;
                if( key.fSync != MHO_FileKeySyncWord ){key_ok = false;}

                fMD5Generator.Initialize();
                std::string name = MHO_ClassIdentity::ClassName(obj);
                fMD5Generator << name;
                fMD5Generator.Finalize();
                MHO_UUID type_uuid = fMD5Generator.GetDigestAsUUID();
                if(key.fTypeId != type_uuid){key_ok = false;}

                if(key_ok)
                {
                    obj.SetObjectUUID(obj_key.fObjectId);
                    fObjectStreamer >> obj;
                    if( fObjectStreamer.IsObjectUnknown() )
                    {
                        //object version was not recognized, skip this object's data
                        if(key.fSize > sizeof(MHO_ClassVersion) )
                        {
                            uint64_t skip_size = key.fSize - sizeof(MHO_ClassVersion);
                            fObjectStreamer.GetStream().seekg(skip_size, std::ios_base::cur);
                            msg_warn("file", "Encountered and skipped an unrecognized version of object with class name: " << name << "." << eom);
                            fObjectStreamer.ResetObjectState(); //reset streamer state for next object
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
                    msg_error("file", "Object uuid: " << type_uuid.as_string() << " file key uuid: "<< key.fTypeId.as_string() << "." << eom);
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
        MHO_BinaryFileStreamer fObjectStreamer;
        MHO_BinaryFileStreamer fKeyStreamer;
        MHO_UUIDGenerator fUUIDGenerator;
        MHO_MD5HashGenerator fMD5Generator;

        //generate the file object key
        template<class XWriteType>
        MHO_FileKey GenerateObjectFileKey(const XWriteType& obj, const std::string& shortname)
        {
            MHO_FileKey key; //sync and label set in constructor

            //add short name
            CopyTruncatedString(shortname, key.fName);

            fMD5Generator.Initialize();
            std::string name = MHO_ClassIdentity::ClassName(obj);
            fMD5Generator << name;
            fMD5Generator.Finalize();
            key.fTypeId = fMD5Generator.GetDigestAsUUID(); //type uuid

            key.fObjectId = obj.GetObjectUUID();
            msg_debug("file", "constructing a file key with object uuid of: "<< key.fObjectId.as_string() << eom);
            //key.fObjectId = fUUIDGenerator.GenerateUUID(); //random uuid of object id
            key.fSize = obj.GetSerializedSize();

            return key;
        }

        void CopyTruncatedString(const std::string& s, char* arr)
        {
            size_t len = s.size();
            if(MHO_FileKeyNameLength <= len){len = MHO_FileKeyNameLength;}
            for(size_t i=0; i<len; i++)
            {
                if(i < s.size()){arr[i] = s[i];}
                else{ arr[i] = '\0'; }
            }
        }


        //file object keys, collected as we go
        bool fCollectKeys;

};


}


#endif /* end of include guard: MHO_BinaryFileInterface */
