#ifndef HMultiTypeMap_HH__
#define HMultiTypeMap_HH__

#include <map>
#include <string>
#include <iostream>

#include "HMeta.hh"

namespace hops
{

template< typename XKeyType, typename XValueType >
class HSingleTypeMap
{
    public:

        HSingleTypeMap(){};
        virtual ~HSingleTypeMap(){};

        void insert(const XKeyType& key, const XValueType& value)
        {
            fMap.insert( std::pair<XKeyType, XValueType>(key,value) );
            std::cout<<"inserting an element: "<<key<<", "<<value<<std::endl;
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

    private:

        std::map< XKeyType, XValueType > fMap;
};

//declare a multi-type map which takes a key type, and variadic template parameter for the types to be stored
template <typename XKeyType, typename... XvalueTypeS>
class HMultiTypeMap;

//declare the base case of the recursion (in which the parameter XValueType is just a single type)
template <typename  XKeyType, typename XValueType>
class HMultiTypeMap< XKeyType, XValueType >: public HSingleTypeMap< XKeyType, XValueType >
{
    public:
        using HSingleTypeMap< XKeyType, XValueType >::insert;
        using HSingleTypeMap< XKeyType, XValueType >::retrieve;
};

//now set up the recursion
template< typename XKeyType, typename XValueType, typename... XValueTypeS >
class HMultiTypeMap< XKeyType, XValueType, XValueTypeS...>: public HMultiTypeMap< XKeyType, XValueType >, HMultiTypeMap< XKeyType, XValueTypeS... >
{
    public:
        using HMultiTypeMap< XKeyType, XValueType >::insert;
        using HMultiTypeMap< XKeyType, XValueTypeS... >::insert;

        using HMultiTypeMap< XKeyType, XValueType >::retrieve;
        using HMultiTypeMap< XKeyType, XValueTypeS... >::retrieve;
};



}

#endif /* end of include guard: HMultiTypeMap_HH__ */
