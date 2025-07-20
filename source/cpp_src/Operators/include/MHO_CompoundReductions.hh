#ifndef MHO_CompoundReductions_HH__
#define MHO_CompoundReductions_HH__

/*!
 *@file MHO_CompoundReductions.hh
 *@class MHO_CompoundReductions
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Nov 19 16:49:30 2020 -0500
 *@brief Collection of functors for compound reductions across an array 
 * (e.g. summation or multiplication of each element along one dimension)
 */

#include <complex>
#include <cstddef>

namespace hops 
{

/**
 * @brief Class MHO_SumIdentity
 */
template< typename XValueType > struct MHO_SumIdentity
{
        /**
         * @brief Returns the identity for the sum operation
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
         * @brief Returns the identity for the sum operation
         * 
         * @return returns false
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
         * @brief Returns the identity for the sum operation
         * 
         * @return Returns 0
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
         * @brief Returns the identity for the sum operation
         * 
         * @return returns 0
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
         * @brief Returns the identity for the sum operation
         * 
         * @return returns 0
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
         * @brief Returns the identity for the sum operation
         * 
         * @return returns 0
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
         * @brief Returns the identity for the sum operation
         * 
         * @return returns 0.0
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
         * @brief Returns the identity for the sum operation
         * 
         * @return Returns 0.0
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
         * @brief Returns the identity for the sum operation
         * 
         * @return Returns 0.0
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
         * @return Returns 0.0 as a std::complex<float>
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
         * @return Returns 0.0 as a std::complex<double>
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
         * @brief Returns the identity for the multiplication operation
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
         * @brief Returns the identity for the multiplication operation
         * 
         * @return Returns true
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
         * @brief Returns the identity for the multiplication operation
         * 
         * @return Returns 1
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
         * @brief Returns the identity for the multiplication operation
         * 
         * @return Returns 1
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
         * @brief Returns the identity for the multiplication operation
         * 
         * @return Returns 1
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
         * @brief Returns the identity for the multiplication operation
         * 
         * @return Returns 1
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
         * @brief Returns the identity for the multiplication operation
         * 
         * @return Returns 1.0
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
         * @brief Returns the identity for the multiplication operation
         * 
         * @return Returns 1.0
         * @note This is a static function.
         */
        static inline double value() { return 1.0; };
};

template<> struct MHO_MultiplyIdentity< long double >
{
        /**
        * @brief Returns the identity for the multiplication operation
        * 
        * @return Returns 1.0
        * @note This is a static function.
        */
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


}//end of namespace 


#endif /*! end of include guard: MHO_CompoundReductions */
