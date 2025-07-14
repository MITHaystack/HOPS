#ifndef MHO_Constants_HH__
#define MHO_Constants_HH__

#include <cmath>
#include <complex>

namespace hops
{

/*!
 *@file MHO_Constants.hh
 *@class MHO_Constants
 *@date Thu Jun 1 13:05:18 2023 -0400
 *@brief a namespace class for storing constants
 *@author J. Barrett - barrettj@mit.edu
 */

class MHO_Constants
{
    private:
        MHO_Constants() = delete; //class is non-constructable, just a namespace for constants

    public:
        constexpr static double pi = 3.14159265358979323846264338;
        constexpr static double deg_to_rad = pi / 180.0;
        constexpr static double rad_to_deg = 180.0 / pi;
        constexpr static double nanosec_to_second = 1e-9;
        constexpr static std::complex< double > imag_unit{0.0, 1.0};
        constexpr static double MHz_to_Hz = 1e6;
        constexpr static double ion_k = -8.448e9;
};

} // namespace hops

#endif /*! end of include guard: MHO_Constants_HH__ */
