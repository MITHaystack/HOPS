#include "MHO_ObjectTags.hh"


namespace hops{



const std::string MHO_ObjectTags::fNameTag = "name";

MHO_ObjectTags::MHO_ObjectTags(){};
MHO_ObjectTags::~MHO_ObjectTags(){};

uint64_t
MHO_ObjectTags::GetSerializedSize() const
{
    uint64_t total_size = 0;
    total_size += sizeof(MHO_ClassVersion); //version number
    total_size += fObjectUUID.size(); //size of the object uuid
    total_size += sizeof(uint64_t); //number of  tag/value pairs

    if(fTags.size() != 0)
    {
        for(auto iter = fTags.begin(); iter != fTags.end(); iter++)
        {
            total_size += sizeof(uint64_t); //each string streams a size parameter too!
            total_size += iter->first.size();
            total_size += sizeof(uint64_t);
            total_size += iter->second.size();
        }
    }
    return total_size;
}

MHO_UUID
MHO_ObjectTags::GetObjectUUID() const {return fObjectUUID;}

std::string
MHO_ObjectTags::GetObjectUUIDAsString() const {return fObjectUUID.as_string();};

void
MHO_ObjectTags::SetObjectUUID(const MHO_UUID& uuid){ fObjectUUID = uuid;};

//set/get a name for the associated object
std::string
MHO_ObjectTags::GetObjectName() const
{
    std::string obj_name = "";
    auto iter = fTags.find(fNameTag);
    if(iter != fTags.end())
    {
        obj_name = iter->second;
    }
    return obj_name;
}

void
MHO_ObjectTags::SetObjectName(const std::string& obj_name)
{
    fTags[fNameTag] = obj_name;
};

void
MHO_ObjectTags::SetObjectName(const char* obj_name)
{
    fTags[fNameTag] = std::string(obj_name);
};

//get the number of present tags
std::size_t
MHO_ObjectTags::GetNTags() const { return fTags.size();}

//set a tag/value pair
void
MHO_ObjectTags::SetTag(const std::string& tag_name, const std::string& tag_value)
{
    fTags[tag_name] = tag_value;
}

//set a tag/value pair
void
MHO_ObjectTags::SetTag(const char* tag_name, const char* tag_value)
{
    fTags[std::string(tag_name)] = std::string(tag_value);
}


//check if a tag with the given name is present
bool
MHO_ObjectTags::IsTagPresent(const std::string& tag_name)
{
    auto iter = fTags.find(tag_name);
    if(iter != fTags.end()){return true;}
    return false;
}

//retrieve the value of a given tag
bool
MHO_ObjectTags::GetTagValue(const std::string& tag_name, std::string& tag_value)
{
    tag_value = "";
    auto iter = fTags.find(tag_name);
    if(iter == fTags.end())
    {
        return false;
    }
    else
    {
        tag_value = iter->second;
        return true;
    }
}

//retrieve the value of a given tag
bool
MHO_ObjectTags::GetTagValue(const char* tag_name, std::string& tag_value)
{
    tag_value = "";
    auto iter = fTags.find(std::string(tag_name));
    if(iter == fTags.end())
    {
        return false;
    }
    else
    {
        tag_value = iter->second;
        return true;
    }
}


//collect all of the present tag names
void
MHO_ObjectTags::DumpTags(std::vector< std::string>& tag_names) const
{
    tag_names.clear();
    for(auto iter = fTags.begin(); iter != fTags.end(); iter++)
    {
        tag_names.push_back(iter->first);
    }
}

//collect all of the preset tag/value pairs
void
MHO_ObjectTags::DumpTagValuePairs(std::vector< std::pair<std::string, std::string> >& tag_value_pairs) const
{
    tag_value_pairs.clear();
    for(auto iter = fTags.begin(); iter != fTags.end(); iter++)
    {
        tag_value_pairs.push_back( std::make_pair(iter->first, iter->second) );
    }
}

}//end of hops namespace
