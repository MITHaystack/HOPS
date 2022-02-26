#include <iostream>
#include <string>
#include <cmath>
#include <sstream>

#include "MHO_Timer.hh"
#include "MHO_TableContainer.hh"
#include "MHO_ChannelizedVisibilities.hh"

using namespace hops;

#define SCALE_FACTOR 2 //value of 2 should make a table 8gb in size

int main(int argc, char** argv)
{
    size_t dim[CH_VIS_NDIM];
    dim[CH_POLPROD_AXIS] = 4;
    dim[CH_CHANNEL_AXIS] = 256*SCALE_FACTOR;
    dim[CH_TIME_AXIS] = 256*SCALE_FACTOR;
    dim[CH_FREQ_AXIS] = 256*SCALE_FACTOR;

    ch_baseline_data_type* vis = new ch_baseline_data_type(dim);
    uint64_t total_size = vis->GetSize();

    std::complex<double> val(1.0, 0.0);

    MHO_Timer timer;
    timer.MeasureWallclockTime();
    double delta = 0;
    //now time how long it takes to iterate through the data with access iterators
    //and set a value
    timer.Start();
    for(auto it = vis->begin(); it != vis->end(); it++)
    {
        *it = val;
    }
    timer.Stop();
    delta = timer.GetDurationAsDouble();
    std::cout<<"time to fill array with iterator: "<<delta<< " seconds "<<std::endl;

    //time now long it takes to iterate through using pointer arithmetic
    timer.Start();
    std::complex<double>* ptr = vis->GetData();
    for(uint64_t i=0; i<total_size; i++)
    {
        *ptr = val;
        ptr++;
    }
    timer.Stop();
    delta = timer.GetDurationAsDouble();
    std::cout<<"time to fill array with pointer: "<<delta<< " seconds "<<std::endl;

    delete vis;

    return 0;
}
