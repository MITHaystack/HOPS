#ifndef MHO_Interval_HH__
#define MHO_Interval_HH__

#include <algorithm>
#include <utility>

#include "MHO_MathUtilities.hh"

namespace hops
{

/*!
 *@file MHO_Interval.hh
 *@class MHO_Interval
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Sep 17 10:45:44 2020 -0400
 *@brief Implements the open integer interval [a,b)
 */

template< typename XIntegerType = std::size_t > class MHO_Interval
{
    public:
        MHO_Interval(): fValid(false), fLowerBound(0), fUpperBound(0){};

        MHO_Interval(XIntegerType lower_bound, XIntegerType upper_bound): fValid(false)
        {
            SetIntervalImpl(lower_bound, upper_bound);
        };

        MHO_Interval(const MHO_Interval& copy): fValid(false) { SetIntervalImpl(copy.fLowerBound, copy.fUpperBound); }

        virtual ~MHO_Interval(){};

        void SetBounds(XIntegerType lower_bound, XIntegerType upper_bound) { SetIntervalImpl(lower_bound, upper_bound); }

        void SetBounds(const std::pair< XIntegerType, XIntegerType >& lower_upper)
        {
            SetIntervalImpl(lower_upper.first, lower_upper.second);
        }

        std::pair< XIntegerType, XIntegerType > GetInterval() const
        {
            return std::pair< XIntegerType, XIntegerType >(fLowerBound, fUpperBound);
        }

        void SetLowerBound(XIntegerType low) { SetIntervalImpl(low, fUpperBound); }

        void SetUpperBound(XIntegerType up) { SetIntervalImpl(fLowerBound, up); }

        XIntegerType GetLowerBound() const { return fLowerBound; }

        XIntegerType GetUpperBound() const { return fUpperBound; }

        XIntegerType GetLength() const { return fUpperBound - fLowerBound; }

        //test if this object itersects with an other interval
        bool Intersects(const MHO_Interval& other) const
        {
            XIntegerType result[2];
            int numIntersections;
            numIntersections = MHO_MathUtilities::FindIntersection< XIntegerType >(
                fLowerBound, fUpperBound, other.GetLowerBound(), other.GetUpperBound(), result);
            if(numIntersections != 0)
            {
                return true;
            }
            return false;
        }

        //test if the closed interval itersects with a point
        bool Intersects(const XIntegerType& idx) const
        {
            if(fLowerBound <= idx && idx <= fUpperBound)
            {
                return true;
            }
            return false;
        }

        //returns the union of the two intervals
        MHO_Interval Union(const MHO_Interval& other) const
        {
            MHO_Interval interval;
            XIntegerType result[2];
            int numIntersections;
            numIntersections = MHO_MathUtilities::FindIntersection< XIntegerType >(
                fLowerBound, fUpperBound, other.GetLowerBound(), other.GetUpperBound(), result);
            if(numIntersections != 0)
            {
                //the two intervals do intersect, so the union of the
                //new interval is just the min/max of the following
                XIntegerType low = std::min(fLowerBound, other.GetLowerBound());
                XIntegerType up = std::max(fUpperBound, other.GetUpperBound());
                interval.SetInterval(low, up);
            }
            //they don't intersect, so just return an empty interval
            return interval;
        }

        MHO_Interval Intersection(const MHO_Interval& other) const
        {
            MHO_Interval interval;
            XIntegerType result[2];
            int numIntersections;
            numIntersections = MHO_MathUtilities::FindIntersection< XIntegerType >(
                fLowerBound, fUpperBound, other.GetLowerBound(), other.GetUpperBound(), result);
            if(numIntersections != 0)
            {
                interval.SetInterval(result[0], result[1]);
            }
            return interval;
        }

        MHO_Interval& operator=(const MHO_Interval& rhs)
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
            else if(up < low)
            {
                fLowerBound = up;
                fUpperBound = low;
                fValid = true;
            }
            else if(low == up)
            {
                fLowerBound = low;
                fUpperBound = up;
                fValid = true;
            }
        }

        bool fValid; //boolean to indicate if any of the upper/lower bounds have been set
        XIntegerType fLowerBound;
        XIntegerType fUpperBound;
};

} // namespace hops

#endif /*! end of include guard: MHO_Interval */
