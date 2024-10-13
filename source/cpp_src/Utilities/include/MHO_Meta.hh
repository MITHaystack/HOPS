#ifndef MHO_Meta_HH__
#define MHO_Meta_HH__

#include <complex>
#include <map>
#include <tuple>
#include <type_traits>

namespace hops
{

/*!
 *@file MHO_Meta.hh
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri Oct 16 11:17:19 2020 -0400
 *@brief template meta-programming helper functions, mostly tuple access/modification
 */

//null and empty types
class MHO_NullType
{};

struct MHO_EmptyType
{};

//typelist
template< typename... T > struct MHO_Typelist
{};

//typelist size
template< typename L > struct MHO_TypelistSizeImpl;

//specialization for typelist
template< class... T > struct MHO_TypelistSizeImpl< MHO_Typelist< T... > >
{
        using type = std::integral_constant< size_t, sizeof...(T) >;
};
//alias to MHO_TypelistSize, retrieve the value itself with ::value (element of std::integral_constant)
template< class L > using MHO_TypelistSize = typename MHO_TypelistSizeImpl< L >::type;

//utility to return 1 if two types are the same, zero otherwise

template< class T, class U > struct is_same_count
{
        constexpr static size_t value = 0;
};

template< class T > struct is_same_count< T, T >
{
        constexpr static size_t value = 1;
};

//utility to count the instances of a particular type in a parameter pack //////
template< typename XCheckType, size_t N, typename... T > struct count_instances_of_type;

//terminating case is N=0
template< typename XCheckType, typename... T > struct count_instances_of_type< XCheckType, 0, T... >
{
        using current_type = typename std::tuple_element< 0, std::tuple< T... > >::type;
        constexpr static std::size_t value = is_same_count< XCheckType, current_type >::value;
};

//N = sizeof...(T) - 1
template< typename XCheckType, size_t N, typename... T > struct count_instances_of_type
{
        using current_type = typename std::tuple_element< N, std::tuple< T... > >::type;
        constexpr static std::size_t value =
            is_same_count< XCheckType, current_type >::value + count_instances_of_type< XCheckType, N - 1, T... >::value;
};

////////////////////////////////////////////////////////////////////////////////

//functions needed to stream tuples/////////////////////////////////////////////
template< size_t N = 0, typename XStream, typename... T >
typename std::enable_if< (N >= sizeof...(T)), XStream& >::type ostream_tuple(XStream& s, const std::tuple< T... >&)
{
    //terminating case, do nothing but return s
    return s;
}

template< size_t N = 0, typename XStream, typename... T >
typename std::enable_if< (N < sizeof...(T)), XStream& >::type ostream_tuple(XStream& s, const std::tuple< T... >& t)
{
    //dump the element @ N
    s << std::get< N >(t);
    //recurse to next
    return ostream_tuple< N + 1 >(s, t);
}

//functions needed to stream tuples
template< size_t N = 0, typename XStream, typename... T >
typename std::enable_if< (N >= sizeof...(T)), XStream& >::type istream_tuple(XStream& s, std::tuple< T... >&)
{
    //terminating case, do nothing but return s
    return s;
}

template< size_t N = 0, typename XStream, typename... T >
typename std::enable_if< (N < sizeof...(T)), XStream& >::type istream_tuple(XStream& s, std::tuple< T... >& t)
{
    //in stream the element @ N
    s >> std::get< N >(t);
    //recurse to next
    return istream_tuple< N + 1 >(s, t);
}

////////////////////////////////////////////////////////////////////////////////

//generic apply functor (which takes and index value!) to all elements of a tuple
template< size_t NTypes > struct indexed_tuple_visit
{
        template< typename XTupleType, typename XFunctorType > static void visit(XTupleType& tup, XFunctorType& functor)
        {
            //apply here and then recurse to the next type
            functor(NTypes - 1, std::get< NTypes - 1 >(tup));
            indexed_tuple_visit< NTypes - 1 >::visit(tup, functor);
        }
};

//base case, terminates the recursion
template<> struct indexed_tuple_visit< 0 >
{
        template< typename XTupleType, typename XFunctorType > static void visit(XTupleType& tup, XFunctorType& functor) {}
};

////////////////////////////////////////////////////////////////////////////////

//generic apply functor to tuple element (for runtime-indexed access)
template< size_t NTypes > struct apply_to_tuple
{
        template< typename XTupleType, typename XFunctorType >
        static void apply(XTupleType& tup, size_t index, XFunctorType& functor)
        {
            //if the index matches the current element, apply the functor
            if(index == NTypes - 1)
            {
                functor(std::get< NTypes - 1 >(tup));
            }
            else
            {
                //else recurse to the next type
                apply_to_tuple< NTypes - 1 >::apply(tup, index, functor);
            }
        }
};

//base case, empty tuple with no elements (should never happen)
template<> struct apply_to_tuple< 0 >
{
        template< typename XTupleType, typename XFunctorType >
        static void apply(XTupleType& tup, size_t index, XFunctorType& functor)
        {}
};

//const access
template< typename XTupleType, typename XFunctorType > void apply_at(const XTupleType& tup, size_t index, XFunctorType& functor)
{
    apply_to_tuple< std::tuple_size< XTupleType >::value >::apply(tup, index, functor);
}

//non-const access
template< typename XTupleType, typename XFunctorType > void apply_at(XTupleType& tup, size_t index, XFunctorType& functor)
{
    apply_to_tuple< std::tuple_size< XTupleType >::value >::apply(tup, index, functor);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//same thing as above, but apply a two argument functor to two tuples of the same type

//generic apply functor to tuple (for runtime-indexed access)
//XTupleType and XTupleType2 must be the same (except for const)
template< size_t NTypes > struct apply_to_tuple2
{
        template< typename XTupleType, typename XTupleType2, typename XFunctorType >
        static void apply(XTupleType& tup1, XTupleType2& tup2, size_t index, XFunctorType& functor)
        {
            //if the index matches the current element, apply the functor
            if(index == NTypes - 1)
            {
                functor(std::get< NTypes - 1 >(tup1), std::get< NTypes - 1 >(tup2));
            }
            else
            {
                //else recurse to the next type
                apply_to_tuple2< NTypes - 1 >::apply(tup1, tup2, index, functor);
            }
        }
};

//base case, empty tuple with no elements (should never happen)
template<> struct apply_to_tuple2< 0 >
{
        template< typename XTupleType, typename XTupleType2, typename XFunctorType >
        static void apply(XTupleType& tup1, XTupleType2& tup2, size_t index, XFunctorType& functor)
        {}
};

//const access on first parameter
template< typename XTupleType, typename XTupleType2, typename XFunctorType >
void apply_at2(const XTupleType& tup1, XTupleType& tup2, size_t index, XFunctorType& functor)
{
    apply_to_tuple2< std::tuple_size< XTupleType >::value >::apply(tup1, tup2, index, functor);
}

//non-const access
template< typename XTupleType, typename XTupleType2, typename XFunctorType >
void apply_at2(XTupleType& tup1, XTupleType2& tup2, size_t index, XFunctorType& functor)
{
    apply_to_tuple2< std::tuple_size< XTupleType >::value >::apply(tup1, tup2, index, functor);
}

////////////////////////////////////////////////////////////////////////////////
//check structs for complex floating point types

template< typename XValueType > struct is_complex: std::false_type
{};

template<> struct is_complex< std::complex< float > >: std::true_type
{};

template<> struct is_complex< std::complex< double > >: std::true_type
{};

template<> struct is_complex< std::complex< long double > >: std::true_type
{};

////////////////////////////////////////////////////////////////////////////////
//zip elements of two iterable (probably STL) containers (which define a value_type)
//into a map which takes the i-th element of the 1st container to the i-th element
//of the 2nd container. Terminates at the end of whatever container stops first.

template< typename XContainer1, typename XContainer2 >
std::map< typename XContainer1::value_type, typename XContainer2::value_type > zip_into_map(const XContainer1& c1,
                                                                                            const XContainer2& c2)
{
    auto it1 = c1.begin();
    auto it2 = c2.begin();
    std::map< typename XContainer1::value_type, typename XContainer2::value_type > zip;
    while(it1 != c1.end() && it2 != c2.end())
    {
        zip[*it1] = *it2;
        ++it1;
        ++it2;
    }
    return zip;
}

////////////////////////////////////////////////////////////////////////////////
//empty classes for detecting classes derived from container types
//only needed for dependent template specializations
class MHO_AxisBase
{};

class MHO_TableContainerBase
{};

class MHO_VectorContainerBase
{}; //only needed for dependent template specializations

class MHO_ScalarContainerBase
{}; //only needed for dependent template specializations

////////////////////////////////////////////////////////////////////////////////

} // namespace hops

#endif /*! end of include guard: MHO_Meta */
