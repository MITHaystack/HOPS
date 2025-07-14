#ifndef MHO_IndexLabelInterface_HH__
#define MHO_IndexLabelInterface_HH__

#include "MHO_JSONHeaderWrapper.hh"

namespace hops
{

/*!
 *@file MHO_IndexLabelInterface.hh
 *@class MHO_IndexLabelInterface
 *@date Sun Feb 4 17:21:38 2024 -0500
 *@brief Class MHO_IndexLabelInterface - adds indexes associated with key:value pairs (used by MHO_Axis)
 * constructor is protected
 * this class is only intended to provide an interface that derived classes may inherit
 * this interface is to enforce a specific access pattern associated with modifying
 * meta data attached to a vector/axis like object that is in the form of a mho_json::array_t
 *@author J. Barrett - barrettj@mit.edu
 */


class MHO_IndexLabelInterface
{

    protected:
        MHO_IndexLabelInterface(): fIndexLabelObjectPtr(nullptr)
        {
            fDummy["index"] = -1; //dummy object for invalid returns, always has an invalid index
        };

        MHO_IndexLabelInterface(const MHO_IndexLabelInterface& copy) { fIndexLabelObjectPtr = copy.fIndexLabelObjectPtr; };

        /**
         * @brief Setter for index label object
         * 
         * @param obj Pointer to mho_json object
         */
        void SetIndexLabelObject(mho_json* obj) { fIndexLabelObjectPtr = obj; }

    public:
        virtual ~MHO_IndexLabelInterface(){};

        /**
         * @brief Getter for index label size
         * 
         * @return Size of the index label object as std::size_t
         */
        std::size_t GetIndexLabelSize() const { return fIndexLabelObjectPtr->size(); }

        /**
         * @brief Clears all index labels.
         */
        void ClearIndexLabels()
        {
            //(*fIndexLabelObjectPtr) = mho_json();
            fIndexLabelObjectPtr->clear();
        }

        /**
         * @brief Function InsertIndexLabelKeyValue
         * 
         * @tparam XValueType Template parameter XValueType
         * @param index (std::size_t)
         * @param key (const std::string&)
         * @param value (const XValueType&)
         */
        template< typename XValueType >
        void InsertIndexLabelKeyValue(std::size_t index, const std::string& key, const XValueType& value)
        {
            if(fIndexLabelObjectPtr != nullptr)
            {
                std::string ikey = index2key(index);
                if(!(fIndexLabelObjectPtr->contains(ikey)))
                {
                    //no such object, so insert one, make sure it gets an 'index' value
                    (*fIndexLabelObjectPtr).emplace(ikey, fDummy);
                    (*fIndexLabelObjectPtr)[ikey]["index"] = index;
                }
                //now update
                mho_json obj;
                obj[key] = value;
                (*fIndexLabelObjectPtr)[ikey].update(obj);
            }
            else
            {
                msg_error("utilities", "cannot insert key:value pair, index label interface is missing object!" << eom);
            }
        }

        /**
         * @brief Retrieves value associated with given key and index from IndexLabelObjectPtr if it exists.
         * 
         * @tparam XValueType Template parameter XValueType
         * @param index Index to retrieve key-value pair for
         * @param key Key to search for in IndexLabelObjectPtr
         * @param value Reference to store retrieved value of type XValueType
         * @return True if retrieval was successful, false otherwise
         */
        template< typename XValueType >
        bool RetrieveIndexLabelKeyValue(std::size_t index, const std::string& key, XValueType& value) const
        {
            if(fIndexLabelObjectPtr != nullptr)
            {
                std::string ikey = index2key(index);
                if((*fIndexLabelObjectPtr)[ikey].contains(key))
                {
                    value = (*fIndexLabelObjectPtr)[ikey][key].get< XValueType >();
                    return true;
                }
            }
            else
            {
                msg_error("utilities", "cannot retrieve key:value pair, index label interface is missing object!" << eom);
            }
            return false;
        }

