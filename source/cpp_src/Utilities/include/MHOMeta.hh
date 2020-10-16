#ifndef MHOMeta_HH__
#define MHOMeta_HH__


/*
*File: MHOMeta.hh
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
class MHONullType {};
struct MHOEmptyType {};

//typelist
template< typename... T > struct MHOTypelist {};

//typelist size
template< typename L > struct MHOTypelistSizeImpl;
//specialization for typelist
template< class... T > struct MHOTypelistSizeImpl< MHOTypelist< T... > >
{
    using type = std::integral_constant< size_t, sizeof...(T) >;
};
//alias to MHOTypelistSize, retrieve the value itself with ::value (element of std::integral_constant)
template< class L > using MHOTypelistSize = typename MHOTypelistSizeImpl<L>::type;



}

#endif /* end of include guard: MHOMeta */
