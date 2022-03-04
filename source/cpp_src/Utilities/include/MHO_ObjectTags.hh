#ifndef MHO_ObjectTags_HH__
#define MHO_ObjectTags_HH__

/*
*File: MHO_ObjectTags.hh
*Class: MHO_ObjectTags
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: container for tag/value meta-data to be attached to objects 
via association with ther  UUID
* could also be used to cross-reference objects via their UUID
*/

#include <set>
#include <map>
#include <string>
#include <vector>
#include <utility>

#include "MHO_Serializable.hh"

#include "MHO_UUID.hh"
#include "MHO_Taggable.hh"

namespace hops{


class MHO_ObjectTags: public MHO_Taggable
{
    public:

        MHO_ObjectTags(){}
        virtual ~MHO_ObjectTags(){};

        //check if a uuid is in the collection
        bool IsObjectUUIDPresent(const MHO_UUID& uuid) const
        {
            auto it = fObjectUUIDSet.find(uuid);
            if( it != fObjectUUIDs.end() ){return true;}
            return false;
        }

        //insert a uuid for an object to be associated with our tag collection
        void AddObjectUUID(const MHO_UUID& uuid)
        {
            if( !IsObjectUUIDPresent(uuid) ){fObjectUUIDSet.insert(uuid);}
        }

        void RemoveObjectUUID(const MHO_UUID& uuid)
        {
            auto it = fObjectUUIDSet.find(uuid);
            if( it != fObjectUUIDs.end() ){fObjectUUIDSet.erase(it);}
        };

        std::size_t GetNObjectUUIDs() const {return fObjectUUIDSet.size();}

        //grab all objec uuids at once
        std::vector< MHO_UUID > GetAllObjectUUIDs const 
        {
            std::vector<MHO_UUID> obj_uuids;
            for(auto it = fObjectUUIDSet.begin(); it != fObjectUUIDSet.end(); it++)
            {
                obj_uuids.push_back(*it);
            }
            return obj_uuids;
        }

        //get the number of tags present
        std::size_t GetNTags() const;

        //check if a tag with the given name is present
        bool IsTagPresent(const std::string& tag_name) const 


        bool IsTagPresent(const char* tag_name) const
        {
            std::string tmp(tag_name);}
        }

        //set a tag/value pair
        template<typename XValueType>
        void SetTagValue(const char* tag_name, const XValueType& tag_value)
        {

        }

        template<typename XValueType>
        void SetTagValue(const std::string& tag_name, const XValueType& tag_value)
        {

        }

        //retrieve the value of a given tag
        template<typename XValueType>
        bool GetTagValue(const std::string& tag_name, XValueType& tag_value)
        {

        }

        template<typename XValueType>
        bool GetTagValue(const char* tag_name, XValueType& tag_value);

        //collect all of the present tag names
        void DumpTags(std::vector< std::string >& tag_names) const;

        //collect all of the preset tag/value pairs
        void DumpTagValuePairs(std::vector< std::pair<std::string, std::string> >& tag_value_pairs) const;

    protected:

        void ConditionallyAdd(const MHO_UUID& uuid)
        {

        }

        //all object UUIDs which are associated with the tags 
        std::set< MHO_UUID > fObjectUUIDSet;

    public:

        virtual uint64_t GetSerializedSize() const override;

        // template<typename XStream> friend XStream& operator>>(XStream& s, MHO_ObjectTags& aData)
        // {
        //     //first item to stream is always the version number
        //     MHO_ClassVersion vers;
        //     s >> vers;
        //     //check if incoming data is equal to the current class version
        //     if( vers != aData.GetVersion() )
        //     {
        //         MHO_ClassIdentity::ClassVersionErrorMsg(aData, vers);
        //         //Flag this as an unknown object version so we can skip over this data
        //         MHO_ObjectStreamState<XStream>::SetUnknown(s);
        //     }
        //     else
        //     {
        //         //now grab uuid
        //         s >> aData.fObjectUUID;
        //         //now number of items in the tag map
        //         uint64_t n_elem;
        //         s >> n_elem;
        //         for(std::size_t i=0; i<n_elem; i++)
        //         {
        //             std::string key;
        //             std::string val;
        //             s >> key;
        //             s >> val;
        // 
        //             aData.fTags[key] = val;
        //         }
        //     }
        //     return s;
        // };
        // 
        // template<typename XStream> friend XStream& operator<<(XStream& s, const MHO_ObjectTags& aData)
        // {
        //     //first item to stream is always the version number
        //     s << aData.GetVersion();
        //     //then do object uuid
        //     s << aData.fObjectUUID;
        //     //no number of tags
        //     uint64_t n_elem = aData.GetNTags();
        //     s << n_elem;
        // 
        //     std::vector< std::pair< std::string, std::string> > tv_pairs;
        //     aData.DumpTagValuePairs(tv_pairs);
        // 
        //     for(auto it = tv_pairs.begin(); it != tv_pairs.end(); it++)
        //     {
        //         s << it->first;
        //         s << it->second;
        //     }
        //     return s;
        // };

};


}//end of hops namespace

#endif /* end of include guard: MHO_ObjectTags */
