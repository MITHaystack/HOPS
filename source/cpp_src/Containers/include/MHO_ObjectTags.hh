#ifndef MHO_ObjectTags_HH__
#define MHO_ObjectTags_HH__

#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "MHO_ExtensibleElement.hh"
#include "MHO_Taggable.hh"
#include "MHO_UUID.hh"

namespace hops
{

/*!
 *@file MHO_ObjectTags.hh
 *@class MHO_ObjectTags
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu May 13 10:44:24 2021 -0400
 *@brief A container object which is intended to associate key/value meta-data with other objects via a list of their UUIDs
 */

/**
 * @brief Class MHO_ObjectTags
 */
class MHO_ObjectTags: public MHO_Taggable, public MHO_ExtensibleElement
{
    public:
        MHO_ObjectTags() {}

        virtual ~MHO_ObjectTags(){};

        /**
         * @brief Checks if a UUID is present in the object collection.
         *
         * @param uuid The UUID to search for.
         * @return True if the UUID is found, false otherwise.
         */
        bool IsObjectUUIDPresent(const MHO_UUID& uuid) const
        {
            auto it = fObjectUUIDSet.find(uuid);
            if(it != fObjectUUIDSet.end())
            {
                return true;
            }
            return false;
        }

        /**
         * @brief Inserts a UUID into the object UUID set for association with this tag collection.
         *
         * @param uuid UUID of the object to be associated with the tag collection
         */
        void AddObjectUUID(const MHO_UUID& uuid) { fObjectUUIDSet.insert(uuid); }

        /**
         * @brief Removes an object UUID from the set if it exists.
         *
         * @param uuid The UUID of the object to remove.
         */
        void RemoveObjectUUID(const MHO_UUID& uuid)
        {
            auto it = fObjectUUIDSet.find(uuid);
            if(it != fObjectUUIDSet.end())
            {
                fObjectUUIDSet.erase(it);
            }
        };

        /**
         * @brief Getter for the number of object uuids
         *
         * @return Number of object UUIDs as std::size_t
         */
        std::size_t GetNObjectUUIDs() const { return fObjectUUIDSet.size(); }

        /**
         * @brief Getter for all object uuids at once
         *
         * @return Vector of MHO_UUID representing all object UUIDs.
         */
        std::vector< MHO_UUID > GetAllObjectUUIDs() const
        {
            std::vector< MHO_UUID > obj_uuids;
            for(auto it = fObjectUUIDSet.begin(); it != fObjectUUIDSet.end(); it++)
            {
                obj_uuids.push_back(*it);
            }
            return obj_uuids;
        }

        /**
         * @brief Checks if a tag with the given name (key) is present in the container.
         *
         * @param tag_name The name of the tag to search for.
         * @return True if the tag is present, false otherwise.
         */
        bool IsTagPresent(const std::string& tag_name) const { return this->HasKey(tag_name); }

        /**
         * @brief Checks if a tag (key) is present in the container.
         *
         * @param tag_name The name of the tag to search for.
         * @return True if the tag is present, false otherwise.
         */
        bool IsTagPresent(const char* tag_name) const
        {
            std::string tmp(tag_name);
            return IsTagPresent(tmp);
        }

        /**
         * @brief Setter for tag/value (key:value pair)
         *
         * @param tag_name Name of the tag as C-style string
         * @param tag_value Value to set for the tag
         * @return No return value (void)
         */
        template< typename XValueType > void SetTagValue(const char* tag_name, const XValueType& tag_value)
        {
            std::string tmp(tag_name);
            SetTagValue(tmp, tag_value);
        }

        /**
         * @brief Setter for tag/value (key:value pair)
         *
         * @param tag_name Tag name as std::string string
         * @param tag_value Value to set for the given tag
         * @return No return value (void)
         */
        template< typename XValueType > void SetTagValue(const std::string& tag_name, const XValueType& tag_value)
        {
            this->Insert(tag_name, tag_value);
        }

        /**
         * @brief Getter for a tag's value
         *
         * @param tag_name Name of the tag to retrieve.
         * @param tag_value (XValueType&)
         * @return True if successful, false otherwise.
         */
        template< typename XValueType > bool GetTagValue(const char* tag_name, XValueType& tag_value)
        {
            std::string tmp(tag_name);
            return GetTagValue(tmp, tag_value);
        }

        /**
         * @brief retrieve the value of a given tag
         *
         * @param tag_name The name of the tag to retrieve the value for
         * @param tag_value Reference to store the retrieved value
         * @return True if the tag is found and its value is successfully stored, false otherwise
         */
        template< typename XValueType > bool GetTagValue(const std::string& tag_name, XValueType& tag_value)
        {
            return this->Retrieve(tag_name, tag_value);
        }

