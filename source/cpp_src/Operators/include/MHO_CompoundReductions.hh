#ifndef MHO_CompoundReductions_HH__
#define MHO_CompoundReductions_HH__

/*!
 *@file MHO_CompoundReductions.hh
 *@class MHO_CompoundReductions
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Nov 19 16:49:30 2020 -0500
 *@brief
 */

#include <complex>
#include <cstddef>

/**
 * @brief Class MHO_SumIdentity
 */
template< typename XValueType > struct MHO_SumIdentity
{
        /**
         * @brief Copies non-active dimension values into index.
         * 
         * @return XValueType
         * @note This is a static function.
         */
        static XValueType value() { return 0.0; };
};

//specializations for common types
/**
 * @brief Class MHO_SumIdentity<bool>
 */
template<> struct MHO_SumIdentity< bool >
{
        /**
         * @brief Copies value of non-active dimensions into index.
         * 
         * @return bool indicating success/failure
         * @note This is a static function.
         */
        static inline bool value() { return false; };
};

/**
 * @brief Class MHO_SumIdentity<int>
 */
template<> struct MHO_SumIdentity< int >
{
        /**
         * @brief Copies non-active dimension values into index.
         * 
         * @return 0 (int)
         * @note This is a static function.
         */
        static inline int value() { return 0; };
};

/**
 * @brief Class MHO_SumIdentity<short>
 */
template<> struct MHO_SumIdentity< short >
{
        /**
         * @brief Copies value of non-active dimensions into index.
         * 
         * @return Short integer representing copied value.
         * @note This is a static function.
         */
        static inline short value() { return 0; };
};

/**
 * @brief Class MHO_SumIdentity<unsigned int>
 */
template<> struct MHO_SumIdentity< unsigned int >
{
        /**
         * @brief Copies non-active dimension values into index.
         * 
         * @return unsigned int value
         * @note This is a static function.
         */
        static inline unsigned int value() { return 0; };
};

/**
 * @brief Class MHO_SumIdentity<std::size_t>
 */
template<> struct MHO_SumIdentity< std::size_t >
{
        /**
         * @brief Copies non-active dimension values into index.
         * 
         * @return Number of copied dimensions.
         * @note This is a static function.
         */
        static inline std::size_t value() { return 0; };
};

/**
 * @brief Class MHO_SumIdentity<float>
 */
template<> struct MHO_SumIdentity< float >
{
        /**
         * @brief Copies value of non-active dimensions into index.
         * 
         * @return Float value representing copied value.
         * @note This is a static function.
         */
        static inline float value() { return 0.0; };
};

/**
 * @brief Class MHO_SumIdentity<double>
 */
template<> struct MHO_SumIdentity< double >
{
        /**
         * @brief Copies the value of non-active dimensions into index.
         * 
         * @return Returns a double representing the copied value.
         * @note This is a static function.
         */
        static inline double value() { return 0.0; };
};

/**
 * @brief Class MHO_SumIdentity<long double>
 */
template<> struct MHO_SumIdentity< long double >
{
        /**
         * @brief Copies non-active dimension values into index.
         * 
         * @return Returns a long double value
         * @note This is a static function.
         */
        static inline long double value() { return 0.0; };
};

/**
 * @brief Class MHO_SumIdentity<std::complex<float>>
 */
template<> struct MHO_SumIdentity< std::complex< float > >
{
        /**
         * @brief Copies non-active dimension values into index.
         * 
         * @return Float value representing copied value.
         * @note This is a static function.
         */
        static inline std::complex< float > value() { return std::complex< float >(0.0, 0.0); };
};

/**
 * @brief Class MHO_SumIdentity<std::complex<double>>
 */
template<> struct MHO_SumIdentity< std::complex< double > >
{
        /**
         * @brief Copies non-active dimension values into index.
         * 
         * @return Returns double value.
         * @note This is a static function.
         */
        static inline std::complex< double > value() { return std::complex< double >(0.0, 0.0); };
};

/**
 * @brief Class MHO_SumIdentity<std::complex<long double>>
 */
