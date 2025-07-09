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
aligns (within epsilon) with the original floating point (frequency) locations of each channel
*/

/**
 * @brief Class MHO_UniformGridPointsCalculator
 */
class MHO_UniformGridPointsCalculator
{
    public:
        MHO_UniformGridPointsCalculator();
        ~MHO_UniformGridPointsCalculator();

        /**
         * @brief Setter for epsilon
         * 
         * @param eps Input epsilon value of type double
         */
        void SetEpsilon(double eps) { fEpsilon = std::fabs(eps); };

        /**
         * @brief Setter for default grid points
         * 
         * @param n Input number of grid points
         */
        void SetDefaultGridPoints(std::size_t n) { fDefaultGridPoints = n; };



        /**
         * @brief pre-processing step -- makes sure the points are unique given some epsilon
         * and if not, then provides a std::map<int, int> to map the input vector indices to the output vector
         * @param in_pts Input vector of double precision points
         * @param eps Epsilon value for uniqueness check
         * @param out_pts Output vector of unique points
         * @param index_map Map from input index to output index
         */
        void GetUniquePoints(const std::vector<double>& in_pts, double eps, 
                        std::vector<double>& out_pts, 
                        std::map< std::size_t, std::size_t >& index_map) const;


        /**
         * @brief Setter for points - expects points to be given in increasing order
         * 
         * @param pts Input points vector
         */
        void SetPoints(const std::vector< double >& pts);
        
        /**
         * @brief Setter for points - expects points to be given in increasing order
         * 
         * @param pts Input vector of double values representing grid points
         */
        void SetPoints(const double* pts, std::size_t npts);
        
        /**
         * @brief Calculates uniform grid points and adjusts point count until spacing error is resolved.
         */
        void Calculate();

        /**
         * @brief Getter for grid points
         * 
         * @param grid_pts (std::vector< double >*)
         */
        void GetGridPoints(std::vector< double >* grid_pts); //fill vector with grid points

        /**
         * @brief Getter for grid start
         * 
         * @return The starting point of the grid as a double.
         */
        double GetGridStart() const { return fStart; };

        /**
         * @brief Getter for grid spacing - the distance between points on the uniform grid
         * 
         * @return Current grid spacing as a double.
         */
        double GetGridSpacing() const { return fSpacing; };

        /**
         * @brief Getter for grid average
         * 
         * @return The average point location as a double.
         */
        double GetGridAverage() const { return fAverageLocation; }

        /**
         * @brief Getter for spread - the grid spread about the average
         * 
         * @return Current spread value as a double
         */
        double GetSpread() const { return fSpread; }

        /**
         * @brief Getter for the number of points in the uniform grid
         * 
         * @return Number of grid points as std::size_t.
         */
        std::size_t GetNGridPoints() const { return fNGridPoints; };

        /**
         * @brief Getter for spacing error status
         * 
         * @return Boolean indicating whether there is a spacing error.
         */
        bool GetSpacingErrorStatus() const { return fSpacingError; };


        /**
         * @brief Getter for grid index map
         * maps the indexes of the original points to their new locations in the
         * uniform grid array
         * 
         * @return std::pair<std::size_t, std::size_t representing the index mapping.
         */
        std::map< std::size_t, std::size_t > GetGridIndexMap() { return fIndexMap; }

    protected:
        /**
         * @brief Calculates uniform grid points for frequency data up to a maximum number of points.
         * based on original implementation (freq_spacing.c), default max number of points is 8192
         * @param max_pts Maximum number of frequency points to consider.
         */
        void Calculate_v1(int max_pts = 8192); 
        
        /**
         * @brief Calculates uniform grid points without enforcing power-of-2 size - untested, not used
         */
        void Calculate_v2();

        /**
         * @brief Finds start point and minimum/maximum spacing between points in a list.
         */
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
