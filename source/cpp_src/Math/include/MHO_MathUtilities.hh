#ifndef MHO_MathUtilities_HH__
#define MHO_MathUtilities_HH__

#include <cmath>
#include <complex>
#include <string>
#include <vector>

namespace hops
{

/*!
 *@file MHO_MathUtilities.hh
 *@class MHO_MathUtilities
 *@author J. Barrett - barrettj@mit.edu
 *@date Tue Dec 5 17:01:15 2023 -0500
 *@brief implements a variety of simple math functions copied from original hops3 c code with minimal changes
 * along with some other simple helper functions
 */

/**
 * @brief Class MHO_MathUtilities
 */
class MHO_MathUtilities
{
    public:
        MHO_MathUtilities(){};
        virtual ~MHO_MathUtilities(){};

        //ported from hops3 c libraries
        
        /**
         * @brief Clamps a value between lower and upper bounds.
         * 
         * @param value The input value to be clamped.
         * @param lower The lower bound for clamping.
         * @param upper The upper bound for clamping.
         * @return The clamped value within the specified bounds.
         * @note This is a static function.
         */
        static double dwin(double value, double lower, double upper);
        /**
         * @brief Calculates parabola parameters and maximum x, amplitude values within a range.
         * 
         * @param y[3] Input coordinates for parabola calculation
         * @param lower Lower bound of the range
         * @param upper Upper bound of the range
         * @param x_max Output: Maximum x-value in the given range
         * @param amp_max Output: Maximum amplitude value in the given range
         * @param q[3] Output: Coefficients for parabola equation
         * @return Error code indicating interpolation status (0: success, 1: max at edge, 2: positive curvature)
         * @note This is a static function.
         */
        static int parabola(double y[3], double lower, double upper, double* x_max, double* amp_max, double q[3]);
        
        /**
         * @brief Calculates the inverse of a 3x3 matrix and stores it in ainv.
         * 
         * @param a[3][3] Input 3x3 matrix to be inverted
         * @param ainv[3][3] Parameter description
         * @return Non-zero if input matrix is singular, zero otherwise
         * @note This is a static function.
         */
        static int minvert3(double a[3][3], double ainv[3][3]);
        
        /**
         * @brief Performs linear interpolation between two points and returns the interpolated value.
         * 
         * @param coord1 Coordinate of the first point
         * @param value1 Value at the first coordinate
         * @param coord2 Coordinate of the second point
         * @param value2 Value at the second coordinate
         * @param coord Coordinate for which to interpolate
         * @param value Output parameter for the interpolated value
         * @return 0 if successful, -1 and error message on failure
         * @note This is a static function.
         */
        static int linterp (double coord1, double value1, double coord2, double value2, double coord, double *value);
        
        /**
         * @brief Calculates average phase from start to stop using given coordinates and values.
         * 
         * @param start Starting point for averaging
         * @param stop Ending point for averaging
         * @param coords Array of coordinate points
         * @param val1 First set of value points corresponding to coords
         * @param val2 Second set of value points corresponding to coords
         * @param n Number of points in coords and val arrays
         * @param nstart Starting point for averaging, saved from previous call
         * @param result1 Output: Average of first set of values
         * @param result2 Output: Average of second set of values
         * @return 0 on success, -1 on error
         * @note This is a static function.
         */
        static int ap_mean(double start, double stop, double *coords, double *val1, double *val2, int n, int *nstart, double *result1, double *result2);

        //returns the average of the values in a vector
        /**
         * @brief Calculates the average of values in a vector.
         * 
         * @param vec Input vector of doubles
         * @return Average value as double
         * @note This is a static function.
         */
        static double average(std::vector< double >& vec);

        //returns the average of the values in a vector assuming they are angles (radians)
        /**
         * @brief Calculates the average angle in radians from a vector of angles.
         * 
         * @param vec Input vector of angles in radians.
         * @return Average angle in radians.
         * @note This is a static function.
         */
        static double angular_average(std::vector< double >& vec);

