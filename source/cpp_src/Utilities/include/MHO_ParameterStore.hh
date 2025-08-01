#ifndef MHO_ParameterStore_HH__
#define MHO_ParameterStore_HH__

#include <string>

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Tokenizer.hh"

namespace hops
{

/*!
 *@file MHO_ParameterStore.hh
 *@class MHO_ParameterStore
 *@date Wed Jun 7 23:42:31 2023 -0400
 *@author J. Barrett - barrettj@mit.edu
 *@brief class to store parameters (typically from control file) for later retrieval
 *There are some deficiencies with this json-based approach, for example everything except terminal values
 *must be named objects (no lists allowed), though maybe we don't really need that functionality
 *TODO -- allow for variable keys in the value path. Sometimes the name
 *of an item is found within the value of another item, so it would be useful to extract
 *that name, and substitute it into the value path for the item of interest.
 *For example consider the following structure:
 * {
 *
 *     "item0":
 *     {
 *         "key1": "kvalue1",
 *         "key2": "kvalue2"
 *     };
 *     "item1":
 *     {
 *         "kvalue1": "value3",
 *         "kvalue2":
 *          {
 *              "key3": "value4",
 *              "key5": "value5"
 *          }
 *     }
 * }
 *Let's say we wanted to access 'value4', but didn't know the name of the key "kvalue2", (only item1 and key3)
 *but knew it could be located under item1 via a key specified by the value associated with "item0/key2"
 *Then a useful construction to retrieve this would be something like the following (with the variable key's location within braces):
 *std::string vpath = "/item1/{/item0/key2}/key3"  --> this gets translated into "/item1/kvalue2/key3" before retrieval
 *auto value = params.Get<std::string>(vpath);
 */

/**
 * @brief Class MHO_ParameterStore
 */
class MHO_ParameterStore
{
    public:
        MHO_ParameterStore()
        {
            fTokenizer.SetDelimiter("/");
            fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
            fTokenizer.SetIncludeEmptyTokensFalse();
        };

        ~MHO_ParameterStore(){};

        /**
         * @brief Dumps the store JSON data to std:cout (for debugging)
         */
        void Dump() { std::cout << fStore.dump(2) << std::endl; }

        /**
         * @brief Copies the parameter store from a given source.
         *
         * @param copy Reference to the MHO_ParameterStore object to copy from.
         */
        void CopyFrom(const MHO_ParameterStore& copy) { fStore = copy.fStore; }

        /**
         * @brief Copies data from internal storage to provided json object.
         *
         * @param data Output parameter to store copied json data.
         */
        void DumpData(mho_json& data) { data = fStore; }

        /**
         * @brief Stores input json data for later use.
         *
         * @param data Input json data to be stored.
         */
        void FillData(const mho_json& data) { fStore = data; }

        /**
         * @brief Clears all data from fStore.
         */
        void ClearData() { fStore.clear(); }

        //returns true if no error adding value
        /**
         * @brief Setter for value at specified path in the parameter store.
         *
         * @param value_path Path to the value as a string reference.
         * @param value Value to set at the given path.
         * @return True if value was successfully set, false otherwise.
         */
        template< typename XValueType > bool Set(const std::string& value_path, const XValueType& value);

        //returns true if found
        /**
         * @brief Retrieves a value by path and returns it as XValueType, using default constructor if not found.
         *
         * @tparam XValueType Template parameter XValueType
         * @param value_path Path to the value to retrieve
         * @param value (XValueType&)
         * @return XValueType value or default constructed value if not found
         */
        template< typename XValueType > bool Get(const std::string& value_path, XValueType& value) const;

        //always returns a value, if not found the value returned is XValueType()
        /**
         * @brief Function IsPresent
         *
         * @tparam XValueType Template parameter XValueType
         * @param value_path (const std::string&)
         * @return Return value (bool)
         */
        /**
         * @brief Getter for as
         *
         * @tparam XValueType Template parameter XValueType
         * @param value_path Path to retrieve value from
         * @return Value of type XValueType retrieved from path or default if not found
         */
        template< typename XValueType > XValueType GetAs(const std::string& value_path) const;

