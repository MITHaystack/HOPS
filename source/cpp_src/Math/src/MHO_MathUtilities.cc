#include "MHO_MathUtilities.hh"
#include "MHO_Constants.hh"

namespace hops
{

double MHO_MathUtilities::dwin(double value, double lower, double upper)
{
    if(value < lower)
        return (lower);
    else if(value > upper)
        return (upper);
    else
        return (value);
}

int MHO_MathUtilities::parabola(double y[3], double lower, double upper, double* x_max, double* amp_max, double q[3])
{
    int i, rc;
    double x, range;
    //extern double dwin(double, double, double);
    range = std::fabs(upper - lower);

    q[0] = (y[0] - 2 * y[1] + y[2]) / 2; /* This is trivial to derive,
                               or see rjc's 94.1.10 derivation */
    q[1] = (y[2] - y[0]) / 2;
    q[2] = y[1];

    if(q[0] < 0.0)
        x = -q[1] / (2 * q[0]); /* x value at maximum y */
    else                        /* no max, pick higher side */
        x = (y[2] > y[0]) ? 1.0 : -1.0;

    *x_max = dwin(x, lower, upper);

    *amp_max = q[0] * *x_max * *x_max + q[1] * *x_max + q[2];

    // Test for error conditions

    rc = 0;       // default: indicates error-free interpolation
    if(q[0] >= 0) // 0 or positive curvature is an interpolation error
        rc = 2;
    // Is maximum at either edge?
    // (simple floating point equality test can fail
    // in machine-dependent way)
    else if(std::fabs(*x_max - x) > (0.001 * range))
        rc = 1;

    return (rc);
}

int MHO_MathUtilities::minvert3(double a[3][3], double ainv[3][3])
{
    // minvert () is passed an n x n matrix a
    // and returns its inverse in ainv
    // rc is non-zero if matrix a is singular
    // initial code (adapted from internet version)  2019.9.6  rjc

    int i, j, k, rc = 0;

    double x[3][2 * 3], ratio, c;

    for(i = 0; i < 3; i++) // copy a xrix into work area
        for(j = 0; j < 2 * 3; j++)
            if(j < 3)
                x[i][j] = a[i][j];
            else
                x[i][j] = 0;

    for(i = 0; i < 3; i++)
        for(j = 3; j < 2 * 3; j++)
            if(i == (j - 3))
                x[i][j] = 1.0;
            else
                x[i][j] = 0.0;

    for(i = 0; i < 3; i++)
        for(j = 0; j < 3; j++)
            if(i != j)
            {
                ratio = x[j][i] / x[i][i];
                for(k = 0; k < 2 * 3; k++)
                    x[j][k] -= ratio * x[i][k];
            }

    for(i = 0; i < 3; i++)
    {
        c = x[i][i];
        for(j = 3; j < 2 * 3; j++)
        {
            x[i][j] /= c;
            rc |= std::isnan(x[i][j]);
        }
    }

    for(i = 0; i < 3; i++) // copy inverse into ainv
        for(j = 0; j < 3; j++)
            ainv[i][j] = x[i][j + 3];

    return rc;
}

double MHO_MathUtilities::average(std::vector< double >& vec)
{
    std::size_t s = vec.size();
    if(s == 0)
    {
        return 0.0;
    }
    double ave = 0;
    for(std::size_t i = 0; i < s; i++)
    {
        ave += vec[i];
    }
    ave /= (double)s;
    return ave;
}

double MHO_MathUtilities::angular_average(std::vector< double >& vec)
{
    std::size_t s = vec.size();
    if(s == 0)
    {
        return 0.0;
    }
    std::complex< double > ave = 0;
    std::complex< double > imagUnit = MHO_Constants::imag_unit;
    for(std::size_t i = 0; i < s; i++)
    {
        std::complex< double > phasor = std::exp(1.0 * imagUnit * (vec[i]));
        ave += phasor;
    }
    ave /= (double)s;
    return std::arg(ave);
}

//
// int
// MHO_MathUtilities::FindIntersection(double a, double b, double c, double d, double result[2])
// {
//     //looks for overlap between the intervals
//     //[a,b) and [c,d)
//     //although if a,b and c,d are the end-points of an intervals
//     //we do not explicitly assume they are ordered there
//
//     double arr[4];
//     int index[4];
//
//     arr[0] = a;
//     index[0] = 0;
//     arr[1] = b;
//     index[1] = 0;
//     arr[2] = c;
//     index[2] = 1;
//     arr[3] = d;
//     index[3] = 1;
//
//     if (arr[1] > arr[3]) {
//         std::swap(arr[1], arr[3]);
//         std::swap(index[1], index[3]);
//     };
//     if (arr[0] > arr[2]) {
//         std::swap(arr[0], arr[2]);
//         std::swap(index[0], index[2]);
//     };
//     if (arr[0] > arr[1]) {
//         std::swap(arr[0], arr[1]);
//         std::swap(index[0], index[1]);
//     };
//     if (arr[2] > arr[3]) {
//         std::swap(arr[2], arr[3]);
//         std::swap(index[2], index[3]);
//     };
//     if (arr[1] > arr[2]) {
//         std::swap(arr[1], arr[2]);
//         std::swap(index[1], index[2]);
//     };
//
//     //now the values in arr should be sorted in increasing order
//     //and the values in index should show which interval's end-points they belong to
//
//     //if the values in index have the form:
//     //0011 or 1100 then there is no overlap...although the end points may
//     //just touch
//
//     //if the values in the index have the form:
//     // 1001, 0110, 0101, or 1010 then there is overlap and the overlap interval
//     //is {arr[1], arr[2]}
//
//     int sum;
//     sum = index[0] + index[1];
//
//     if( (sum == 0) || (sum == 2) )
//     {
//         //there is no overlap, but we need to see if the end-points of the
//         //two intervals are the same number
//         if( arr[2] == arr[1] )
//         {
//             //endpoints are the same value
//             //call this an intersection of 1 point
//             result[0] = arr[1];
//             return 1;
//         }
//         else
//         {
//             //no intersection at all
//             return 0;
//         }
//     }
//     else
//     {
//         //there is overlap, but check how big the overlap interval is
//         if( arr[2] == arr[1] )
//         {
//             //the two overlapping points are the same value
//             //call this an intersection of 1 point
//             result[0] = arr[1];
//             return 1;
//         }
//         else
//         {
//             //overlap is larger than zero, return the interval of overlap
//             result[0] = arr[1];
//             result[1] = arr[2];
//             return 2;
//         }
//
//     }
// }

} // namespace hops
