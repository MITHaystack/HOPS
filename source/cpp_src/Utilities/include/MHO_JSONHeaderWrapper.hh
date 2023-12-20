#ifndef JSON_WRAPPER_HH__
#define JSON_WRAPPER_HH__

#include "nlohmann/json.hpp"
using mho_json = nlohmann::json;
using mho_ordered_json = nlohmann::ordered_json;
namespace nl=nlohmann;

#include "MHO_Message.hh"

namespace hops
{


//constructor is protected
//this class is only intended to provide an interface that derived classes may inherit
class MHO_JSONWrapper
{
    protected:

        MHO_JSONWrapper():fObject(nullptr){};
        MHO_JSONWrapper(const MHO_JSONWrapper& copy)
        {
            fObject = copy.fObject;
        };
        
        void SetObject(mho_json* obj){fObject = obj;}
        
    private:
        
        mho_json* fObject;

    public:
        
        virtual ~MHO_JSONWrapper(){};

        bool HasKey(const std::string& key) const
        {
            if(fObject == nullptr){return false;}
            auto it = fObject->find(key);
            if(it != fObject->end()){return true;}
            return false;
        }

        bool HasKey(const char* char_key) const
        {
            std::string key(char_key);
            return HasKey(key);
        }

        MHO_JSONWrapper& operator=(const MHO_JSONWrapper& rhs)
        {
            if(this != &rhs){fObject = rhs.fObject;}
            return *this;
        }

        //start of multi-type map interface 
        std::size_t MapSize() const 
        {
            if(fObject != nullptr){return fObject->size();}
            return 0;
        }
        
        void Clear()
        {
            if(fObject != nullptr){fObject->clear();};
        }

        template< typename XValueType> 
        void Insert(const std::string& key, const XValueType& value)
        {
            //allow replacement of values
            if(fObject != nullptr){(*fObject)[key] = value;}
        }

        template< typename XValueType> 
        bool Retrieve(const std::string& key, XValueType& value) const
        {
            if(fObject == nullptr){return false;}
            auto iter = fObject->find(key);
            if(iter == fObject->end()){return false;}
            else
            {
                mho_json test;
                test["test"] = value;
                //TODO FIXME - this is a major KLUDGE 
                //but needed to avoid exceptions when key is present, but value type is different
                if(test["test"].type() == (*fObject)[key].type()) 
                {
                    value = (*fObject)[key].get<XValueType>();
                    return true;
                }
                else 
                {
                    return false;
                }
            }
        }

        std::vector<std::string> DumpKeys() const
        {
            std::vector< std::string > keys;
            if(fObject != nullptr)
            {
                for(auto iter = fObject->begin(); iter != fObject->end(); iter++)
                {
                    keys.push_back(iter.key());
                }
            }
            return keys;
        }

        //TODO eliminate me
        void DumpMap() const
        {
            if(fObject != nullptr)
            {
                for(auto iter = fObject->begin(); iter != fObject->end(); iter++)
                {
                    std::cout<<iter.key()<<" : "<<iter.value()<<std::endl;
                }
            }
        }

        bool ContainsKey(const std::string& key) const
        {
            if(fObject == nullptr){return false;}
            auto iter = fObject->find(key);
            if(iter == fObject->end()){return false;}
            else{return true;}
        }
};



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//constructor is protected
//this class is only intended to provide an interface that derived classes may inherit
//this interface is to enforce a specific access pattern associated with modifying 
//meta data attached to a vector/axis like object that is in the form of a mho_json::array_t
class MHO_IndexLabelInterface
{

    protected:
        
        MHO_IndexLabelInterface():fIndexLabelObjectPtr(nullptr)
        {
            fCurrentSize = 0;
            fDummy["index"] = -1; //dummy object for invalid returns, always has an invalid index
        };
        
        MHO_IndexLabelInterface(const MHO_IndexLabelInterface& copy)
        {
            fIndexLabelObjectPtr = copy.fIndexLabelObjectPtr;
            if(fIndexLabelObjectPtr != nullptr){fCurrentSize = fIndexLabelObjectPtr->size();}
        };
    
