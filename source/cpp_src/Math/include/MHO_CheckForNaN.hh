#ifndef MHO_CheckForNaN_HH__
#define MHO_CheckForNaN_HH__

/*!
 *@file MHO_CheckForNaN.hh
 *@class MHO_CheckForNaN
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Aug 12 11:16:36 2021 -0400
 *@brief checks if a value is NaN for various numerical types
 */

#include <cmath>
#include <complex>

/**
 * @brief Class MHO_CheckForNaN
 */
template< typename XNumericalType > class MHO_CheckForNaN
{
    public:
        MHO_CheckForNaN();
        virtual ~MHO_CheckForNaN();


        /**
         * @brief Checks if a numerical value is NaN by comparing it to itself.
         * note that this primitive (X != X) comparison will not detect "INF" (only NAN)
         * @param value Input value of type XNumericalType& to check for NaN.
         * @return True if value is NaN, false otherwise.
         * @note This is a static function.
         */
        static bool isnan(const XNumericalType& value) { return value != value; }

    private:
};

//overloads for numerical types

/**
 * @brief Checks if a float value is NaN using std::isnan.
 * 
 * @param value Input float value to check for NaN.
 * @return Boolean indicating whether the input value is NaN.
 */
template<> inline bool MHO_CheckForNaN< float >::isnan(const float& value)
{
    return std::isnan(value);
}

/**
 * @brief Checks if a given double value is NaN.
 * 
 * @param value Input double value to check for NaN.
 * @return True if value is NaN, false otherwise.
 */
template<> inline bool MHO_CheckForNaN< double >::isnan(const double& value)
{
    return std::isnan(value);
}

/**
 * @brief Checks if a given value is Not-a-Number (NaN) using standard library's isnan().
 * 
 * @param value Input long double value to check for NaN
 * @return Boolean indicating whether the input value is NaN or not
 */
template<> inline bool MHO_CheckForNaN< long double >::isnan(const long double& value)
{
    return std::isnan(value);
}

/**
 * @brief Checks if a complex float value is NaN by examining its real and imaginary parts.
 * 
 * @param value Input complex float value to check for NaN.
 * @return True if either real or imaginary part is NaN, false otherwise.
 */
template<> inline bool MHO_CheckForNaN< std::complex< float > >::isnan(const std::complex< float >& value)
{
    return std::isnan(value.real()) || std::isnan(value.imag());
}

/**
 * @brief Checks if a complex double value is NaN by examining its real and imaginary parts.
 * 
 * @param value Input complex double value to check for NaN.
 * @return True if either real or imaginary part is NaN, false otherwise.
 */
template<> inline bool MHO_CheckForNaN< std::complex< double > >::isnan(const std::complex< double >& value)
{
    return std::isnan(value.real()) || std::isnan(value.imag());
}

/**
 * @brief Checks if a complex long double value is NaN by examining its real and imaginary parts.
 * 
 * @param value Input complex number to check for NaN
 * @return True if either real or imaginary part is NaN, false otherwise
 */
template<> inline bool MHO_CheckForNaN< std::complex< long double > >::isnan(const std::complex< long double >& value)
{
    return std::isnan(value.real()) || std::isnan(value.imag());
}

#endif /*! end of include guard: MHO_CheckForNaN */
