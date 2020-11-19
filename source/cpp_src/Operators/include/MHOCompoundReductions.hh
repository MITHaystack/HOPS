#ifndef MHOCompoundReductions_HH__
#define MHOCompoundReductions_HH__

/*
*File: MHOCompoundReductions.hh
*Class: MHOCompoundReductions
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description:
*/

#include <cstddef>
#include <complex>

template< typename XValueType >
struct MHOSumIdentity
{
    static constexpr XValueType value = 0;
};

//specializations for common types
template<> struct MHOSumIdentity<bool>{static constexpr bool value = false;};
template<> struct MHOSumIdentity<int>{static constexpr int value = 0;};
template<> struct MHOSumIdentity<short>{static constexpr short value = 0;};
template<> struct MHOSumIdentity<unsigned int>{static constexpr unsigned int value = 0;};
template<> struct MHOSumIdentity<std::size_t>{static constexpr std::size_t value = 0;};
template<> struct MHOSumIdentity<float>{static constexpr float value = 0.0;};
template<> struct MHOSumIdentity<double>{static constexpr double value = 0.0;};
template<> struct MHOSumIdentity<long double>{static constexpr long double value = 0.0;};
template<> struct MHOSumIdentity< std::complex< float > >{static constexpr std::complex< float > value = std::complex<float>(0.0, 0.0);};
template<> struct MHOSumIdentity< std::complex< double > >{static constexpr std::complex< double > value = std::complex<double>(0.0, 0.0);};
template<> struct MHOSumIdentity< std::complex< long double > >{static constexpr std::complex< long double > value = std::complex<long double>(0.0, 0.0);};



template< typename XValueType >
struct MHOMultiplyIdentity
{
    static constexpr XValueType value = 1;
};

//specializations for common types
template<> struct MHOMultiplyIdentity<bool>{static constexpr bool value = true;};
template<> struct MHOMultiplyIdentity<int>{static constexpr int value = 1;};
template<> struct MHOMultiplyIdentity<short>{static constexpr short value = 1;};
template<> struct MHOMultiplyIdentity<unsigned int>{static constexpr unsigned int value = 1;};
template<> struct MHOMultiplyIdentity<std::size_t>{static constexpr std::size_t value = 1;};
template<> struct MHOMultiplyIdentity<float>{static constexpr float value = 1.0;};
template<> struct MHOMultiplyIdentity<double>{static constexpr double value = 1.0;};
template<> struct MHOMultiplyIdentity<long double>{static constexpr long double value = 1.0;};
template<> struct MHOMultiplyIdentity< std::complex< float > >{static constexpr std::complex< float > value = std::complex<float>(1.0, 0.0);};
template<> struct MHOMultiplyIdentity< std::complex< double > >{static constexpr std::complex< double > value = std::complex<double>(1.0, 0.0);};
template<> struct MHOMultiplyIdentity< std::complex< long double > >{static constexpr std::complex< long double > value = std::complex<long double>(1.0, 0.0);};


//struct which implements operator as compound summation
template< typename XValueType >
struct MHOCompoundSum
{
    inline void operator()(XValueType& a, const XValueType& b)
    {
        a += b;
    }

    const XValueType identity = MHOSumIdentity<XValueType>::value;
};

//struct which implements operator as compound multiplication
template< typename XValueType >
struct MHOCompoundMultiply
{
    void operator()(XValueType& a, const XValueType& b)
    {
        a *= b;
    }

    const XValueType identity = MHOMultiplyIdentity<XValueType>::value;
};

//TODO -- Can we think of any other useful reduction operations to put here?


#endif /* end of include guard: MHOCompoundReductions */