        /**
         * @brief Calculates lower and upper frequency limits for a given channel based on sky frequency, bandwidth, and net sideband.
         * 
         * @param sky_freq Input sky frequency in MHz
         * @param bandwidth Bandwidth of the channel in MHz
         * @param net_sideband Network sideband ('U' for upper, 'L' for lower)
         * @param lower_freq Output lower frequency limit in MHz
         * @param upper_freq Output upper frequency limit in MHz
         * @note This is a static function.
         */
        static void DetermineChannelFrequencyLimits(double sky_freq, double bandwidth, std::string net_sideband,
                                                    double& lower_freq, double& upper_freq)
        {
            if(net_sideband == "U")
            {
                lower_freq = sky_freq;
                upper_freq = sky_freq + bandwidth;
                return;
            }
            if(net_sideband == "L")
            {
                upper_freq = sky_freq;
                lower_freq = sky_freq - bandwidth;
                return;
            }
            //not upper or lower sideband...assume double sideband (sky_freq is center)
            upper_freq = sky_freq + bandwidth;
            lower_freq = sky_freq - bandwidth;
        }

        /**
         * @brief Function FindIntersection
         * looks for overlap between the intervals
         * [a,b) and [c,d)
         * although if a,b and c,d are the end-points of an intervals
         * we do not explicitly assume they are ordered there
         * @tparam XValueType Template parameter XValueType
         * @param a (XValueType)
         * @param b (XValueType)
         * @param c (XValueType)
         * @param d (XValueType)
         * @param result[2] Parameter description
         * @return Return value (int)
         * @note This is a static function.
         */
        template< typename XValueType >
        static int FindIntersection(XValueType a, XValueType b, XValueType c, XValueType d, XValueType result[2])
        {


            XValueType arr[4];
            int index[4];

            arr[0] = a;
            index[0] = 0;
            arr[1] = b;
            index[1] = 0;
            arr[2] = c;
            index[2] = 1;
            arr[3] = d;
            index[3] = 1;

            if(arr[1] > arr[3])
            {
                std::swap(arr[1], arr[3]);
                std::swap(index[1], index[3]);
            };
            if(arr[0] > arr[2])
            {
                std::swap(arr[0], arr[2]);
                std::swap(index[0], index[2]);
            };
            if(arr[0] > arr[1])
            {
                std::swap(arr[0], arr[1]);
                std::swap(index[0], index[1]);
            };
            if(arr[2] > arr[3])
            {
                std::swap(arr[2], arr[3]);
                std::swap(index[2], index[3]);
            };
            if(arr[1] > arr[2])
            {
                std::swap(arr[1], arr[2]);
                std::swap(index[1], index[2]);
            };

            //now the values in arr should be sorted in increasing order
            //and the values in index should show which interval's end-points they belong to

            //if the values in index have the form:
            //0011 or 1100 then there is no overlap...although the end points may
            //just touch

            //if the values in the index have the form:
            // 1001, 0110, 0101, or 1010 then there is overlap and the overlap interval
            //is {arr[1], arr[2]}

            int sum;
            sum = index[0] + index[1];

            if((sum == 0) || (sum == 2))
            {
                //there is no overlap, but we need to see if the end-points of the
                //two intervals are the same number
                if(arr[2] == arr[1])
                {
                    //endpoints are the same value
                    //call this an intersection of 1 point
                    result[0] = arr[1];
                    return 1;
                }
                else
                {
                    //no intersection at all
                    return 0;
                }
            }
            else
            {
                //there is overlap, but check how big the overlap interval is
                if(arr[2] == arr[1])
                {
                    //the two overlapping points are the same value
                    //call this an intersection of 1 point
                    result[0] = arr[1];
                    return 1;
                }
                else
                {
                    //overlap is larger than zero, return the interval of overlap
                    result[0] = arr[1];
                    result[1] = arr[2];
                    return 2;
                }
            }
        }
};

} // namespace hops

#endif /*! end of include guard: MHO_MathUtilities_HH__ */
