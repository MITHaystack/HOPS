#ifndef HkInterval_HH__
#define HkInterval_HH__

/*
*File: HkInterval.hh
*Class: HkInterval
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: Implements the open integer interval [a,b)
*/

#include <utility>
#include <algorithm>

namespace hops
{

template< typename XIntegerType = std::size_t >
class HkInterval
{
    public:

        HkInterval():
            fValid(false),
            fLowerBound(0),
            fUpperBound(0)
        {};

        HkInterval(XIntegerType lower_bound, XIntegerType upper_bound):
            fValid(false)
        {
            SetIntervalImpl(lower_bound,upper_bound);
        };

        HkInterval(const HkInterval& copy):
            fValid(false)
        {
            SetIntervalImpl(copy.fLowerBound, copy.fUpperBound);
        }

        virtual ~HkInterval(){};

        void SetBounds(XIntegerType lower_bound, XIntegerType upper_bound)
        {
            SetIntervalImpl(lower_bound,upper_bound);
        }

        void SetBounds(const std::pair<XIntegerType,XIntegerType>& lower_upper)
        {
            SetIntervalImpl(lower_upper.first, lower_upper.second);
        }

        std::pair<XIntegerType,XIntegerType> GetInterval() const
        {
            return std::pair<XIntegerType,XIntegerType>(fLowerBound, fUpperBound);
        }


        void SetLowerBound(XIntegerType low)
        {
            SetIntervalImpl(low, fUpperBound);
        }


        void SetUpperBound(XIntegerType up)
        {
            SetIntervalImpl(fLowerBound, up);
        }

        XIntegerType GetLowerBound() const
        {
            return fLowerBound;
        }

        XIntegerType GetUpperBound() const
        {
            return fUpperBound;
        }

        //test if this object itersects with an other interval
        bool Intersects(HkInterval& other) const
        {
            XIntegerType result[2];
            int numIntersections;
            numIntersections = FindIntersection(fLowerBound, fUpperBound, other.GetLowerBound(), other.GetUpperBound(), result);
            if(numIntersections != 0)
            {
                return true;
            }
            return false;
        }

        //returns the union of the two intervals
        HkInterval Union(HkInterval& other) const
        {
            HkInterval interval;
            XIntegerType result[2];
            int numIntersections;
            numIntersections = FindIntersection(fLowerBound, fUpperBound, other.GetLowerBound(), other.GetUpperBound(), result);
            if(numIntersections != 0)
            {
                //the two intervals do intersect, so the union of the
                //new interval is just the min/max of the following
                XIntegerType low  = std::min(fLowerBound, other.GetLowerBound() );
                XIntegerType up = std::max(fUpperBound, other.GetUpperBound() );
                interval.SetInterval(low, up);
            }
            //they don't intersect, so just return an empty interval
            return interval;
        }

        HkInterval Intersection(const HkInterval& other) const
        {
            HkInterval interval;
            XIntegerType result[2];
            int numIntersections;
            numIntersections = FindIntersection(fLowerBound, fUpperBound, other.GetLowerBound(), other.GetUpperBound(), result);
            if(numIntersections != 0)
            {
                interval.SetInterval(result[0], result[1]);
            }
            return interval;
        }

        HkInterval& operator=(const HkInterval& rhs)
        {
            if(this != &rhs)
            {
                fValid = rhs.fValid;
                fLowerBound = rhs.fLowerBound;
                fUpperBound = rhs.fUpperBound;
            }
            return *this;
        }

    protected:

        void SetIntervalImpl(XIntegerType low, XIntegerType up)
        {
            if(low < up)
            {
                fLowerBound = low;
                fUpperBound = up;
                fValid = true;
            }
            else if (up < low)
            {
                fLowerBound = up;
                fUpperBound = low;
                fValid = true;
            }
            else if (low == up)
            {
                fLowerBound = low;
                fUpperBound = up;
                fValid = true;
            }

        }

        int FindIntersection(XIntegerType a, XIntegerType b, XIntegerType c, XIntegerType d, XIntegerType result[2])
        {
            //looks for overlap between the intervals
            //[a,b) and [c,d)
            //although if a,b and c,d are the end-points of an intervals
            //we do not explicitly assume they are ordered there

            XIntegerType arr[4];
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


        bool fValid; //boolean to indicate if any of the upper/lower bounds have been set
        XIntegerType fLowerBound;
        XIntegerType fUpperBound;

};


} //end of namespace

#endif /* end of include guard: HkInterval */
