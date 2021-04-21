#ifndef MHO_MultiTypeMap_HH__
#define MHO_MultiTypeMap_HH__


/*
*File: MHO_MultiTypeMap.hh
*Class: MHO_MultiTypeMap
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:56:55.583Z
*Description:
*/

#include <set>
#include <map>
#include <string>
#include <iostream>
#include <vector>

#include "MHO_Meta.hh"

namespace hops
{

template< typename XKeyType, typename XValueType >
class MHO_SingleTypeMap
{
    public:

        MHO_SingleTypeMap(){};
        virtual ~MHO_SingleTypeMap(){};

        std::size_t Size() const {return fMap.size(); }

        void Insert(const XKeyType& key, const XValueType& value)
        {
            fMap.insert( std::pair<XKeyType, XValueType>(key,value) );
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
        using MHO_SingleTypeMap< XKeyType, XValueType >::Size;
        using MHO_SingleTypeMap< XKeyType, XValueType >::Insert;
        using MHO_SingleTypeMap< XKeyType, XValueType >::Retrieve;
        using MHO_SingleTypeMap< XKeyType, XValueType >::ContainsKey;
        using MHO_SingleTypeMap< XKeyType, XValueType >::DumpKeys;
        using MHO_SingleTypeMap< XKeyType, XValueType >::DumpMap;
        using MHO_SingleTypeMap< XKeyType, XValueType >::CopyFrom;
        using MHO_SingleTypeMap< XKeyType, XValueType >::CopyTo;

        template<typename U = XValueType> typename std::enable_if< std::is_same<U,XValueType >::value, std::size_t >::type
        Size() const
        {
            return static_cast< const MHO_SingleTypeMap< XKeyType, XValueType >* >( this )->Size();
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

};

//now set up the recursion
template< typename XKeyType, typename XValueType, typename... XValueTypeS >
class MHO_MultiTypeMap< XKeyType, XValueType, XValueTypeS...>: public MHO_MultiTypeMap< XKeyType, XValueType >, public MHO_MultiTypeMap< XKeyType, XValueTypeS... >
{
    public:

        using MHO_MultiTypeMap< XKeyType, XValueType >::Size;
        using MHO_MultiTypeMap< XKeyType, XValueTypeS... >::Size;

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

};


}

#endif /* end of include guard: MHO_MultiTypeMap_HH__ */