        bool IsPresent(const std::string& value_path) const
        {
            std::string path = SanitizePath(value_path);
            fPath.clear();
            fTokenizer.SetString(&path);
            fTokenizer.GetTokens(&fPath);

            auto* p = &fStore;
            for(auto it = fPath.begin(); it != fPath.end(); it++)
            {
                auto jit = p->find(*it);
                if(jit == p->end())
                {
                    return false;
                }
                else if(*it == *(fPath.rbegin()))
                {
                    return true;
                }
                else
                {
                    p = &(jit.value());
                }
            }
            return false;
        }

    private:
        //sanitize the value_path string -- for example a trailing '/' is no good
        /**
         * @brief Removes leading/trailing whitespace and trailing '/' from input path string.
         *
         * @param value_path Input path string to sanitize
         * @return Sanitized path string
         */
        std::string SanitizePath(const std::string& value_path) const
        {
            std::string vpath = MHO_Tokenizer::TrimLeadingAndTrailingWhitespace(value_path);
            if(vpath.size() > 0 && vpath.back() == '/') //trim any trailing '/'
            {
                vpath.pop_back();
            }
            return vpath;
        }

        //helpers
        mutable MHO_Tokenizer fTokenizer;
        mutable std::vector< std::string > fPath;

        //stash data in a json object
        mho_json fStore;
};

/**
 * @brief Function MHO_ParameterStore::Set
 *
 * @param value_path (const std::string&)
 * @param value (const XValueType&)
 * @return Return value (template< typename XValueType > bool)
 */
template< typename XValueType > bool MHO_ParameterStore::Set(const std::string& value_path, const XValueType& value)
{
    //first we tokenize the value path into a sequence of names
    std::string path = SanitizePath(value_path);
    fPath.clear();
    fTokenizer.SetString(&path);
    fTokenizer.GetTokens(&fPath);

    bool ok = false;
    if(fPath.size() > 0)
    {
        //insert and/or replace an object
        auto* p = &fStore;
        for(auto it = fPath.begin(); it != fPath.end(); it++)
        {
            if(*it != *(fPath.rbegin()))
            {
                if(p->contains(*it)) //item with path exists
                {
                    p = &(p->at(*it)); //point to this item
                }
                else
                {
                    //item doesn't exist yet, so create an entry with this key (*it)
                    p = &(*p)[*it];
                }
            }
            else //we've arrived at the terminal point, so set this item to the value
            {
                p = &(*p)[*it];
                *p = value;
                ok = true;
            }
        }
    }
    return ok;
}

/**
 * @brief Retrieves a value from the parameter store by path and returns it as XValueType.
 *
 * @param value_path Path to the value in the parameter store
 * @param value (XValueType&)
 * @return True if retrieval was successful, false otherwise
 */
template< typename XValueType > bool MHO_ParameterStore::Get(const std::string& value_path, XValueType& value) const
{
    //NOTE: we do not use json_pointer to access values specified by path
    //because it will throw an exception if used when the path is not present/complete
    std::string path = SanitizePath(value_path);
    fPath.clear();
    fTokenizer.SetString(&path);
    fTokenizer.GetTokens(&fPath);

    bool present = false;
    auto* p = &fStore;
    for(auto it = fPath.begin(); it != fPath.end(); it++)
    {
        auto jit = p->find(*it);
        if(jit == p->end())
        {
            return false;
        }
        else if(*it == *(fPath.rbegin()))
        {
            value = jit->get< XValueType >();
            return true;
        }
        else
        {
            p = &(jit.value());
        }
    }
    return false;
}

/**
 * @brief Retrieves and returns a value as specified type from the parameter store using the given path.
 *
 * @param value_path Path to the desired value in the parameter store
 * @return Value retrieved as specified template type or default value if retrieval fails
 */
template< typename XValueType > XValueType MHO_ParameterStore::GetAs(const std::string& value_path) const
{
    XValueType v = XValueType(); //default constructor (zero for int, double, etc)
    bool ok = Get(value_path, v);
    if(!ok)
    {
        msg_error("utility", "failed to retrieve value: " << value_path << " returning a default value." << eom);
    }
    return v;
}

} // namespace hops

#endif /*! end of include guard: MHO_ParameterStore_HH__ */
