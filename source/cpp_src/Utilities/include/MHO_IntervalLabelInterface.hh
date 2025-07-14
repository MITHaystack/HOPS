#ifndef MHO_IntervalLabelInterface_HH__
#define MHO_IntervalLabelInterface_HH__

#include "MHO_JSONHeaderWrapper.hh"

namespace hops
{

/*!
 *@file MHO_IntervalLabelInterface.hh
 *@class MHO_IntervalLabelInterface
 *@date Sun Feb 4 17:21:38 2024 -0500
 *@brief Class MHO_IntervalLabelInterface - adds intervals with associated key:value pairs (used by MHO_Axis)
 * constructor is protected
 * this class is only intended to provide an interface that derived classes may inherit
 * this interface is to enforce a specific access pattern associated with modifying
 * meta data attached to a vector/axis like object that is in the form of a mho_json::array_t
 *@author J. Barrett - barrettj@mit.edu
 */


class MHO_IntervalLabelInterface
{
    protected:
        MHO_IntervalLabelInterface(): fIntervalLabelObjectPtr(nullptr)
        {
            // fTokenizer.SetDelimiter(",");
            fDummy["lower_index"] = -1; //dummy object for invalid returns, always has an invalid index
            fDummy["upper_index"] = -1; //dummy object for invalid returns, always has an invalid index
        };

        MHO_IntervalLabelInterface(const MHO_IntervalLabelInterface& copy)
        {
            // fTokenizer.SetDelimiter(",");
            fDummy["lower_index"] = -1; //dummy object for invalid returns, always has an invalid index
            fDummy["upper_index"] = -1; //dummy object for invalid returns, always has an invalid index
            fIntervalLabelObjectPtr = copy.fIntervalLabelObjectPtr;
        };

        /**
         * @brief Setter for interval label object
         * 
         * @param obj Input mho_json object to set as interval label object
         */
        void SetIntervalLabelObject(mho_json* obj) { fIntervalLabelObjectPtr = obj; }

    public:
        virtual ~MHO_IntervalLabelInterface(){};

        /**
         * @brief Clears interval labels by setting *fIntervalLabelObjectPtr to an empty json object.
         */
        void ClearIntervalLabels() { *fIntervalLabelObjectPtr = mho_json(); }

        /**
         * @brief Inserts an interval label key-value pair into a map referenced by fIntervalLabelObjectPtr.
         * 
         * @tparam XValueType Template parameter XValueType
         * @param lower_index Lower bound index for the interval
         * @param upper_index Upper bound index for the interval
         * @param key Key string for the value to be inserted
         * @param value Value associated with the given key
         */
        template< typename XValueType >
        void InsertIntervalLabelKeyValue(std::size_t lower_index, std::size_t upper_index, const std::string& key,
                                         const XValueType& value)
        {
            std::string ikey = ConstructKey(lower_index, upper_index);
            (*fIntervalLabelObjectPtr)[ikey][key] = value;
            (*fIntervalLabelObjectPtr)[ikey]["lower_index"] = lower_index;
            (*fIntervalLabelObjectPtr)[ikey]["upper_index"] = upper_index;
        }

        /**
         * @brief Retrieves a value associated with a key within an interval range.
         * 
         * @tparam XValueType Template parameter XValueType
         * @param lower_index Lower bound index for the interval.
         * @param upper_index Upper bound index for the interval.
         * @param key Key to search for in the interval's label object.
         * @param value Reference to store the retrieved value.
         * @return True if retrieval was successful, false otherwise.
         */
        template< typename XValueType >
        bool RetrieveIntervalLabelKeyValue(std::size_t lower_index, std::size_t upper_index, const std::string& key,
                                           const XValueType& value) const
        {
            std::string ikey = ConstructKey(lower_index, upper_index);
            if(fIntervalLabelObjectPtr->contains(ikey))
            {
                if((*fIntervalLabelObjectPtr)[ikey].contains(key))
                {
                    value = (*fIntervalLabelObjectPtr)[ikey][key].get< XValueType >();
                    return true;
                }
            }
            else
            {
                msg_warn("containers", "cannot retrieve a key value pair for interval: " << ikey << "." << eom);
            }
            return false;
        }

