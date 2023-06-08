#include <iostream>

#include "MHO_ParameterStore.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{

    MHO_ParameterStore params;
    
    std::string key1 = "/global/dr_win";
    std::vector<double> val1 = {1.0, 2.0};
    std::string key2 = "/station/E/pcal_mode";
    std::string val2 = "manual";

    params.Set(key1, val1);
    params.Set(key2, val2);
    
    std::vector<double> readback1;
    std::string readback2;
    
    params.Get(key1, readback1);
    params.Get(key2, readback2);
    
    std::cout<<key1<<" = "<<readback1.at(0)<<", "<<readback1.at(1)<<std::endl;
    std::cout<<key2<<" = "<<readback2<<std::endl;

    std::string key3 = "/baseline/GE/fringe_algo";
    std::string key4 = "/baseline/XY/fringe_algo";
    std::string val3 = "default";
    std::string val4 = "special";
    
    params.Set(key3, val3);
    params.Set(key4, val4);

    std::cout<<key3<<" = "<<params.GetAs<std::string>(key3)<<std::endl;
    std::cout<<key4<<" = "<<params.GetAs<std::string>(key4)<<std::endl;

    return 0;
}