template<> struct MHO_SumIdentity< std::complex< long double > >
{
        /**
         * @brief Copies non-active dimension values into index.
         * 
         * @return Returns 0.0 as a long double
         * @note This is a static function.
         */
        static inline std::complex< long double > value() { return std::complex< long double >(0.0, 0.0); };
};

/**
 * @brief Class MHO_MultiplyIdentity
 */
template< typename XValueType > struct MHO_MultiplyIdentity
{
        /**
         * @brief Copies non-active dimension values into index.
         * 
         * @return XValueType
         * @note This is a static function.
         */
        static XValueType value() { return 0.0; };
};

//specializations for common types
/**
 * @brief Class MHO_MultiplyIdentity<bool>
 */
template<> struct MHO_MultiplyIdentity< bool >
{
        /**
         * @brief Copies the value of non-active dimensions into index.
         * 
         * @return bool indicating success/failure
         * @note This is a static function.
         */
        static inline bool value() { return true; };
};

/**
 * @brief Class MHO_MultiplyIdentity<int>
 */
template<> struct MHO_MultiplyIdentity< int >
{
        /**
         * @brief Copies value of non-active dimensions into index.
         * 
         * @return Returns 0.
         * @note This is a static function.
         */
        static inline int value() { return 1; };
};

/**
 * @brief Class MHO_MultiplyIdentity<short>
 */
template<> struct MHO_MultiplyIdentity< short >
{
        /**
         * @brief Copies value of non-active dimensions into index.
         * 
         * @return Short integer representing the copied value.
         * @note This is a static function.
         */
        static inline short value() { return 1; };
};

/**
 * @brief Class MHO_MultiplyIdentity<unsigned int>
 */
template<> struct MHO_MultiplyIdentity< unsigned int >
{
        /**
         * @brief Copies non-active dimension values into index.
         * 
         * @return unsigned int value
         * @note This is a static function.
         */
        static inline unsigned int value() { return 1; };
};

/**
 * @brief Class MHO_MultiplyIdentity<std::size_t>
 */
template<> struct MHO_MultiplyIdentity< std::size_t >
{
        /**
         * @brief Copies non-active dimension values into index.
         * 
         * @return Number of copied dimensions.
         * @note This is a static function.
         */
        static inline std::size_t value() { return 1; };
};

/**
 * @brief Class MHO_MultiplyIdentity<float>
 */
template<> struct MHO_MultiplyIdentity< float >
{
        /**
         * @brief Copies the value of non-active dimensions into index.
         * 
         * @return Float value representing the copied value.
         * @note This is a static function.
         */
        static inline float value() { return 1.0; };
};

/**
 * @brief Class MHO_MultiplyIdentity<double>
 */
template<> struct MHO_MultiplyIdentity< double >
{
        /**
         * @brief Copies the value of non-active dimensions into index.
         * 
         * @return Returns a double representing the copied value.
         * @note This is a static function.
         */
        static inline double value() { return 1.0; };
};

template<> struct MHO_MultiplyIdentity< long double >
{
        static inline long double value() { return 1.0; };
};

template<> struct MHO_MultiplyIdentity< std::complex< float > >
{
        static inline std::complex< float > value() { return std::complex< float >(1.0, 0.0); };
};

template<> struct MHO_MultiplyIdentity< std::complex< double > >
{
        static inline std::complex< double > value() { return std::complex< double >(1.0, 0.0); };
};

template<> struct MHO_MultiplyIdentity< std::complex< long double > >
{
        static inline std::complex< long double > value() { return std::complex< long double >(1.0, 0.0); };
};

//struct which implements operator as compound summation
template< typename XValueType > struct MHO_CompoundSum
{
        inline void operator()(XValueType& a, const XValueType& b) { a += b; }

        const XValueType identity = MHO_SumIdentity< XValueType >::value();
};

//struct which implements operator as compound multiplication
template< typename XValueType > struct MHO_CompoundMultiply
{
        void operator()(XValueType& a, const XValueType& b) { a *= b; }

        const XValueType identity = MHO_MultiplyIdentity< XValueType >::value();
};

//TODO -- Can we think of any other useful reduction operations to put here?

#endif /*! end of include guard: MHO_CompoundReductions */
