#ifndef HMultiTypeMap_HH__
#define HMultiTypeMap_HH__

#include <map>
#include <string>
#include <iostream>

#include "HMeta.hh"

namespace hops
{

template< typename XValueType >
class HSingleTypeMap
{
    public:

        HSingleTypeMap(){};
        virtual ~HSingleTypeMap(){};

        void insert(const std::string& key, const XValueType& value)
        {
            fMap.insert( std::pair< std::string, XValueType>(key,value) );
            std::cout<<"inserting an element: "<<key<<", "<<value<<std::endl;
        }

    private:

        std::map< std::string, XValueType > fMap;
};

//declare a multi-type map which takes a variadic template parameter
template <typename... XvalueTypeS>
class HMultiTypeMap;

//declare the base case of the recursion (which the parameter is a single type)
template <typename XValueType>
class HMultiTypeMap< XValueType >: public HSingleTypeMap< XValueType >
{
    public:
        using HSingleTypeMap< XValueType >::insert;
};

//now set up the recursion 
template< typename XValueType, typename... XvalueTypeS >
class HMultiTypeMap< XValueType, XvalueTypeS...>: public HMultiTypeMap< XValueType >, HMultiTypeMap< XvalueTypeS... >
{
    public:
        using HMultiTypeMap< XValueType >::insert;
        using HMultiTypeMap< XvalueTypeS... >::insert;
};



}

#endif /* end of include guard: HMultiTypeMap_HH__ */
