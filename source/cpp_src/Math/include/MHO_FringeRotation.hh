#ifndef MHO_FringeRotation_HH__
#define MHO_FringeRotation_HH__


#include <complex>
#include <cmath>


/*
*File: MHO_FringeRotation.hh
*Class: MHO_FringeRotation
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: collection of static math functions
*/

namespace hops 
{

class MHO_FringeRotation 
{
    public:
        MHO_FringeRotation(){};
        virtual ~MHO_FringeRotation(){};

        //virtual function (can be pointed to different implementations)
        virtual std::complex<double> vrot(double time_delta, double freq, double ref_freq, double dr, double mbd) const;

    private:

        static const std::complex<double> fImagUnit;

        std::complex<double> vrot_v1(double time_delta, double freq, double ref_freq, double dr, double mbd) const;

};

}

#endif /* end of include guard: MHO_FringeRotation_HH__ */
