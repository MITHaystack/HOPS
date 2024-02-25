#ifndef MHO_CompoundReductions_HH__
#define MHO_CompoundReductions_HH__

/*!
*@file MHO_CompoundReductions.hh
*@class MHO_CompoundReductions
*@author J. Barrett - barrettj@mit.edu
*@date Thu Nov 19 16:49:30 2020 -0500
*@brief
*/

#include <cstddef>
#include <complex>

template< typename XValueType >
struct MHO_SumIdentity
{
    static XValueType value() {return 0.0;};
};

//specializations for common types
template<> struct MHO_SumIdentity<bool>{static inline bool value(){ return false;};};
template<> struct MHO_SumIdentity<int>{static inline int value(){ return 0;};};
template<> struct MHO_SumIdentity<short>{static inline short value(){ return 0;};};
template<> struct MHO_SumIdentity<unsigned int>{static inline unsigned int value(){ return 0;};};
template<> struct MHO_SumIdentity<std::size_t>{static inline std::size_t value(){ return 0;};};
template<> struct MHO_SumIdentity<float>{static inline float value(){ return 0.0;};};
template<> struct MHO_SumIdentity<double>{static inline double value(){ return 0.0;};};
template<> struct MHO_SumIdentity<long double>{static inline long double value(){ return 0.0;};};
template<> struct MHO_SumIdentity< std::complex< float > >{static inline std::complex< float > value(){ return std::complex<float>(0.0, 0.0);};};
template<> struct MHO_SumIdentity< std::complex< double > >{static inline std::complex< double > value(){ return std::complex<double>(0.0, 0.0);};};
template<> struct MHO_SumIdentity< std::complex< long double > >{static inline std::complex< long double > value(){ return std::complex<long double>(0.0, 0.0);};};



template< typename XValueType >
struct MHO_MultiplyIdentity
{
    static XValueType value() {return 0.0;};
};

//specializations for common types
template<> struct MHO_MultiplyIdentity<bool>{static inline bool value(){ return true;};};
template<> struct MHO_MultiplyIdentity<int>{static inline int value(){ return 1;};};
template<> struct MHO_MultiplyIdentity<short>{static inline short value(){ return 1;};};
template<> struct MHO_MultiplyIdentity<unsigned int>{static inline unsigned int value(){ return 1;};};
template<> struct MHO_MultiplyIdentity<std::size_t>{static inline std::size_t value(){ return 1;};};
template<> struct MHO_MultiplyIdentity<float>{static inline float value(){ return 1.0;};};
template<> struct MHO_MultiplyIdentity<double>{static inline double value(){ return 1.0;};};
template<> struct MHO_MultiplyIdentity<long double>{static inline long double value(){ return 1.0;};};
template<> struct MHO_MultiplyIdentity< std::complex< float > >{static inline std::complex< float > value(){ return std::complex<float>(1.0, 0.0);};};
template<> struct MHO_MultiplyIdentity< std::complex< double > >{static inline std::complex< double > value(){ return std::complex<double>(1.0, 0.0);};};
template<> struct MHO_MultiplyIdentity< std::complex< long double > >{static inline std::complex< long double > value(){ return std::complex<long double>(1.0, 0.0);};};


//struct which implements operator as compound summation
template< typename XValueType >
struct MHO_CompoundSum
{
    inline void operator()(XValueType& a, const XValueType& b)
    {
        a += b;
    }

    const XValueType identity = MHO_SumIdentity<XValueType>::value();
};

//struct which implements operator as compound multiplication
template< typename XValueType >
struct MHO_CompoundMultiply
{
    void operator()(XValueType& a, const XValueType& b)
    {
        a *= b;
    }

    const XValueType identity = MHO_MultiplyIdentity<XValueType>::value();
};

//TODO -- Can we think of any other useful reduction operations to put here?


#endif /*! end of include guard: MHO_CompoundReductions */