        /**
         * @brief Getter for a tag's value, with forced conversion to a string
         *
         * @param tag_name (const std::string&)
         * @return Return value (std::string)
         */
        std::string GetTagValueAsString(const std::string& tag_name) const
        {
            std::stringstream ss;
            //TODO FIXME, what if key is not unique among types?
            if(this->ContainsKey(tag_name))
            {
                {
                    char value;
                    bool ok = this->Retrieve(tag_name, value);
                    if(ok)
                    {
                        ss << value;
                        return ss.str();
                    }
                }

                {
                    bool value;
                    bool ok = this->Retrieve(tag_name, value);
                    if(ok)
                    {
                        ss << value;
                        return ss.str();
                    }
                }

                {
                    int value;
                    bool ok = this->Retrieve(tag_name, value);
                    if(ok)
                    {
                        ss << value;
                        return ss.str();
                    }
                }

                {
                    double value;
                    bool ok = this->Retrieve(tag_name, value);
                    if(ok)
                    {
                        ss << value;
                        return ss.str();
                    }
                }

                {
                    std::string value;
                    bool ok = this->Retrieve(tag_name, value);
                    if(ok)
                    {
                        return value;
                    }
                }

                {
                    mho_json value;
                    bool ok = this->Retrieve(tag_name, value);
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

        /**
         * @brief collect all of the present tag names, and fill the passed reference
         *
         * @param tag_names Reference to std::vector<std::string> that will be cleared and populated with current tag names
         */
        void DumpTags(std::vector< std::string >& tag_names) const
        {
            tag_names.clear();
            tag_names = this->DumpKeys();
        }

        /**
         * @brief Getter for tagged object uuid set
         *
         * @return The current tagged object UUID set.
         */
        std::set< MHO_UUID > GetTaggedObjectUUIDSet() const { return fObjectUUIDSet; }

    protected:
        //all object UUIDs which are associated with the tags
        std::set< MHO_UUID > fObjectUUIDSet;

    public:
        /**
         * @brief Getter for serialized size
         *
         * @return Total serialized size as uint64_t
         * @note This is a virtual function.
         */
        virtual uint64_t GetSerializedSize() const override
        {
            uint64_t total_size = 0;
            total_size += sizeof(MHO_ClassVersion); //version number
            total_size += sizeof(uint64_t);         //number of uuids
            total_size += MHO_UUID::ByteSize() * (fObjectUUIDSet.size());
            total_size += MHO_Taggable::GetSerializedSize();
            return total_size;
        }

        template< typename XStream > friend XStream& operator>>(XStream& s, MHO_ObjectTags& aData)
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
                    MHO_ObjectStreamState< XStream >::SetUnknown(s);
            }
            return s;
        }

        template< typename XStream > friend XStream& operator<<(XStream& s, const MHO_ObjectTags& aData)
        {
            switch(aData.GetVersion())
            {
                case 0:
                    s << aData.GetVersion();
                    aData.StreamOutData_V0(s);
                    break;
                default:
                    msg_error("containers", "error, cannot stream out MHO_Taggable object with unknown version: "
                                                << aData.GetVersion() << eom);
            }
            return s;
        }

    private:
        /**
         * @brief Reads and processes object UUIDs from a stream and adds them to the current tag collection.
         *
         * @param s Input stream of object data.
         * @return No return value (void)
         */
        template< typename XStream > void StreamInData_V0(XStream& s)
        {
            //then do the number of object uuids
            uint64_t n_uuids = 0;
            s >> n_uuids;
            //then do object uuids
            for(uint64_t i = 0; i < n_uuids; i++)
            {
                MHO_UUID tmp_uuid;
                s >> tmp_uuid;
                this->AddObjectUUID(tmp_uuid);
            }
            //now do the taggable element;
            s >> static_cast< MHO_Taggable& >(*this);
        };

        /**
         * @brief Serializes object UUIDs and taggable element to an output stream.
         *
         * @param s Output stream of type XStream&.
         * @return void
         */
        template< typename XStream > void StreamOutData_V0(XStream& s) const
        {
            //then do the number of object uuids
            uint64_t n_uuids = this->fObjectUUIDSet.size();
            s << n_uuids;
            //then do object uuids
            for(auto it = this->fObjectUUIDSet.begin(); it != this->fObjectUUIDSet.end(); it++)
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

} // namespace hops

#endif /*! end of include guard: MHO_ObjectTags */
