#ifndef MHO_MathUtilities_HH__
#define MHO_MathUtilities_HH__



#include <complex>
#include <cmath>
#include <vector>


namespace hops
{

/*!
*@file MHO_MathUtilities.hh
*@class MHO_MathUtilities
*@author J. Barrett - barrettj@mit.edu
*@date Tue Dec 5 17:01:15 2023 -0500
*@brief implements a variety of simple math functions copied from original c code with minimal changes
*/


class MHO_MathUtilities
{
    public:
        MHO_MathUtilities(){};
        virtual ~MHO_MathUtilities(){};

        //ported from hops3 c libraries
        static double dwin(double value, double lower, double upper);
        static int parabola (double y[3], double lower, double upper, double* x_max, double* amp_max, double q[3]);
        static int minvert3( double a[3][3], double ainv[3][3]);

        //returns the average of the values in a vector
        static double average(std::vector<double>& vec);

        //returns the average of the values in a vector assuming they are angles (radians)
        static double angular_average(std::vector<double>& vec);

        template< typename XValueType >
        static int FindIntersection(XValueType a, XValueType b, XValueType c, XValueType d, XValueType result[2])
        {
            //looks for overlap between the intervals
            //[a,b) and [c,d)
            //although if a,b and c,d are the end-points of an intervals
            //we do not explicitly assume they are ordered there

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

            if (arr[1] > arr[3]) {
                std::swap(arr[1], arr[3]);
                std::swap(index[1], index[3]);
            };
            if (arr[0] > arr[2]) {
                std::swap(arr[0], arr[2]);
                std::swap(index[0], index[2]);
            };
            if (arr[0] > arr[1]) {
                std::swap(arr[0], arr[1]);
                std::swap(index[0], index[1]);
            };
            if (arr[2] > arr[3]) {
                std::swap(arr[2], arr[3]);
                std::swap(index[2], index[3]);
            };
            if (arr[1] > arr[2]) {
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

            if( (sum == 0) || (sum == 2) )
            {
                //there is no overlap, but we need to see if the end-points of the
                //two intervals are the same number
                if( arr[2] == arr[1] )
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
                if( arr[2] == arr[1] )
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

}

#endif /*! end of include guard: MHO_MathUtilities_HH__ */
