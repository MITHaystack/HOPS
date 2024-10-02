#ifndef MHO_UniformGridPointsCalculator_HH__
#define MHO_UniformGridPointsCalculator_HH__

#include <cmath>
#include <limits>
#include <map>
#include <vector>

namespace hops
{

/*!
*@file
*@class
*@author J. Barrett - barrettj@mit.edu
*@date Thu Jan 12 16:03:58 2023 -0500
*@brief This class re-implements the function freq_spacing from hops3.
Basically it is a primitive method to figure out an approximate (but uniformly spaced) grid which
aligns (within epsilon) with the original floating point locations.
*/

class MHO_UniformGridPointsCalculator
{
    public:
        MHO_UniformGridPointsCalculator();
        ~MHO_UniformGridPointsCalculator();

        void SetEpsilon(double eps) { fEpsilon = std::fabs(eps); };

        void SetDefaultGridPoints(std::size_t n) { fDefaultGridPoints = n; };

        //expects points to be given in increasing order
        void SetPoints(const std::vector< double >& pts);
        void SetPoints(const double* pts, std::size_t npts);
        void Calculate();

        void GetGridPoints(std::vector< double >* grid_pts); //fill vector with grid points

        double GetGridStart() const { return fStart; }; //get value of start

        double GetGridSpacing() const { return fSpacing; }; //the distance between points on the uniform grid

        double GetGridAverage() const { return fAverageLocation; } //the average point location

        double GetSpread() const { return fSpread; } //the spread about the average

        std::size_t GetNGridPoints() const { return fNGridPoints; }; //the number of points in the uniform grid

        bool GetSpacingErrorStatus() const { return fSpacingError; };

        //maps the indexes of the original points to their new locations in the
        //uniform grid array
        std::map< std::size_t, std::size_t > GetGridIndexMap() { return fIndexMap; }

    protected:
        void Calculate_v1(int max_pts = 8192); //based on original implementation (freq_spacing.c)
        void Calculate_v2();

        void FindStartAndMinMaxSpacing();

        double fEpsilon;
        double fStart;
        double fSpacing;
        double fMinSpacing;
        double fMaxSpacing;
        double fAverageLocation;
        double fNGridPoints;
        double fSpread;
        std::vector< double > fPoints;
        std::map< std::size_t, std::size_t > fIndexMap;

        int fDefaultGridPoints;
        bool fSpacingError;
        double fAbsEps; //use to check that value is not zero
};

} // namespace hops

#endif /*! end of include guard: MHO_UniformGridPointsCalculator_HH__ */
