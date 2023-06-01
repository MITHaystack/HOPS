#ifndef MHO_Constants_HH__
#define MHO_Constants_HH__

#include <cmath>
#include <complex>

namespace hops 
{

class MHO_Constants
{
    MHO_Constants() = delete; //class is non-constructable, just a namespace for constants
    
    constexpr static double pi = 3.14159265358979323846264338;
    constexpr static double deg_to_rad = pi/180.0;
    constexpr static double rad_to_deg = 180.0/pi;
    constexpr static std::complex<double> imag_unit{0.0, 1.0};
};

}


#endif /* end of include guard: MHO_Constants_HH__ */
