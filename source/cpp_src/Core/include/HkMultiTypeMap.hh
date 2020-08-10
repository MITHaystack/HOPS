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
            std::cout<<"inserting an element with key: "<<key<<std::endl;
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

    private:

        std::map< XKeyType, XValueType > fMap;
};

//declare a multi-type map which takes a key type, and variadic template parameter for the types to be stored
template <typename XKeyType, typename... XvalueTypeS>
class HkMultiTypeMap;

//declare the specialization for the base case of the recursion (in which the parameter XValueType is just a single type)
template <typename  XKeyType, typename XValueType>
class HkMultiTypeMap< XKeyType, XValueType >: public HkSingleTypeMap< XKeyType, XValueType >
{
    public:
        using HkSingleTypeMap< XKeyType, XValueType >::insert;
        using HkSingleTypeMap< XKeyType, XValueType >::retrieve;
        using HkSingleTypeMap< XKeyType, XValueType >::dump_map;

        template<typename U = XValueType> typename std::enable_if<std::is_same<U,XValueType>::value>::type
        dump_map()
        {
            static_cast< HkSingleTypeMap< XKeyType, XValueType >* >( this )->dump_map();
        };

};

//now set up the recursion
template< typename XKeyType, typename XValueType, typename... XValueTypeS >
class HkMultiTypeMap< XKeyType, XValueType, XValueTypeS...>: public HkMultiTypeMap< XKeyType, XValueType >, HkMultiTypeMap< XKeyType, XValueTypeS... >
{
    public:

        using HkMultiTypeMap< XKeyType, XValueType >::insert;
        using HkMultiTypeMap< XKeyType, XValueTypeS... >::insert;

        using HkMultiTypeMap< XKeyType, XValueType >::retrieve;
        using HkMultiTypeMap< XKeyType, XValueTypeS... >::retrieve;

        using HkMultiTypeMap< XKeyType, XValueType >::dump_map;
        using HkMultiTypeMap< XKeyType, XValueTypeS... >::dump_map;
};


}

#endif /* end of include guard: HkMultiTypeMap_HH__ */
