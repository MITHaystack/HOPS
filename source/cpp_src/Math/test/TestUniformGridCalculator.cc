#include <vector>
#include <string>
#include <iostream>
#include <stdint.h>
#include <getopt.h>

#include "MHO_UniformGridPointsCalculator.hh"

using namespace hops;

int main(int argc, char** argv)
{

    std::vector<double> pts;
    // pts.push_back(1.0);
    // pts.push_back(2.0);
    pts.push_back(3.0);
    pts.push_back(5.0);
    pts.push_back(7.0);
    // pts.push_back(11.32332342);
    pts.push_back(13.0);
    pts.push_back(17.0);
    pts.push_back(17.1);
    pts.push_back(17.33);
    pts.push_back(19.0);
    pts.push_back(71.0);
    pts.push_back(72.0);

    MHO_UniformGridPointsCalculator grid_calc;
    grid_calc.SetPoints(pts);
    grid_calc.Calculate();

    std::cout<<"info: "<<grid_calc.GetGridStart()<<", "<<grid_calc.GetGridSpacing()<<", "<<grid_calc.GetNGridPoints()<<std::endl;

    return 0;

    
}
