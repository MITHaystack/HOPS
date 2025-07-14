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

/**
 * @brief Class MHO_Interval
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

        /**
         * @brief Setter for bounds
         * 
         * @param lower_bound Lower bound value of type XIntegerType
         * @param upper_bound Upper bound value of type XIntegerType
         */
        void SetBounds(XIntegerType lower_bound, XIntegerType upper_bound) { SetIntervalImpl(lower_bound, upper_bound); }

        /**
         * @brief Setter for bounds
         * 
         * @param lower_bound Lower bound of type XIntegerType
         * @param upper_bound Upper bound of type XIntegerType
         */
        void SetBounds(const std::pair< XIntegerType, XIntegerType >& lower_upper)
        {
            SetIntervalImpl(lower_upper.first, lower_upper.second);
        }

        /**
         * @brief Getter for interval bounds
         * 
         * @return A pair of XIntegerType representing the current interval's lower and upper bounds.
         */
        std::pair< XIntegerType, XIntegerType > GetInterval() const
        {
            return std::pair< XIntegerType, XIntegerType >(fLowerBound, fUpperBound);
        }

        /**
         * @brief Setter for lower bound
         * 
         * @param low New lower bound value of type XIntegerType.
         */
        void SetLowerBound(XIntegerType low) { SetIntervalImpl(low, fUpperBound); }

        /**
         * @brief Setter for upper bound
         * 
         * @param up New upper bound value of type XIntegerType
         */
        void SetUpperBound(XIntegerType up) { SetIntervalImpl(fLowerBound, up); }

        /**
         * @brief Getter for lower bound
         * 
         * @return Lower bound value as XIntegerType
         */
        XIntegerType GetLowerBound() const { return fLowerBound; }

        /**
         * @brief Getter for upper bound
         * 
         * @return Upper bound value as XIntegerType
         */
        XIntegerType GetUpperBound() const { return fUpperBound; }

        /**
         * @brief Getter for length
         * 
         * @return Length calculated as fUpperBound - fLowerBound
         */
        XIntegerType GetLength() const { return fUpperBound - fLowerBound; }

        //test if this object itersects with an other interval
        /**
         * @brief Checks if this interval intersects with another.
         * 
         * @param other The other interval to check for intersection
         * @return True if there is an intersection, false otherwise
         */
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
        /**
         * @brief Checks if closed interval intersects with another interval.
         * 
         * @param idx (const XIntegerType&)
         * @return True if intervals intersect, false otherwise
         */
        bool Intersects(const XIntegerType& idx) const
        {
            if(fLowerBound <= idx && idx <= fUpperBound)
            {
                return true;
            }
            return false;
        }

        //returns the union of the two intervals
        /**
         * @brief Calculates and returns the union interval of this interval and another.
         * 
         * @param other The other interval to calculate the union with.
         * @return The union interval as an MHO_Interval object.
         */
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

        /**
         * @brief Checks if this interval intersects with another and returns true if it does.
         * 
         * @param other The other interval to check for intersection.
         * @return True if there is an intersection, false otherwise.
         */
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
        /**
         * @brief Setter for interval impl
         * 
         * @param low Lower bound of new interval (copied)
         * @param up Upper bound of new interval (copied)
         */
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
