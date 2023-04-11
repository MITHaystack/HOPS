#ifndef MHO_UniformGridPointsCalculator_HH__
#define MHO_UniformGridPointsCalculator_HH__

#include <cmath>
#include <limits>
#include <map>
#include <vector>


/*
*File: 
*Class: 
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date:
*Description: This class re-implements the function freq_spacing from hops3.
Basically it is a primitive method to figure out an approximate (but uniformly spaced) grid which 
aligns (within epsilon) with the original floating point locations.
*/


namespace hops
{

class MHO_UniformGridPointsCalculator
{
    public:
        MHO_UniformGridPointsCalculator();
        ~MHO_UniformGridPointsCalculator();

        void SetEpsilon(double eps){fEpsilon = std::fabs(eps);};

        //expects points to be given in increasing order 
        void SetPoints(const std::vector<double>& pts);
        void SetPoints(const double* pts, std::size_t npts);


        void Calculate();

        void GetGridPoints(std::vector<double>* grid_pts);//fill vector with grid points

        double GetGridStart() const {return fStart;};//get value of start
        double GetGridSpacing() const {return fSpacing;}; //the distance between points on the uniform grid
        std::size_t GetNGridPoints() const {return fNGridPoints;}; //the number of points in the uniform grid 

        //maps the indexes of the original points to their new locations in the 
        //uniform grid array
        std::map<std::size_t, std::size_t> GetGridIndexMap(){return fIndexMap;}

    protected:

        void Calculate_v1(); //based on original implementation (freq_spacing.c)
        void Calculate_v2();

        void FindStartAndMinMaxSpacing();


        double fEpsilon;
        double fStart;
        double fSpacing;
        double fMinSpacing;
        double fMaxSpacing;
        double fNGridPoints;
        std::vector<double> fPoints;
        std::map<std::size_t, std::size_t> fIndexMap;

        double fAbsEps; //use to check that value is not zero

};

}

#endif /* end of include guard: MHO_UniformGridPointsCalculator_HH__ */