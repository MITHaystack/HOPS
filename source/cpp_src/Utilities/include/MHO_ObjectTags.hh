#ifndef MHO_ObjectTags_HH__
#define MHO_ObjectTags_HH__

/*
*File: MHO_ObjectTags.hh
*Class: MHO_ObjectTags
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: container for tag/value meta-data to be attached to an object UUID
* could also be used to cross-reference objects via their UUID
*/

#include <map>
#include <string>
#include <vector>
#include <utility>

#include "MHO_Serializable.hh"
#include "MHO_UUID.hh"

namespace hops{


class MHO_ObjectTags: virtual public MHO_Serializable
{
    public:
        MHO_ObjectTags();
        virtual ~MHO_ObjectTags();

        //set/get the unique id for the associated object
        MHO_UUID GetObjectUUID() const;
        void SetObjectUUID(const MHO_UUID& uuid);
        std::string GetObjectUUIDAsString() const;
        void SetObjectUUIDFromString(std::string uuid);

        //set/get a name for the associated object
        std::string GetObjectName() const;
        void SetObjectName(const std::string& obj_name);
        void SetObjectName(const char* obj_name);

        //get the number of present tags
        std::size_t GetNTags() const;

        //set a tag/value pair
        void SetTag(const char* tag_name, const char* tag_value);
        void SetTag(const std::string& tag_name, const std::string& tag_value);

        //check if a tag with the given name is present
        bool IsTagPresent(const std::string& tag_name) const;
        bool IsTagPresent(const char* tag_name) const;

        //retrieve the value of a given tag
        bool GetTagValue(const std::string& tag_name, std::string& tag_value);
        bool GetTagValue(const char* tag_name, std::string& tag_value);

        //collect all of the present tag names
        void DumpTags(std::vector< std::string >& tag_names) const;

        //collect all of the preset tag/value pairs
        void DumpTagValuePairs(std::vector< std::pair<std::string, std::string> >& tag_value_pairs) const;

    protected:

        MHO_UUID fObjectUUID; //uuid of the object
        std::map< std::string, std::string> fTags;
        static const std::string fNameTag;

    public:

        virtual uint64_t GetSerializedSize() const override;

        template<typename XStream> friend XStream& operator>>(XStream& s, MHO_ObjectTags& aData)
        {
            //first item to stream is always the version number
            MHO_ClassVersion vers;
            s >> vers;
            //check if incoming data is equal to the current class version
            if( vers != aData.GetVersion() )
            {
                MHO_ClassIdentity::ClassVersionErrorMsg(aData, vers);
                //Flag this as an unknown object version so we can skip over this data
                MHO_ObjectStreamState<XStream>::SetUnknown(s);
            }
            else
            {
                //now grab uuid
                s >> aData.fObjectUUID;
                //now number of items in the tag map
                uint64_t n_elem;
                s >> n_elem;
                for(std::size_t i=0; i<n_elem; i++)
                {
                    std::string key;
                    std::string val;
                    s >> key;
                    s >> val;

                    aData.fTags[key] = val;
                }
            }
            return s;
        };

        template<typename XStream> friend XStream& operator<<(XStream& s, const MHO_ObjectTags& aData)
        {
            //first item to stream is always the version number
            s << aData.GetVersion();
            //then do object uuid
            s << aData.fObjectUUID;
            //no number of tags
            uint64_t n_elem = aData.GetNTags();
            s << n_elem;

            std::vector< std::pair< std::string, std::string> > tv_pairs;
            aData.DumpTagValuePairs(tv_pairs);

            for(auto it = tv_pairs.begin(); it != tv_pairs.end(); it++)
            {
                s << it->first;
                s << it->second;
            }
            return s;
        };

};


}//end of hops namespace

#endif /* end of include guard: MHO_ObjectTags */
