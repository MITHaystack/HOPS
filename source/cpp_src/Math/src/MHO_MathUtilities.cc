#include "MHO_MathUtilities.hh"
#include "MHO_Constants.hh"

namespace hops
{

double
MHO_MathUtilities::dwin(double value, double lower, double upper)
{
    if (value < lower) return (lower);
    else if (value > upper) return (upper);
    else return (value);
}

int
MHO_MathUtilities::parabola (double y[3], double lower, double upper, double* x_max, double* amp_max, double q[3])
{
    int i, rc;
    double x, range;
    //extern double dwin(double, double, double);
    range = std::fabs (upper - lower);

    q[0] = (y[0] - 2 * y[1] + y[2]) / 2;      /* This is trivial to derive,
                                    or see rjc's 94.1.10 derivation */
    q[1] = (y[2] - y[0]) / 2;
    q[2] = y[1];


    if (q[0] < 0.0)
        x = -q[1] / (2 * q[0]);                      /* x value at maximum y */
    else                                         /* no max, pick higher side */
        x = (y[2] > y[0]) ? 1.0 : -1.0;

    *x_max = dwin (x, lower, upper);

    *amp_max = q[0] * *x_max * *x_max  +  q[1] * *x_max  +  q[2];

    // Test for error conditions

    rc = 0;                         // default: indicates error-free interpolation
    if (q[0] >= 0)                  // 0 or positive curvature is an interpolation error
        rc = 2;
                                    // Is maximum at either edge?
                                    // (simple floating point equality test can fail
                                    // in machine-dependent way)
    else if (std::fabs (*x_max - x) > (0.001 * range))
        rc = 1;

    return (rc);
}


double
MHO_MathUtilities::average(std::vector<double>& vec)
{
    std::size_t s = vec.size();
    if(s == 0){return 0.0;}
    double ave = 0;
    for(std::size_t i=0; i<s; i++){ave += vec[i];}
    ave /= (double) s;
    return ave;
}


double
MHO_MathUtilities::angular_average(std::vector<double>& vec)
{
    std::size_t s = vec.size();
    if(s == 0){return 0.0;}
    std::complex<double> ave = 0;
    std::complex<double> imagUnit = MHO_Constants::imag_unit;
    for(std::size_t i=0; i<s; i++)
    {
        std::complex<double> phasor = std::exp(1.0*imagUnit*(vec[i]));
        ave += phasor;
    }
    ave /= (double) s;
    return std::arg(ave);
}


}//end namespace