        /**
         * @brief Get a reference to the dictionary object associated with this index
         * 
         * @param lower_index Lower index for interval
         * @param upper_index Upper index for interval
         * @return Reference to mho_json object if exists, else dummy object
         */
        mho_json& GetIntervalLabelObject(std::size_t lower_index, std::size_t upper_index)
        {
            std::string ikey = ConstructKey(lower_index, upper_index);
            if(fIntervalLabelObjectPtr->contains(ikey))
            {
                return (*fIntervalLabelObjectPtr)[ikey];
            }
            else
            {
                msg_warn("containers", "cannot retrieve interval data for: " << ikey << "." << eom);
                return fDummy;
            }
        }

        /**
         * @brief Setter for interval label object
         * 
         * @param obj Reference to mho_json object containing metadata
         * @param lower_index Lower index of interval
         * @param upper_index Upper index of interval
         */
        void SetIntervalLabelObject(mho_json& obj, std::size_t lower_index, std::size_t upper_index)
        {
            std::string ikey = ConstructKey(lower_index, upper_index);
            obj["lower_index"] = std::min(lower_index, upper_index);
            obj["upper_index"] = std::max(lower_index, upper_index);
            (*fIntervalLabelObjectPtr)[ikey] = obj;
            // (*fIntervalLabelObjectPtr).emplace(ikey,obj);
        }

        /**
         * @brief get a vector of interval labels which contain a key with the same name
         * 
         * @param key Key to search for within interval labels
         * @return Vector of matching mho_json objects
         */
        std::vector< mho_json > GetMatchingIntervalLabels(std::string key) const
        {
            std::vector< mho_json > objects;
            for(auto it : fIntervalLabelObjectPtr->items())
            {
                if(it.value().contains(key))
                {
                    objects.push_back(it.value());
                }
            }
            return objects;
        }

        /**
         * @brief Getter for first interval with key value
         * 
         * @tparam XLabelValueType Template parameter XLabelValueType
         * @param key Key to search for in interval labels
         * @param value Value associated with the key to match
         * @return First matching mho_json object or empty if none found
         */
        template< typename XLabelValueType >
        mho_json GetFirstIntervalWithKeyValue(std::string key, const XLabelValueType& value) const
        {
            mho_json obj;
            for(auto it : fIntervalLabelObjectPtr->items())
            {
                if(it.value().contains(key))
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

        /**
         * @brief Constructs a key string from lower and upper indices, ensuring lower_index <= upper_index.
         * 
         * @param lower_index Lower index value (must be less than or equal to upper_index)
         * @param upper_index Upper index value
         * @return String representation of the sorted pair of indices separated by a comma
         * @note This is a static function.
         */
        static std::string ConstructKey(std::size_t lower_index, std::size_t upper_index)
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

        /**
         * @brief Extracts lower and upper indexes from a key string separated by comma.
         * 
         * @param key Input key string containing two indexes separated by comma.
         * @param lower_index (std::size_t&)
         * @param upper_index (std::size_t&)
         * @return True if extraction is successful, false otherwise.
         */
        bool ExtractIndexesFromKey(const std::string& key, std::size_t& lower_index, std::size_t& upper_index)
        {
            lower_index = 0;
            upper_index = 0;
            if(key.find(',') == std::string::npos)
            {
                return false;
            }
            auto idx_pair = ExtractIndexesFromKey(key);
            lower_index = idx_pair.first;
            upper_index = idx_pair.second;
            if(upper_index < lower_index)
            {
                lower_index = idx_pair.second;
                upper_index = idx_pair.first;
            }
            return true;
        }

        /**
         * @brief Extracts lower and upper indexes from a key string separated by comma.
         * 
         * @param key Input key string containing two indexes separated by comma.
         * @return True if extraction is successful, false otherwise.
         * @note This is a static function.
         */
        static std::pair< std::size_t, std::size_t > ExtractIndexesFromKey(const std::string& key)
        {
            std::size_t lower_index = 0;
            std::size_t upper_index = 0;
            size_t pos = key.find(',');
            if(pos == std::string::npos)
            {
                return std::make_pair(lower_index, upper_index);
            }
            std::string first = key.substr(0, pos);
            std::string second = key.substr(pos + 1);
            lower_index = std::stoul(first);
            upper_index = std::stoul(second);
            if(upper_index < lower_index)
            {
                //flip them
                std::make_pair(upper_index, lower_index);
            }
            return std::make_pair(lower_index, upper_index);
        }

    private:
        mho_json* fIntervalLabelObjectPtr; //array of mho_json objects holding key:value pairs
        mho_json fDummy;
};

} // namespace hops

#endif /*! end of include guard: MHO_IntervalLabelInterface_HH__ */