        /**
         * @brief get a reference to the dictionary object associated with this index
         * 
         * @param obj (mho_json&)
         * @param index (std::size_t)
         */
        void SetLabelObject(mho_json& obj, std::size_t index)
        {
            if(fIndexLabelObjectPtr != nullptr)
            {
                if(obj.is_null())
                {
                    return;
                }
                if(!(fIndexLabelObjectPtr->is_object()))
                {
                    (*fIndexLabelObjectPtr) = mho_json();
                } //something blew away our object, reset
                std::string ikey = index2key(index);
                if(!(fIndexLabelObjectPtr->contains(ikey)))
                {
                    //no such object, so insert one, make sure it gets an 'index' value
                    // (*fIndexLabelObjectPtr).emplace(ikey, fDummy);
                    (*fIndexLabelObjectPtr)[ikey] = fDummy;
                    (*fIndexLabelObjectPtr)[ikey]["index"] = index;
                }

                //make sure the object also contains the index value:
                obj["index"] = index;
                (*fIndexLabelObjectPtr)[ikey].update(obj);
            }
            else
            {
                msg_error("utilities", "cannot insert label object, index label interface is missing object!" << eom);
            }
        }

        /**
         * @brief get a reference to the dictionary object associated with this index
         * 
         * @param index Index of type std::size_t used to locate the label object.
         * @return Reference to the mho_json object associated with the given index, or a dummy object if the index label interface is missing.
         */
        mho_json& GetLabelObject(std::size_t index)
        {
            if(fIndexLabelObjectPtr != nullptr)
            {
                std::string ikey = index2key(index);
                return (*fIndexLabelObjectPtr)[ikey];
            }
            else
            {
                msg_error("utilities", "cannot retrieve label object, index label interface is missing object!" << eom);
                return fDummy;
            }
        }

        /**
         * @brief Getter for label object
         * 
         * @param index Index of type std::size_t for retrieving label object
         * @return mho_json reference to label object if found, otherwise dummy object
         */
        mho_json GetLabelObject(std::size_t index) const
        {
            if(fIndexLabelObjectPtr != nullptr)
            {
                std::string ikey = index2key(index);
                return (*fIndexLabelObjectPtr)[ikey];
            }
            else
            {
                msg_error("utilities", "cannot retrieve label object, index label interface is missing object!" << eom);
                return fDummy;
            }
        }

        /**
         * @brief get a vector of indexes which contain a key with the same name
         * 
         * @param key Input key to search for
         * @return Vector of matching indexes
         */
        std::vector< std::size_t > GetMatchingIndexes(std::string& key) const
        {
            std::vector< std::size_t > idx;
            if(fIndexLabelObjectPtr != nullptr)
            {
                for(std::size_t i = 0; i < fIndexLabelObjectPtr->size(); i++)
                {
                    std::string ikey = index2key(i);
                    if((*fIndexLabelObjectPtr)[ikey].contains(key))
                    {
                        idx.push_back(i);
                    }
                }
            }
            else
            {
                msg_error("utilities", "cannot determine matching indexes, index label interface is missing object!" << eom);
            }
            return idx;
        }

        /**
         * @brief Get a vector of indexes which contain a key with a value which matches the passed value
         * 
         * @tparam XValueType Template parameter XValueType
         * @param key Input key string to match
         * @param value (const XValueType&)
         * @return Vector of matching indexes
         */
        template< typename XValueType >
        std::vector< std::size_t > GetMatchingIndexes(std::string& key, const XValueType& value) const
        {
            std::vector< std::size_t > idx;
            if(fIndexLabelObjectPtr != nullptr)
            {
                for(std::size_t i = 0; i < fIndexLabelObjectPtr->size(); i++)
                {
                    std::string ikey = index2key(i);
                    if((*fIndexLabelObjectPtr)[ikey].contains(key))
                    {
                        XValueType v = (*fIndexLabelObjectPtr)[ikey][key].get< XValueType >();
                        if(v == value)
                        {
                            idx.push_back(i);
                        }
                    }
                }
            }
            else
            {
                msg_error("utilities", "cannot determine matching indexes, index label interface is missing object!" << eom);
            }
            return idx;
        }

    private:

        /**
         * @brief Converts an index to a string key.
         * 
         * @param idx Input index as size_t
         * @return String representation of the input index
         * @note This is a static function.
         */
        static std::string index2key(const std::size_t& idx) { return std::to_string(idx); }

        mho_json* fIndexLabelObjectPtr; //array of mho_json objects holding key:value pairs
        mho_json fDummy;
};

} // namespace hops

#endif /*! end of include guard: MHO_IndexLabelInterface_HH__ */
