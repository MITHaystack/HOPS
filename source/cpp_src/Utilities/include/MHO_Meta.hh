#ifndef MHO_Meta_HH__
#define MHO_Meta_HH__


/*
*File: MHO_Meta.hh
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: 2020-05-13T17:43:26.831Z
*Description: template meta-programming library implementation
*from A. Alexandrescu Modern C++ Design and
*updated with P. Dimov's Simple C++11 metaprogramming to take advantage
*of variadic template parameters
*/

#include <type_traits>
#include <tuple>

namespace hops
{

//null and empty types
class MHO_NullType {};
struct MHO_EmptyType {};

//typelist
template< typename... T > struct MHO_Typelist {};

//typelist size
template< typename L > struct MHO_TypelistSizeImpl;
//specialization for typelist
template< class... T > struct MHO_TypelistSizeImpl< MHO_Typelist< T... > >
{
    using type = std::integral_constant< size_t, sizeof...(T) >;
};
//alias to MHO_TypelistSize, retrieve the value itself with ::value (element of std::integral_constant)
template< class L > using MHO_TypelistSize = typename MHO_TypelistSizeImpl<L>::type;



//functions needed to stream tuples
template<size_t N = 0, typename XStream, typename... T >
typename std::enable_if< (N >= sizeof...(T)), XStream& >::type
    ostream_tuple(XStream& s, const std::tuple<T...>&)
{
    //terminating case, do nothing but return s
    return s;
}

template <size_t N = 0, typename XStream, typename... T>
typename std::enable_if< (N < sizeof...(T)), XStream& >::type
    ostream_tuple(XStream& s, const std::tuple<T...>& t)
{
    //dump the element @ N
    s << std::get<N>(t);
    //recurse to next
    return ostream_tuple<N+1>(s, t);
}

//functions needed to stream tuples
template<size_t N = 0, typename XStream, typename... T >
typename std::enable_if< (N >= sizeof...(T)), XStream& >::type
    istream_tuple(XStream& s, std::tuple<T...>&)
{
    //terminating case, do nothing but return s
    return s;
}

template <size_t N = 0, typename XStream, typename... T>
typename std::enable_if< (N < sizeof...(T)), XStream& >::type
    istream_tuple(XStream& s, std::tuple<T...>& t)
{
    //in stream the element @ N
    s >> std::get<N>(t);
    //recurse to next
    return istream_tuple<N+1>(s, t);
}



}

#endif /* end of include guard: MHO_Meta */
