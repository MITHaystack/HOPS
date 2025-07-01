#ifndef JSON_WRAPPER_HH__
#define JSON_WRAPPER_HH__

#include "nlohmann/json.hpp"
using mho_json = nlohmann::json;
using mho_ordered_json = nlohmann::ordered_json;
namespace nl = nlohmann;

#include <sstream>
#include <string>

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

/*!
 *@file MHO_JSONHeaderWrapper.hh
 *@class MHO_JSONHeaderWrapper
 *@date Thu May 27 13:13:02 2021 -0400
 *@brief
 *@author J. Barrett - barrettj@mit.edu
 */

//constructor is protected
//this class is only intended to provide an interface that derived classes may inherit
class MHO_JSONWrapper
{
    protected:
        MHO_JSONWrapper(){};

        MHO_JSONWrapper(const MHO_JSONWrapper& copy) { fObject = copy.fObject; };

        /**
         * @brief Setter for object
         * 
         * @param obj Input mho_json object to set
         */
        void SetObject(mho_json obj) { fObject = obj; }

    protected:
        mho_json fObject;

    public:
        virtual ~MHO_JSONWrapper(){};

        /**
         * @brief Checks if a key exists in the internal map.
         * 
         * @param key The key to search for in the map.
         * @return True if the key is found, false otherwise.
         */
        bool HasKey(const std::string& key) const
        {
            auto it = fObject.find(key);
            if(it != fObject.end())
            {
                return true;
            }
            return false;
        }

        /**
         * @brief Checks if a key exists in the internal map.
         * 
         * @param char_key (const char*)
         * @return True if the key is found, false otherwise.
         */
        bool HasKey(const char* char_key) const
        {
            std::string key(char_key);
            return HasKey(key);
        }

        MHO_JSONWrapper& operator=(const MHO_JSONWrapper& rhs)
        {
            if(this != &rhs)
            {
                fObject = rhs.fObject;
            }
            return *this;
        }

        /**
         * @brief Clears the internal object.
         */
        void Clear() { fObject.clear(); }

        /**
         * @brief Inserts or replaces an object in a map using a key and value.
         * 
         * @param key The unique identifier for the object to be inserted/replaced.
         * @param value (const XValueType&)
         * @return No return value (void)
         */
        template< typename XValueType > void Insert(const std::string& key, const XValueType& value)
        {
            //allow replacement of values
            fObject[key] = value;
        }

        /**
         * @brief Retrieves a value from an object by key and casts it to the specified type.
         * 
         * @param key Key used to lookup the value in the object
         * @param value (XValueType&)
         * @return True if retrieval was successful, false otherwise
         */
        template< typename XValueType > bool Retrieve(const std::string& key, XValueType& value) const
        {
            auto iter = fObject.find(key);
            if(iter == fObject.end())
            {
                return false;
            }
            mho_json test;
            test["test"] = value;
            if(iter.value().type() == test.begin().value().type())
            {
                value = iter.value().get< XValueType >();
                return true;
            }
            else
            {
                return false;
            }
        }

        /**
         * @brief Dumps all keys from the internal object.
         * 
         * @return Vector of strings containing all keys
         */
        std::vector< std::string > DumpKeys() const
        {
            std::vector< std::string > keys;
            for(auto iter = fObject.begin(); iter != fObject.end(); iter++)
            {
                keys.push_back(iter.key());
            }
            return keys;
        }

        //TODO eliminate me
        /**
         * @brief Dumps the contents of the map fObject to standard output.
         */
        void DumpMap() const
        {
            for(auto iter = fObject.begin(); iter != fObject.end(); iter++)
            {
                std::cout << iter.key() << " : " << iter.value() << std::endl;
            }
        }

        /**
         * @brief Checks if a map contains a specific key.
         * 
         * @param key The key to search for in the map.
         * @return True if the key is found, false otherwise.
         */
        bool ContainsKey(const std::string& key) const
        {
            auto iter = fObject.find(key);
            if(iter == fObject.end())
            {
                return false;
            }
            else
            {
                return true;
            }
        }
};

//specialize for mho_json
/**
 * @brief Retrieves a value from the JSON object by key and stores it in the provided reference.
 * 
 * @param key Key to search for in the JSON object
 * @param value Reference to store the retrieved value
 * @return True if retrieval was successful, false otherwise
 */
template<> inline bool MHO_JSONWrapper::Retrieve(const std::string& key, mho_json& value) const
{
    auto iter = fObject.find(key);
    if(iter == fObject.end())
    {
        return false;
    }
    value = fObject[key];
    return true;
}

} // namespace hops

#endif /*! end of include guard: JSON_WRAPPER */
