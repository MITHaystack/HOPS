#ifndef MHO_ObjectTags_HH__
#define MHO_ObjectTags_HH__

/*!
*@file MHO_ObjectTags.hh
*@class MHO_ObjectTags
*@author J. Barrett - barrettj@mit.edu 
*
*@date
*@brief container for tag/value meta-data to be attached to objects
*via association with their UUID
*/

#include <set>
#include <string>
#include <vector>
#include <sstream>

#include "MHO_UUID.hh"
#include "MHO_Taggable.hh"
#include "MHO_ExtensibleElement.hh"

namespace hops{


class MHO_ObjectTags: public MHO_Taggable, public MHO_ExtensibleElement
{
    public:

        MHO_ObjectTags(){}
        virtual ~MHO_ObjectTags(){};

        //check if a uuid is in the collection
        bool IsObjectUUIDPresent(const MHO_UUID& uuid) const
        {
            auto it = fObjectUUIDSet.find(uuid);
            if( it != fObjectUUIDSet.end() ){return true;}
            return false;
        }

        //insert a uuid for an object to be associated with our tag collection
        void AddObjectUUID(const MHO_UUID& uuid)
        {
            fObjectUUIDSet.insert(uuid);
        }

        void RemoveObjectUUID(const MHO_UUID& uuid)
        {
            auto it = fObjectUUIDSet.find(uuid);
            if( it != fObjectUUIDSet.end() ){fObjectUUIDSet.erase(it);}
        };

        std::size_t GetNObjectUUIDs() const {return fObjectUUIDSet.size();}

        //grab all object uuids at once
        std::vector< MHO_UUID > GetAllObjectUUIDs() const
        {
            std::vector<MHO_UUID> obj_uuids;
            for(auto it = fObjectUUIDSet.begin(); it != fObjectUUIDSet.end(); it++)
            {
                obj_uuids.push_back(*it);
            }
            return obj_uuids;
        }

        //check if a tag with the given name is present
        bool IsTagPresent(const std::string& tag_name) const
        {
            return this->HasKey(tag_name);
        }


        bool IsTagPresent(const char* tag_name) const
        {
            std::string tmp(tag_name);
            return IsTagPresent(tmp);
        }

        //set a tag/value pair
        template<typename XValueType>
        void SetTagValue(const char* tag_name, const XValueType& tag_value)
        {
            std::string tmp(tag_name);
            SetTagValue(tmp, tag_value);
        }

        template<typename XValueType>
        void SetTagValue(const std::string& tag_name, const XValueType& tag_value)
        {
            this->Insert(tag_name, tag_value);
        }

        template<typename XValueType>
        bool GetTagValue(const char* tag_name, XValueType& tag_value)
        {
            std::string tmp;
            return GetTagValue(tmp, tag_value);
        }

        //retrieve the value of a given tag
        template<typename XValueType>
        bool GetTagValue(const std::string& tag_name, XValueType& tag_value)
        {
            return this->Retrieve(tag_name, tag_value);
        }

        //get the number of tags present
        std::string GetTagValueAsString(const std::string& tag_name) const
        {
            std::stringstream ss;
            //TODO FIXME, what if key is not unique among types?
            if(this->ContainsKey(tag_name))
            {
                {
                    char value;
                    bool ok = this->Retrieve(tag_name,value);
                    if(ok)
                    {
                        ss << value;
                        return ss.str();
                    }
                }

                {
                    bool value;
                    bool ok = this->Retrieve(tag_name,value);
                    if(ok)
                    {
                        ss << value;
                        return ss.str();
                    }
                }


                {
                    int value;
                    bool ok = this->Retrieve(tag_name,value);
                    if(ok)
                    {
                        ss << value;
                        return ss.str();
                    }
                }


                {
                    double value;
                    bool ok = this->Retrieve(tag_name,value);
                    if(ok)
                    {
                        ss << value;
                        return ss.str();
                    }
                }

                {
                    std::string value;
                    bool ok = this->Retrieve(tag_name,value);
                    if(ok)
                    {
                        return value;
                    }
                }

                {
                    mho_json value;
                    bool ok = this->Retrieve(tag_name,value);
                    if(ok)
                    {
                        std::stringstream jss;
                        jss << value;
                        return jss.str();
                    }
                }
            }

            //return nothing
            return std::string("");

        }


        //collect all of the present tag names
        void DumpTags(std::vector< std::string >& tag_names) const
        {
            tag_names.clear();
            tag_names = this->DumpKeys();
        }

        std::set< MHO_UUID > GetTaggedObjectUUIDSet() const {return fObjectUUIDSet;}

    protected:

        //all object UUIDs which are associated with the tags
        std::set< MHO_UUID > fObjectUUIDSet;

    public:

        virtual uint64_t GetSerializedSize() const override
        {
            uint64_t total_size = 0;
            total_size += sizeof(MHO_ClassVersion); //version number
            total_size += sizeof(uint64_t); //number of uuids
            total_size += MHO_UUID::ByteSize()*(fObjectUUIDSet.size());
            total_size += MHO_Taggable::GetSerializedSize();
            return total_size;
        }


        template<typename XStream> friend XStream& operator>>(XStream& s, MHO_ObjectTags& aData)
        {
            MHO_ClassVersion vers;
            s >> vers;
            switch(vers)
            {
                case 0:
                    aData.StreamInData_V0(s);
                break;
                default:
                    MHO_ClassIdentity::ClassVersionErrorMsg(aData, vers);
                    //Flag this as an unknown object version so we can skip over this data
                    MHO_ObjectStreamState<XStream>::SetUnknown(s);
            }
            return s;
        }

        template<typename XStream> friend XStream& operator<<(XStream& s, const MHO_ObjectTags& aData)
        {
            switch( aData.GetVersion() )
            {
                case 0:
                    s << aData.GetVersion();
                    aData.StreamOutData_V0(s);
                break;
                default:
                    msg_error("containers",
                        "error, cannot stream out MHO_Taggable object with unknown version: "
                        << aData.GetVersion() << eom );
            }
            return s;
        }

    private:

        template<typename XStream> void StreamInData_V0(XStream& s)
        {
            //then do the number of object uuids
            uint64_t n_uuids = 0;
            s >> n_uuids;
            //then do object uuids
            for(uint64_t i=0; i<n_uuids; i++)
            {
                MHO_UUID tmp_uuid;
                s >> tmp_uuid;
                this->AddObjectUUID(tmp_uuid);
            }
            //now do the taggable element;
            s >> static_cast< MHO_Taggable& >(*this);
        };

        template<typename XStream> void StreamOutData_V0(XStream& s) const
        {
            //then do the number of object uuids
            uint64_t n_uuids = this->fObjectUUIDSet.size();
            s << n_uuids;
            //then do object uuids
            for(auto it= this->fObjectUUIDSet.begin(); it != this->fObjectUUIDSet.end(); it++)
            {
                s << *it;
            }
            //now do the taggable element;
            s << static_cast< const MHO_Taggable& >(*this);
        };

        virtual MHO_UUID DetermineTypeUUID() const override
        {
            MHO_MD5HashGenerator gen;
            gen.Initialize();
            std::string name = MHO_ClassIdentity::ClassName(*this);
            gen << name;
            gen.Finalize();
            return gen.GetDigestAsUUID();
        }

};


}//end of hops namespace

#endif /*! end of include guard: MHO_ObjectTags */
