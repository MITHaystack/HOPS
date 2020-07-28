#ifndef HkMeta_HH__
#define HkMeta_HH__


/*
*File: HkMeta.hh
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-13T17:43:26.831Z
*Description: template meta-programming library implementation
*from A. Alexandrescu Modern C++ Design and
*updated with P. Dimov's Simple C++11 metaprogramming to take advantage
*of variadic template parameters
*/

#include <type_traits>

namespace hops
{

//null and empty types
class HkNullType {};
struct HkEmptyType {};

//typelist
template< typename... T > struct HkTypelist {};

//typelist size
template< typename L > struct HkTypelistSizeImpl;
//specialization for typelist
template< class... T > struct HkTypelistSizeImpl< HkTypelist< T... > >
{
    using type = std::integral_constant< size_t, sizeof...(T) >;
};
//alias to HkTypelistSize, retrieve the value itself with ::value (element of std::integral_constant)
template< class L > using HkTypelistSize = typename HkTypelistSizeImpl<L>::type;



}

#endif /* end of include guard: HkMeta */
