#include <iostream>

#include "HTimeStampConverter.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
    time_t current_time = std::time(nullptr);

    uint64_t epoch_sec = (uint64_t)current_time;
    double frac = 0.123;
    std::string tmp_date;

    bool ret_val;
    ret_val = HTimeStampConverter::ConvertEpochSecondToTimeStamp(epoch_sec, frac, tmp_date);
    std::cout<<"epoch second: "<<epoch_sec<<", frac: "<<frac<<std::endl;
    std::cout<<"time stamp: "<<tmp_date<<std::endl;

    uint64_t read_back_epoch_sec = 0;
    double read_back_frac = 0.0;
    ret_val = HTimeStampConverter::ConvertTimeStampToEpochSecond(tmp_date, read_back_epoch_sec, read_back_frac);

    std::cout<<"read back epoch second: "<<read_back_epoch_sec<<", frac: "<<read_back_frac<<std::endl;
    return 0;
}
