#include <map>

//implemenation omitted between {}
template< typename XKeyType, typename XValueType >
class MHO_SingleTypeMap
{
    //...impl. omitted...
    private:
        std::map< XKeyType, XValueType > fMap;
};

//declare a multi-type map which takes a key type, and variadic template parameter for the types to be stored
template <typename XKeyType, typename... XValueTypeS>
class MHO_MultiTypeMap;

//declare the specialization for the base case of the recursion (in which the parameter XValueType is just a single type)
template <typename  XKeyType, typename XValueType>
class MHO_MultiTypeMap< XKeyType, XValueType >: public MHO_SingleTypeMap< XKeyType, XValueType > {};

//now set up the recursion
template< typename XKeyType, typename XValueType, typename... XValueTypeS >
class MHO_MultiTypeMap< XKeyType, XValueType, XValueTypeS...>:
        public MHO_MultiTypeMap< XKeyType, XValueType >,
        public MHO_MultiTypeMap< XKeyType, XValueTypeS... > {};
