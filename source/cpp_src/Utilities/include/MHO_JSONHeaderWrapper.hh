#ifndef JSON_WRAPPER_HH__
#define JSON_WRAPPER_HH__

#include "nlohmann/json.hpp"
using mho_json = nlohmann::json;
using mho_ordered_json = nlohmann::ordered_json;
namespace nl=nlohmann;

#include <sstream>
#include <string>
#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{


//constructor is protected
//this class is only intended to provide an interface that derived classes may inherit
class MHO_JSONWrapper
{
    protected:

        MHO_JSONWrapper(){};
        MHO_JSONWrapper(const MHO_JSONWrapper& copy)
        {
            fObject = copy.fObject;
        };

        void SetObject(mho_json obj){fObject = obj;}

    protected:

        mho_json fObject;

    public:

        virtual ~MHO_JSONWrapper(){};

        bool HasKey(const std::string& key) const
        {
            auto it = fObject.find(key);
            if(it != fObject.end()){return true;}
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

        void Clear()
        {
            fObject.clear();
        }

        template< typename XValueType>
        void Insert(const std::string& key, const XValueType& value)
        {
            //allow replacement of values
            fObject[key] = value;
        }

        template< typename XValueType>
        bool Retrieve(const std::string& key, XValueType& value) const
        {
            auto iter = fObject.find(key);
            if(iter == fObject.end()){return false;}
            mho_json test;
            test["test"] = value;
            if( iter.value().type() ==  test.begin().value().type() )
            {
                value = iter.value().get<XValueType>();
                return true;
            }
            else{return false;}
        }

        std::vector<std::string> DumpKeys() const
        {
            std::vector< std::string > keys;
            for(auto iter = fObject.begin(); iter != fObject.end(); iter++)
            {
                keys.push_back(iter.key());
            }
            return keys;
        }

        //TODO eliminate me
        void DumpMap() const
        {
            for(auto iter = fObject.begin(); iter != fObject.end(); iter++)
            {
                std::cout<<iter.key()<<" : "<<iter.value()<<std::endl;
            }
        }

        bool ContainsKey(const std::string& key) const
        {
            auto iter = fObject.find(key);
            if(iter == fObject.end()){return false;}
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
            fDummy["index"] = -1; //dummy object for invalid returns, always has an invalid index
        };

        MHO_IndexLabelInterface(const MHO_IndexLabelInterface& copy)
        {
            fIndexLabelObjectPtr = copy.fIndexLabelObjectPtr;
        };

        void SetIndexLabelObject(mho_json* obj){fIndexLabelObjectPtr = obj;}

    public:

        virtual ~MHO_IndexLabelInterface(){};

        std::size_t GetIndexLabelSize() const {return fIndexLabelObjectPtr->size();}

        template< typename XValueType >
        void InsertIndexLabelKeyValue(std::size_t index, const std::string& key, const XValueType& value)
        {
            if(fIndexLabelObjectPtr != nullptr)
            {
                std::string ikey = index2key(index);
                if( !(fIndexLabelObjectPtr->contains(ikey) ) )
                {
                    //no such object, so insert one, make sure it gets an 'index' value
                    // (*fIndexLabelObjectPtr).emplace(ikey, fDummy);
                    (*fIndexLabelObjectPtr).emplace(ikey, fDummy);
                    (*fIndexLabelObjectPtr)[ ikey ]["index"] = index;
                }
                //now update
                mho_json obj;
                obj[key] = value;
                (*fIndexLabelObjectPtr)[ikey].update(obj);
            }
            else 
            {
                msg_error("utilities", "cannot insert key:value pair, index label inteface is missing object!" << eom);
            }
        }

        template< typename XValueType >
        bool RetrieveIndexLabelKeyValue(std::size_t index, const std::string& key, XValueType& value) const
        {
            if(fIndexLabelObjectPtr != nullptr)
            {
                std::string ikey = index2key(index);
                if( (*fIndexLabelObjectPtr)[ikey].contains(key) )
                {
                    value = (*fIndexLabelObjectPtr)[ikey][key].get<XValueType>();
                    return true;
                }
            }
            else 
            {
                msg_error("utilities", "cannot retrieve key:value pair, index label inteface is missing object!" << eom);
            }
            return false;
        }

        //get a reference to the dictionary object associated with this index
        void SetLabelObject(mho_json& obj, std::size_t index)
        {
            if(fIndexLabelObjectPtr != nullptr)
            {
                std::string ikey = index2key(index);
                if( !(fIndexLabelObjectPtr->contains(ikey) ) )
                {
                    //no such object, so insert one, make sure it gets an 'index' value
                    (*fIndexLabelObjectPtr).emplace(ikey, fDummy);
                    (*fIndexLabelObjectPtr)[ ikey ]["index"] = index;
                }

                //make sure the object also contains the index value:
                obj["index"] = index;
                (*fIndexLabelObjectPtr)[ikey].update(obj);
            }
            else 
            {
                msg_error("utilities", "cannot insert label object, index label inteface is missing object!" << eom);
            }
        }

        //get a reference to the dictionary object associated with this index
        mho_json& GetLabelObject(std::size_t index)
        {
            if(fIndexLabelObjectPtr != nullptr)
            {
                std::string ikey = index2key(index);
                return (*fIndexLabelObjectPtr)[ikey];
            }
            else 
            {
                msg_error("utilities", "cannot retrieve label object, index label inteface is missing object!" << eom);
            }
        }

        //get a vector of indexes which contain a key with the same name
        std::vector< std::size_t > GetMatchingIndexes(std::string& key) const
        {
            std::vector<std::size_t> idx;
            if(fIndexLabelObjectPtr != nullptr)
            {
                for(std::size_t i=0; i<fIndexLabelObjectPtr->size(); i++)
                {
                    std::string ikey = index2key(i);
                    if( (*fIndexLabelObjectPtr)[ikey].contains(key) )
                    {
                        idx.push_back( i );
                    }
                }
            }
            else 
            {
                msg_error("utilities", "cannot determine matching indexes, index label inteface is missing object!" << eom);
            }
            return idx;
        }

        //get a vector of indexes which contain a key with a value which matches the passed value
        template< typename XValueType >
        std::vector< std::size_t > GetMatchingIndexes(std::string& key, const XValueType& value) const
        {
            std::vector<std::size_t> idx;
            if(fIndexLabelObjectPtr != nullptr)
            {
                for(std::size_t i=0; i<fIndexLabelObjectPtr->size(); i++)
                {
                    std::string ikey = index2key(i);
                    if( (*fIndexLabelObjectPtr)[ikey].contains(key) )
                    {
                        XValueType v = (*fIndexLabelObjectPtr)[ikey][key].get<XValueType>();
                        if(v == value)
                        {
                            idx.push_back(i);
                        }
                    }
                }
            }
            else 
            {
                msg_error("utilities", "cannot determine matching indexes, index label inteface is missing object!" << eom);
            }
            return idx;
        }


    private:

        static std::string index2key(const std::size_t& idx) {return std::to_string(idx);}

        mho_json* fIndexLabelObjectPtr; //array of mho_json objects holding key:value pairs
        mho_json fDummy;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//constructor is protected
//this class is only intended to provide an interface that derived classes may inherit
//this interface is to enforce a specific access pattern associated with modifying
//meta data attached to a vector/axis like object that is in the form of a mho_json::array_t
class MHO_IntervalLabelInterface
{
    protected:

        MHO_IntervalLabelInterface():fIntervalLabelObjectPtr(nullptr)
        {
            fTokenizer.SetDelimiter(",");
            fDummy["lower_index"] = -1; //dummy object for invalid returns, always has an invalid index
            fDummy["upper_index"] = -1; //dummy object for invalid returns, always has an invalid index
        };

        MHO_IntervalLabelInterface(const MHO_IntervalLabelInterface& copy)
        {
            fTokenizer.SetDelimiter(",");
            fDummy["lower_index"] = -1; //dummy object for invalid returns, always has an invalid index
            fDummy["upper_index"] = -1; //dummy object for invalid returns, always has an invalid index
            fIntervalLabelObjectPtr = copy.fIntervalLabelObjectPtr;
        };

        void SetIntervalLabelObject(mho_json* obj){fIntervalLabelObjectPtr = obj;}

    public:

        virtual ~MHO_IntervalLabelInterface(){};

        template< typename XValueType >
        void InsertIntervalLabelKeyValue(std::size_t lower_index, std::size_t upper_index, const std::string& key, const XValueType& value)
        {
            std::string ikey = ConstructKey(lower_index, upper_index);
            (*fIntervalLabelObjectPtr)[ikey][key] = value;
        }

        template< typename XValueType >
        bool RetrieveIntervalLabelKeyValue(std::size_t lower_index, std::size_t upper_index, const std::string& key, const XValueType& value) const
        {
            std::string ikey = ConstructKey(lower_index, upper_index);
            if(fIntervalLabelObjectPtr->contains(ikey) )
            {
                if( (*fIntervalLabelObjectPtr)[ikey].contains(key) )
                {
                    value =  (*fIntervalLabelObjectPtr)[ikey][key].get<XValueType>();
                    return true;
                }
            }
            else
            {
                msg_warn("containers", "cannot retrieve a key value pair for interval: "<<ikey<<"."<< eom);
            }
            return false;
        }

        //get a reference to the dictionary object associated with this index
        mho_json& GetIntervalLabelObject(std::size_t lower_index, std::size_t upper_index)
        {
            std::string ikey = ConstructKey(lower_index, upper_index);
            if(fIntervalLabelObjectPtr->contains(ikey) )
            {
                return (*fIntervalLabelObjectPtr)[ikey];
            }
            else
            {
                msg_warn("containers", "cannot retrieve interval data for: "<<ikey<<"."<< eom);
                return fDummy;
            }
        }

        void SetIntervalLabelObject(mho_json& obj, std::size_t lower_index, std::size_t upper_index)
        {
            std::string ikey = ConstructKey(lower_index, upper_index);
            obj["lower_index"] = std::min(lower_index, upper_index);
            obj["upper_index"] = std::max(lower_index, upper_index);
            (*fIntervalLabelObjectPtr).emplace(ikey,obj);
        }


        //get a vector of interval labels which contain a key with the same name
        std::vector< mho_json > GetMatchingIntervalLabels(std::string key) const
        {
            std::vector<mho_json> objects;
            for(auto it: fIntervalLabelObjectPtr->items() )
            {
                if( it.value().contains(key) )
                {
                    objects.push_back( it.value() );
                }
            }
            return objects;
        }

        template< typename XLabelValueType >
        mho_json GetFirstIntervalWithKeyValue(std::string key, const XLabelValueType& value) const
        {
            mho_json obj;
            for(auto it: fIntervalLabelObjectPtr->items() )
            {
                if( it.value().contains(key) )
                {
                    XLabelValueType v;
                    v = it.value()[key];
                    if(v == value)
                    {
                        obj = it.value();
                        break;
                    }
                }
            }
            return obj;
        }

        std::string ConstructKey(std::size_t lower_index, std::size_t upper_index) const
        {
            if(lower_index <= upper_index)
            {
                return std::to_string(lower_index) + "," + std::to_string(upper_index);
            }
            else
            {
                //flip them around
                return std::to_string(upper_index) + "," + std::to_string(lower_index);
            }
        }

        bool ExtractIndexesFromKey(const std::string& key, std::size_t& lower_index, std::size_t& upper_index)
        {
            fTokens.clear();
            fTokenizer.SetString(&key);
            fTokenizer.GetTokens(&fTokens);
            if(fTokens.size() != 2 ){return false;}
            lower_index = std::stoul(fTokens[0]);
            upper_index = std::stoul(fTokens[1]);
            if(upper_index < lower_index)
            {
                //flip them
                lower_index = upper_index;
                upper_index = std::stoul(fTokens[0]);
            }
            return true;
        }

        std::pair< std::size_t, std::size_t> ExtractIndexesFromKey(const std::string& key)
        {
            std::size_t lower_index = 0;
            std::size_t upper_index = 0;
            fTokens.clear();
            fTokenizer.SetString(&key);
            fTokenizer.GetTokens(&fTokens);
            if(fTokens.size() != 2 ){std::make_pair(0,0);}
            lower_index = std::stoul(fTokens[0]);
            upper_index = std::stoul(fTokens[1]);
            if(upper_index < lower_index)
            {
                //flip them
                lower_index = upper_index;
                upper_index = std::stoul(fTokens[0]);
            }
            return std::make_pair(lower_index, upper_index);
        }

    private:

        MHO_Tokenizer fTokenizer;
        std::vector< std::string > fTokens;
        std::size_t fCurrentSize;
        mho_json* fIntervalLabelObjectPtr; //array of mho_json objects holding key:value pairs
        mho_json fDummy;
};







}//end namespace

#endif /* end of include guard: JSON_WRAPPER */
