#ifndef MHO_MathUtilities_HH__
#define MHO_MathUtilities_HH__

/*!
*@file MHO_MathUtilities.hh
*@class MHO_MathUtilities
*@author J. Barrett - barrettj@mit.edu
*@date
*@brief implements a variety of simple math functions copied from original c code with minimal changes
*/


#include <complex>
#include <cmath>
#include <vector>


namespace hops
{

class MHO_MathUtilities
{
    public:
        MHO_MathUtilities(){};
        virtual ~MHO_MathUtilities(){};

        static double dwin(double value, double lower, double upper);
        static int parabola (double y[3], double lower, double upper, double* x_max, double* amp_max, double q[3]);

        //returns the average of the values in a vector
        static double average(std::vector<double>& vec);

        //returns the average of the values in a vector assuming they are angles (radians)
        static double angular_average(std::vector<double>& vec);

};

}

#endif /*! end of include guard: MHO_MathUtilities_HH__ */