        void SetIndexLabelObject(mho_json::array_t* obj){fIndexLabelObjectPtr = obj;}
        
    public:
        
        virtual ~MHO_IndexLabelInterface(){};
        
        std::size_t GetIndexLabelSize() const {return fCurrentSize;}

        virtual void ResizeIndexLabels(std::size_t size)
        {
            if(fIndexLabelObjectPtr != nullptr)
            {
                fCurrentSize = fIndexLabelObjectPtr->size();
                if(fCurrentSize != size)
                {
                    fIndexLabelObjectPtr->reserve(size);
                    for(std::size_t i=0;i<size;i++) //fill with empty entries
                    {
                        mho_json empty;
                        empty["index"] = i;
                        (*fIndexLabelObjectPtr)[i] = empty;
                    }
                    fCurrentSize = size;
                }
            }
        }

        template< typename XValueType >
        void InsertIndexLabelKeyValue(std::size_t index, const std::string& key, const XValueType& value)
        {
            if(index < fCurrentSize)
            {
                (*fIndexLabelObjectPtr)[index][key] = value;
            }
            else
            {
                msg_warn("containers", "cannot insert key value pair for index: "<< index << " for array of size: "<< fCurrentSize << eom);
            }
        }
        
        template< typename XValueType >
        bool RetrieveIndexLabelKeyValue(std::size_t index, const std::string& key, const XValueType& value) const
        {
            if(index < fCurrentSize)
            {
                mho_json test;
                test["test"] = value;
                //TODO FIXME - this is a major KLUDGE 
                //but needed to avoid exceptions when key is present, but value type is different
                if(test["test"].type() == (*fIndexLabelObjectPtr)[index][key].type()) 
                {
                    XValueType label_value = (*fIndexLabelObjectPtr)[index][key].get<XValueType>();
                    value = label_value;
                    return true;
                }
            }
            else
            {
                msg_warn("containers", "cannot insert key value pair for index: "<< index << " for array of size: "<< fCurrentSize << eom);
            }
            return false;
        }

        //get a reference to the dictionary object associated with this index
        mho_json& GetLabelObject(std::size_t index)
        {
            if(index < fCurrentSize)
            {
                return (*fIndexLabelObjectPtr)[index];
            }
            else
            {
                msg_error("containers", "cannot access label object for index: "<< index << ", array size is only: "<< fCurrentSize << eom);
                return fDummy;
            }
        }
        
        //get a vector of indexes which contain a key with the same name
        std::vector< std::size_t > GetMatchingIndexes(std::string& key)
        {
            std::vector<std::size_t> idx;
            for(std::size_t i=0; i<fCurrentSize; i++)
            {
                if((*fIndexLabelObjectPtr)[i].contains(key))
                {
                    idx.push_back(i);
                }
            }
            return idx;
        }

        //get a vector of indexes which contain a key with a value which matches the passed value
        template< typename XValueType >
        std::vector< std::size_t > GetMatchingIndexes(std::string& key, const XValueType& value)
        {
            std::vector<std::size_t> idx;
            for(std::size_t i=0; i<fCurrentSize; i++)
            {
                if((*fIndexLabelObjectPtr)[i].contains(key))
                {
                    mho_json test;
                    test["test"] = value;
                    //TODO FIXME - this is a major KLUDGE 
                    //but needed to avoid exceptions when key is present, but value type is different
                    if(test["test"].type() == (*fIndexLabelObjectPtr)[i][key].type()) 
                    {
                        XValueType label_value = (*fIndexLabelObjectPtr)[i][key].get<XValueType>();
                        if(label_value == value){idx.push_back(i);};
                    }
                }
            }
            return idx;
        }
        
    private:
        
        
        std::size_t fCurrentSize;
        mho_json::array_t* fIndexLabelObjectPtr; //array of mho_json objects holding key:value pairs
        mho_json fDummy;
};









}//end namespace

#endif /* end of include guard: JSON_WRAPPER */
