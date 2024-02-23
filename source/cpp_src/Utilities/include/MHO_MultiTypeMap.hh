#ifndef MHO_MultiTypeMap_HH__
#define MHO_MultiTypeMap_HH__


/*!
*@file MHO_MultiTypeMap.hh
*@class MHO_MultiTypeMap
*@author J. Barrett - barrettj@mit.edu 
*@date 2020-05-15T20:56:55.583Z
*@brief
*/

#include <set>
#include <map>
#include <string>
#include <iostream>
#include <vector>

#include "MHO_Types.hh"
#include "MHO_Meta.hh"

namespace hops
{

template< typename XKeyType, typename XValueType >
class MHO_SingleTypeMap
{
    public:

        MHO_SingleTypeMap(){};
        virtual ~MHO_SingleTypeMap(){};

        std::size_t MapSize() const {return fMap.size(); }

        void Insert(const XKeyType& key, const XValueType& value)
        {
            fMap[key] = value;//allow replacement of values (as opposed to line below)
            //fMap.insert( std::pair<XKeyType, XValueType>(key,value) );
        }

        void Clear()
        {
            fMap.clear();
        }

        bool Retrieve(const XKeyType& key, XValueType& value) const
        {
            auto iter = fMap.find(key);
            if(iter == fMap.end())
            {
                return false;
            }
            else
            {
                value = iter->second;
                return true;
            }
        }

        std::vector<XKeyType> DumpKeys() const
        {
            std::vector< XKeyType > keys;
            for(auto iter = fMap.begin(); iter != fMap.end(); iter++)
            {
                keys.push_back(iter->first);
            }
            return keys;
        }

        void DumpMap() const
        {
            for(auto iter = fMap.begin(); iter != fMap.end(); iter++)
            {
                std::cout<<iter->first<<" : "<<iter->second<<std::endl;
            }
        }

        bool ContainsKey(const XKeyType& key) const
        {
            auto iter = fMap.find(key);
            if(iter == fMap.end()){return false;}
            else{return true;}
        }

        void CopyFrom(const MHO_SingleTypeMap<XKeyType, XValueType>& copy_from_obj)
        {
            if(this != &copy_from_obj)
            {
                fMap.clear();
                fMap = copy_from_obj.fMap;
            }
        }

        void CopyTo(MHO_SingleTypeMap<XKeyType, XValueType>& copy_to_obj) const
        {
            if(this != &copy_to_obj)
            {
                copy_to_obj.fMap.clear();
                copy_to_obj.fMap = fMap;
            }
        }

    private:

        std::map< XKeyType, XValueType > fMap;
};

//declare a multi-type map which takes a key type, and variadic template parameter for the types to be stored
template <typename XKeyType, typename... XValueTypeS>
class MHO_MultiTypeMap;

//declare the specialization for the base case of the recursion (in which the parameter XValueType is just a single type)
template <typename  XKeyType, typename XValueType>
class MHO_MultiTypeMap< XKeyType, XValueType >: public MHO_SingleTypeMap< XKeyType, XValueType >
{
    public:
        using MHO_SingleTypeMap< XKeyType, XValueType >::MapSize;
        using MHO_SingleTypeMap< XKeyType, XValueType >::Insert;
        using MHO_SingleTypeMap< XKeyType, XValueType >::Retrieve;
        using MHO_SingleTypeMap< XKeyType, XValueType >::ContainsKey;
        using MHO_SingleTypeMap< XKeyType, XValueType >::DumpKeys;
        using MHO_SingleTypeMap< XKeyType, XValueType >::DumpMap;
        using MHO_SingleTypeMap< XKeyType, XValueType >::CopyFrom;
        using MHO_SingleTypeMap< XKeyType, XValueType >::CopyTo;
        using MHO_SingleTypeMap< XKeyType, XValueType >::Clear;

        template<typename U = XValueType> typename std::enable_if< std::is_same<U,XValueType >::value, std::size_t >::type
        MapSize() const
        {
            return static_cast< const MHO_SingleTypeMap< XKeyType, XValueType >* >( this )->MapSize();
        };

        template<typename U = XValueType> typename std::enable_if< std::is_same<U,XValueType>::value, std::vector<XKeyType> >::type
        DumpKeys() const
        {
            return static_cast< const MHO_SingleTypeMap< XKeyType, XValueType >* >( this )->DumpKeys();
        };

        template<typename U = XValueType> typename std::enable_if<std::is_same<U,XValueType>::value>::type
        DumpMap() const
        {
            static_cast< const MHO_SingleTypeMap< XKeyType, XValueType >* >( this )->DumpMap();
        };

        template<typename U = XValueType> typename std::enable_if<std::is_same<U,XValueType>::value, bool>::type
        ContainsKey(const XKeyType& key) const
        {
            return static_cast< const MHO_SingleTypeMap< XKeyType, XValueType >* >( this )->ContainsKey(key);
        };

        template<typename U = XValueType> typename std::enable_if<std::is_same<U,XValueType>::value>::type
        CopyFrom(const MHO_MultiTypeMap<XKeyType, XValueType>& copy_from_obj)
        {
            static_cast< MHO_SingleTypeMap< XKeyType, XValueType >* >( this )->
            CopyFrom( *(static_cast< const MHO_SingleTypeMap< XKeyType, XValueType >* >(&copy_from_obj)) );
        };

