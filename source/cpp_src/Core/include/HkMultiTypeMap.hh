#ifndef HkMultiTypeMap_HH__
#define HkMultiTypeMap_HH__


/*
*File: HkMultiTypeMap.hh
*Class: HkMultiTypeMap
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:56:55.583Z
*Description:
*/

#include <map>
#include <string>
#include <iostream>

#include "HkMeta.hh"

namespace hops
{

template< typename XKeyType, typename XValueType >
class HkSingleTypeMap
{
    public:

        HkSingleTypeMap(){};
        virtual ~HkSingleTypeMap(){};

        void insert(const XKeyType& key, const XValueType& value)
        {
            fMap.insert( std::pair<XKeyType, XValueType>(key,value) );
            //std::cout<<"inserting an element with key: "<<key<<std::endl;
        }

        bool retrieve(const XKeyType& key, XValueType& value)
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

        void dump_map()
        {
            typedef typename std::map< XKeyType, XValueType>::iterator iterType;
            for(iterType iter = fMap.begin(); iter != fMap.end(); iter++)
            {
                std::cout<<iter->first<<" : "<<iter->second<<std::endl;
            }
        }

        void CopyFrom(const HkSingleTypeMap<XKeyType, XValueType>& copy_from_obj)
        {
            if(this != &copy_from_obj)
            {
                fMap.clear();
                fMap = copy_from_obj.fMap;
            }
        }

        void CopyTo(HkSingleTypeMap<XKeyType, XValueType>& copy_to_obj) const
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
class HkMultiTypeMap;

//declare the specialization for the base case of the recursion (in which the parameter XValueType is just a single type)
template <typename  XKeyType, typename XValueType>
class HkMultiTypeMap< XKeyType, XValueType >: public HkSingleTypeMap< XKeyType, XValueType >
{
    public:
        using HkSingleTypeMap< XKeyType, XValueType >::insert;
        using HkSingleTypeMap< XKeyType, XValueType >::retrieve;
        using HkSingleTypeMap< XKeyType, XValueType >::dump_map;
        using HkSingleTypeMap< XKeyType, XValueType >::CopyFrom;
        using HkSingleTypeMap< XKeyType, XValueType >::CopyTo;

        template<typename U = XValueType> typename std::enable_if<std::is_same<U,XValueType>::value>::type
        dump_map()
        {
            static_cast< HkSingleTypeMap< XKeyType, XValueType >* >( this )->dump_map();
        };

        template<typename U = XValueType> typename std::enable_if<std::is_same<U,XValueType>::value>::type
        CopyFrom(const HkMultiTypeMap<XKeyType, XValueType>& copy_from_obj)
        {
            static_cast< HkSingleTypeMap< XKeyType, XValueType >* >( this )->
            CopyFrom( *(static_cast< const HkSingleTypeMap< XKeyType, XValueType >* >(&copy_from_obj)) );
        };

        template<typename U = XValueType> typename std::enable_if<std::is_same<U,XValueType>::value>::type
        CopyTo(HkMultiTypeMap<XKeyType, XValueType>& copy_to_obj) const
        {
            static_cast< const HkSingleTypeMap< XKeyType, XValueType >* >( this )->
            CopyTo( *(static_cast< HkSingleTypeMap< XKeyType, XValueType >* >(&copy_to_obj)) );
        };

};

//now set up the recursion
template< typename XKeyType, typename XValueType, typename... XValueTypeS >
class HkMultiTypeMap< XKeyType, XValueType, XValueTypeS...>: public HkMultiTypeMap< XKeyType, XValueType >, public HkMultiTypeMap< XKeyType, XValueTypeS... >
{
    public:

        using HkMultiTypeMap< XKeyType, XValueType >::insert;
        using HkMultiTypeMap< XKeyType, XValueTypeS... >::insert;

        using HkMultiTypeMap< XKeyType, XValueType >::retrieve;
        using HkMultiTypeMap< XKeyType, XValueTypeS... >::retrieve;

        using HkMultiTypeMap< XKeyType, XValueType >::dump_map;
        using HkMultiTypeMap< XKeyType, XValueTypeS... >::dump_map;

        using HkMultiTypeMap< XKeyType, XValueType >::CopyFrom;
        using HkMultiTypeMap< XKeyType, XValueTypeS... >::CopyFrom;

        using HkMultiTypeMap< XKeyType, XValueType >::CopyTo;
        using HkMultiTypeMap< XKeyType, XValueTypeS... >::CopyTo;
};


}

#endif /* end of include guard: HkMultiTypeMap_HH__ */
