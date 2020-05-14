#ifndef HMeta_HH__
#define HMeta_HH__


/*
*File: HMeta.hh
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
class HNullType {};
struct HEmptyType {};

//typelist
template< typename... T > struct HTypelist {};

//typelist size
template< typename L > struct HTypelistSizeImpl;
//specialization for typelist
template< class... T > struct HTypelistSizeImpl< HTypelist< T... > >
{
    using type = std::integral_constant< size_t, sizeof...(T) >;
};
//alias to HTypelistSize, retrieve the value itself with ::value
template< class L > using HTypelistSize = typename HTypelistSizeImpl<L>::type;



}

#endif /* end of include guard: HMeta */
