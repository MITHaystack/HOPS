#ifndef MHOMultiTypeMap_HH__
#define MHOMultiTypeMap_HH__


/*
*File: MHOMultiTypeMap.hh
*Class: MHOMultiTypeMap
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-15T20:56:55.583Z
*Description:
*/

#include <map>
#include <string>
#include <iostream>

#include "MHOMeta.hh"

namespace hops
{

template< typename XKeyType, typename XValueType >
class MHOSingleTypeMap
{
    public:

        MHOSingleTypeMap(){};
        virtual ~MHOSingleTypeMap(){};

        void Insert(const XKeyType& key, const XValueType& value)
        {
            fMap.insert( std::pair<XKeyType, XValueType>(key,value) );
            //std::cout<<"inserting an element with key: "<<key<<std::endl;
        }

        bool Retrieve(const XKeyType& key, XValueType& value)
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

        void DumpMap()
        {
            typedef typename std::map< XKeyType, XValueType>::iterator iterType;
            for(iterType iter = fMap.begin(); iter != fMap.end(); iter++)
            {
                std::cout<<iter->first<<" : "<<iter->second<<std::endl;
            }
        }

        void CopyFrom(const MHOSingleTypeMap<XKeyType, XValueType>& copy_from_obj)
        {
            if(this != &copy_from_obj)
            {
                fMap.clear();
                fMap = copy_from_obj.fMap;
            }
        }

        void CopyTo(MHOSingleTypeMap<XKeyType, XValueType>& copy_to_obj) const
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
class MHOMultiTypeMap;

//declare the specialization for the base case of the recursion (in which the parameter XValueType is just a single type)
template <typename  XKeyType, typename XValueType>
class MHOMultiTypeMap< XKeyType, XValueType >: public MHOSingleTypeMap< XKeyType, XValueType >
{
    public:
        using MHOSingleTypeMap< XKeyType, XValueType >::Insert;
        using MHOSingleTypeMap< XKeyType, XValueType >::Retrieve;
        using MHOSingleTypeMap< XKeyType, XValueType >::DumpMap;
        using MHOSingleTypeMap< XKeyType, XValueType >::CopyFrom;
        using MHOSingleTypeMap< XKeyType, XValueType >::CopyTo;

        template<typename U = XValueType> typename std::enable_if<std::is_same<U,XValueType>::value>::type
        DumpMap()
        {
            static_cast< MHOSingleTypeMap< XKeyType, XValueType >* >( this )->DumpMap();
        };

        template<typename U = XValueType> typename std::enable_if<std::is_same<U,XValueType>::value>::type
        CopyFrom(const MHOMultiTypeMap<XKeyType, XValueType>& copy_from_obj)
        {
            static_cast< MHOSingleTypeMap< XKeyType, XValueType >* >( this )->
            CopyFrom( *(static_cast< const MHOSingleTypeMap< XKeyType, XValueType >* >(&copy_from_obj)) );
        };

        template<typename U = XValueType> typename std::enable_if<std::is_same<U,XValueType>::value>::type
        CopyTo(MHOMultiTypeMap<XKeyType, XValueType>& copy_to_obj) const
        {
            static_cast< const MHOSingleTypeMap< XKeyType, XValueType >* >( this )->
            CopyTo( *(static_cast< MHOSingleTypeMap< XKeyType, XValueType >* >(&copy_to_obj)) );
        };

};

//now set up the recursion
template< typename XKeyType, typename XValueType, typename... XValueTypeS >
class MHOMultiTypeMap< XKeyType, XValueType, XValueTypeS...>: public MHOMultiTypeMap< XKeyType, XValueType >, public MHOMultiTypeMap< XKeyType, XValueTypeS... >
{
    public:

        using MHOMultiTypeMap< XKeyType, XValueType >::Insert;
        using MHOMultiTypeMap< XKeyType, XValueTypeS... >::Insert;

        using MHOMultiTypeMap< XKeyType, XValueType >::Retrieve;
        using MHOMultiTypeMap< XKeyType, XValueTypeS... >::Retrieve;

        using MHOMultiTypeMap< XKeyType, XValueType >::DumpMap;
        using MHOMultiTypeMap< XKeyType, XValueTypeS... >::DumpMap;

        using MHOMultiTypeMap< XKeyType, XValueType >::CopyFrom;
        using MHOMultiTypeMap< XKeyType, XValueTypeS... >::CopyFrom;

        using MHOMultiTypeMap< XKeyType, XValueType >::CopyTo;
        using MHOMultiTypeMap< XKeyType, XValueTypeS... >::CopyTo;
};


}

#endif /* end of include guard: MHOMultiTypeMap_HH__ */
