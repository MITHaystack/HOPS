#ifndef MHO_IntervalLabelInterface_HH__
#define MHO_IntervalLabelInterface_HH__


#include "MHO_JSONHeaderWrapper.hh"

namespace hops
{

/*!
*@file MHO_IntervalLabelInterface.hh
*@class MHO_IntervalLabelInterface
*@date Sun Feb 4 17:21:38 2024 -0500
*@brief
*@author J. Barrett - barrettj@mit.edu
*/


//constructor is protected
//this class is only intended to provide an interface that derived classes may inherit
//this interface is to enforce a specific access pattern associated with modifying
//meta data attached to a vector/axis like object that is in the form of a mho_json::array_t
class MHO_IntervalLabelInterface
{
    protected:

        MHO_IntervalLabelInterface():fIntervalLabelObjectPtr(nullptr)
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

        void SetIntervalLabelObject(mho_json* obj){fIntervalLabelObjectPtr = obj;}

    public:

        virtual ~MHO_IntervalLabelInterface(){};

        void ClearIntervalLabels()
        {
            *fIntervalLabelObjectPtr = mho_json();
        }

        template< typename XValueType >
        void InsertIntervalLabelKeyValue(std::size_t lower_index, std::size_t upper_index, const std::string& key, const XValueType& value)
        {
            std::string ikey = ConstructKey(lower_index, upper_index);
            (*fIntervalLabelObjectPtr)[ikey][key] = value;
            (*fIntervalLabelObjectPtr)[ikey]["lower_index"] = lower_index;
            (*fIntervalLabelObjectPtr)[ikey]["upper_index"] = upper_index;
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

        bool ExtractIndexesFromKey(const std::string& key, std::size_t& lower_index, std::size_t& upper_index)
        {
            lower_index = 0;
            upper_index = 0;
            if(key.find(',') == std::string::npos){return false;}
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

        static std::pair< std::size_t, std::size_t> ExtractIndexesFromKey(const std::string& key)
        {
            std::size_t lower_index = 0;
            std::size_t upper_index = 0;
            size_t pos = key.find(',');
            if(pos == std::string::npos){return std::make_pair(lower_index,upper_index);}
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


} //end namespace


#endif /*! end of include guard: MHO_IntervalLabelInterface_HH__ */