        template<typename U = XValueType> typename std::enable_if<std::is_same<U,XValueType>::value>::type
        CopyTo(MHO_MultiTypeMap<XKeyType, XValueType>& copy_to_obj) const
        {
            static_cast< const MHO_SingleTypeMap< XKeyType, XValueType >* >( this )->
            CopyTo( *(static_cast< MHO_SingleTypeMap< XKeyType, XValueType >* >(&copy_to_obj)) );
        };

        template<typename U = XValueType> typename std::enable_if<std::is_same<U,XValueType>::value>::type
        Clear()
        {
            static_cast< MHO_SingleTypeMap< XKeyType, XValueType >* >( this )->Clear();
        };



};

//now set up the recursion
template< typename XKeyType, typename XValueType, typename... XValueTypeS >
class MHO_MultiTypeMap< XKeyType, XValueType, XValueTypeS...>: public MHO_MultiTypeMap< XKeyType, XValueType >, public MHO_MultiTypeMap< XKeyType, XValueTypeS... >
{
    public:

        using MHO_MultiTypeMap< XKeyType, XValueType >::MapSize;
        using MHO_MultiTypeMap< XKeyType, XValueTypeS... >::MapSize;

        using MHO_MultiTypeMap< XKeyType, XValueType >::Insert;
        using MHO_MultiTypeMap< XKeyType, XValueTypeS... >::Insert;

        using MHO_MultiTypeMap< XKeyType, XValueType >::Retrieve;
        using MHO_MultiTypeMap< XKeyType, XValueTypeS... >::Retrieve;

        using MHO_MultiTypeMap< XKeyType, XValueType >::ContainsKey;
        using MHO_MultiTypeMap< XKeyType, XValueTypeS... >::ContainsKey;

        using MHO_MultiTypeMap< XKeyType, XValueType >::DumpKeys;
        using MHO_MultiTypeMap< XKeyType, XValueTypeS... >::DumpKeys;

        using MHO_MultiTypeMap< XKeyType, XValueType >::DumpMap;
        using MHO_MultiTypeMap< XKeyType, XValueTypeS... >::DumpMap;

        using MHO_MultiTypeMap< XKeyType, XValueType >::CopyFrom;
        using MHO_MultiTypeMap< XKeyType, XValueTypeS... >::CopyFrom;

        using MHO_MultiTypeMap< XKeyType, XValueType >::CopyTo;
        using MHO_MultiTypeMap< XKeyType, XValueTypeS... >::CopyTo;

        using MHO_MultiTypeMap< XKeyType, XValueType >::Clear;
        using MHO_MultiTypeMap< XKeyType, XValueTypeS... >::Clear;

};

//convenience definitions below ////////////////////////////////////////////////

//TODO: Make sure this set of types is complete for data axis labeling needs
//Consider what other types might be needed (float? short? dates?)
//#pragma message("TODO FIXME -- need to specify fixed sized types in MHO_MultiTypeMap from cstdint for portability.")
typedef MHO_MultiTypeMap< std::string, char, bool, int, double, std::string > MHO_CommonLabelMap;

template< typename XItemType >
struct cm_size_calculator
{
    const XItemType* item;
    uint64_t get_item_size()
    {
        return (uint64_t) sizeof(XItemType);
    }
};

template<>
struct cm_size_calculator<std::string>
{
    const std::string* item;
    uint64_t get_item_size()
    {
        //every string get streamed with a 'size'
        uint64_t total_size = 0;
        total_size += sizeof(uint64_t);
        total_size += item->size();
        return total_size;
    }
};

template<typename XItemType >
uint64_t cm_aggregate_serializable_item_size(const MHO_CommonLabelMap& aMap)
{
    uint64_t total_size = 0;
    std::vector< std::string > keys;
    keys = aMap.DumpKeys< XItemType >();
    total_size += sizeof(uint64_t); //for the number of keys

    //calculate the size of each key and item in the map
    cm_size_calculator<std::string> str_calc;
    cm_size_calculator<XItemType> itm_calc;
    for(std::size_t i=0; i<keys.size(); i++)
    {
        XItemType val;
        aMap.Retrieve(keys[i], val);
        str_calc.item = &(keys[i]);
        total_size += str_calc.get_item_size();

        itm_calc.item = &(val);
        total_size += itm_calc.get_item_size();
    }
    return total_size;
}

template<typename XStream, typename XImportType >
void cm_stream_importer(XStream& s, MHO_CommonLabelMap& aMap)
{
    std::size_t n_elem;
    s >> n_elem;
    for(std::size_t i=0; i<n_elem; i++)
    {
        std::string key;
        XImportType val;
        s >> key;
        s >> val;
        aMap.Insert(key, val);
    }
}

template<typename XStream, typename XExportType>
void cm_stream_exporter(XStream& s, const MHO_CommonLabelMap& aMap)
{
    std::vector< std::string > keys;
    keys = aMap.DumpKeys< XExportType >();
    s << (uint64_t) keys.size();
    for(std::size_t i=0; i<keys.size(); i++)
    {
        XExportType val;
        aMap.Retrieve(keys[i], val);
        s << keys[i];
        s << val;
    }
}

}//end of namespace

#endif /*! end of include guard: MHO_MultiTypeMap_HH__ */
