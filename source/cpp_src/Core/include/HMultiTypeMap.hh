#ifndef HMultiTypeMap_HH__
#define HMultiTypeMap_HH__

#include <map>


template< typename XKeyType, typename XValueType >
class KSingleTypeMap
{
    public:

        KSingleTypeMap();
        virtual ~KSingleTypeMap();


        void insert(const XKeyType& key, const XValueType& value)
        {
            fMap.insert( std::pair<XKeyType, XValueType>(key,value) );
        }

    private:

        std::map< XKeyType, XXValueType > fMap;
};


template< typename XKeyType, typename... XValueTypes >
class HMultiTypeMap: public KSingleTypeMap<XKeyType, XValueTypes>...



};




#endif /* end of include guard: HMultiTypeMap_HH__ */
