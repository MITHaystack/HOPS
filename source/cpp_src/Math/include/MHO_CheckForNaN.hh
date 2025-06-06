#ifndef MHO_CheckForNaN_HH__
#define MHO_CheckForNaN_HH__

/*!
 *@file MHO_CheckForNaN.hh
 *@class MHO_CheckForNaN
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Aug 12 11:16:36 2021 -0400
 *@brief
 */

#include <cmath>
#include <complex>

template< typename XNumericalType > class MHO_CheckForNaN
{
    public:
        MHO_CheckForNaN();
        virtual ~MHO_CheckForNaN();

        //note this primitive != comparison will not detect "INF" (only NAN)
        static bool isnan(const XNumericalType& value) { return value != value; }

    private:
};

//overloads for numerical types

template<> inline bool MHO_CheckForNaN< float >::isnan(const float& value)
{
    return std::isnan(value);
}

template<> inline bool MHO_CheckForNaN< double >::isnan(const double& value)
{
    return std::isnan(value);
}

template<> inline bool MHO_CheckForNaN< long double >::isnan(const long double& value)
{
    return std::isnan(value);
}

template<> inline bool MHO_CheckForNaN< std::complex< float > >::isnan(const std::complex< float >& value)
{
    return std::isnan(value.real()) || std::isnan(value.imag());
}

template<> inline bool MHO_CheckForNaN< std::complex< double > >::isnan(const std::complex< double >& value)
{
    return std::isnan(value.real()) || std::isnan(value.imag());
}

template<> inline bool MHO_CheckForNaN< std::complex< long double > >::isnan(const std::complex< long double >& value)
{
    return std::isnan(value.real()) || std::isnan(value.imag());
}

#endif /*! end of include guard: MHO_CheckForNaN */
