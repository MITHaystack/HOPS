#ifndef MHO_CompoundReductions_HH__
#define MHO_CompoundReductions_HH__

/*
*File: MHO_CompoundReductions.hh
*Class: MHO_CompoundReductions
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <cstddef>
#include <complex>

template< typename XValueType >
struct MHO_SumIdentity
{
    static constexpr XValueType value = 0;
};

//specializations for common types
template<> struct MHO_SumIdentity<bool>{static constexpr bool value = false;};
template<> struct MHO_SumIdentity<int>{static constexpr int value = 0;};
template<> struct MHO_SumIdentity<short>{static constexpr short value = 0;};
template<> struct MHO_SumIdentity<unsigned int>{static constexpr unsigned int value = 0;};
template<> struct MHO_SumIdentity<std::size_t>{static constexpr std::size_t value = 0;};
template<> struct MHO_SumIdentity<float>{static constexpr float value = 0.0;};
template<> struct MHO_SumIdentity<double>{static constexpr double value = 0.0;};
template<> struct MHO_SumIdentity<long double>{static constexpr long double value = 0.0;};
template<> struct MHO_SumIdentity< std::complex< float > >{static constexpr std::complex< float > value = std::complex<float>(0.0, 0.0);};
template<> struct MHO_SumIdentity< std::complex< double > >{static constexpr std::complex< double > value = std::complex<double>(0.0, 0.0);};
template<> struct MHO_SumIdentity< std::complex< long double > >{static constexpr std::complex< long double > value = std::complex<long double>(0.0, 0.0);};



template< typename XValueType >
struct MHO_MultiplyIdentity
{
    static constexpr XValueType value = 1;
};

//specializations for common types
template<> struct MHO_MultiplyIdentity<bool>{static constexpr bool value = true;};
template<> struct MHO_MultiplyIdentity<int>{static constexpr int value = 1;};
template<> struct MHO_MultiplyIdentity<short>{static constexpr short value = 1;};
template<> struct MHO_MultiplyIdentity<unsigned int>{static constexpr unsigned int value = 1;};
template<> struct MHO_MultiplyIdentity<std::size_t>{static constexpr std::size_t value = 1;};
template<> struct MHO_MultiplyIdentity<float>{static constexpr float value = 1.0;};
template<> struct MHO_MultiplyIdentity<double>{static constexpr double value = 1.0;};
template<> struct MHO_MultiplyIdentity<long double>{static constexpr long double value = 1.0;};
template<> struct MHO_MultiplyIdentity< std::complex< float > >{static constexpr std::complex< float > value = std::complex<float>(1.0, 0.0);};
template<> struct MHO_MultiplyIdentity< std::complex< double > >{static constexpr std::complex< double > value = std::complex<double>(1.0, 0.0);};
template<> struct MHO_MultiplyIdentity< std::complex< long double > >{static constexpr std::complex< long double > value = std::complex<long double>(1.0, 0.0);};


//struct which implements operator as compound summation
template< typename XValueType >
struct MHO_CompoundSum
{
    inline void operator()(XValueType& a, const XValueType& b)
    {
        a += b;
    }

    const XValueType identity = MHO_SumIdentity<XValueType>::value;
};

//struct which implements operator as compound multiplication
template< typename XValueType >
struct MHO_CompoundMultiply
{
    void operator()(XValueType& a, const XValueType& b)
    {
        a *= b;
    }

    const XValueType identity = MHO_MultiplyIdentity<XValueType>::value;
};

//TODO -- Can we think of any other useful reduction operations to put here?


#endif /* end of include guard: MHO_CompoundReductions */
