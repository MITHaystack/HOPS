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
    std::cout<<"WARNING: make sure to compile code as 'Release' for an accurate time measurement."<<std::endl;

    size_t dim[CH_VIS_NDIM];
    dim[CH_POLPROD_AXIS] = 4;
    dim[CH_CHANNEL_AXIS] = 256*SCALE_FACTOR;
    dim[CH_TIME_AXIS] = 256*SCALE_FACTOR;
    dim[CH_FREQ_AXIS] = 256*SCALE_FACTOR;

    ch_baseline_data_type vis;
    vis.Resize(dim);
    uint64_t total_size = vis.GetSize();
    uint64_t MB = 1024*1024;

    std::complex<double> val(1.0, 0.0);

    MHO_Timer timer;
    timer.MeasureWallclockTime();
    double delta = 0;

    std::cout<<"container size = "<< (sizeof(std::complex<double>)*total_size)/MB <<" MB." <<std::endl;

    //time how long it takes to zero the array 
    timer.Start();
    vis.ZeroArray();
    timer.Stop();
    delta = timer.GetDurationAsDouble();
    std::cout<<"time to zero array: "<<delta<< " seconds "<<std::endl;

    //time how long it takes to set the array to a single value
    timer.Start();
    vis.SetArray(val);
    timer.Stop();
    delta = timer.GetDurationAsDouble();
    std::cout<<"time to set array to same value: "<<delta<< " seconds "<<std::endl;

    //time how long it takes to iterate through using pointer arithmetic
    timer.Start();
    std::complex<double>* ptr = vis.GetData();
    for(uint64_t i=0; i<total_size; i++)
    {
        *ptr = val;
        ++ptr;
    }
    timer.Stop();
    delta = timer.GetDurationAsDouble();
    std::cout<<"time to fill array with pointer: "<<delta<< " seconds "<<std::endl;

    //now time how long it takes to iterate through the data with access iterators
    timer.Start();
    for(auto it = vis.begin(); it != vis.end(); it++)
    {
        *it = val;
    }
    timer.Stop();
    delta = timer.GetDurationAsDouble();
    std::cout<<"time to fill array with iterator: "<<delta<< " seconds "<<std::endl;


    //now time how long it takes to iterate through the data with [] operator
    timer.Start();
    for(uint64_t i=0; i<total_size; i++)
    {
        vis[i] = val;
    }
    timer.Stop();
    delta = timer.GetDurationAsDouble();
    std::cout<<"time to fill array with [] operator: "<<delta<< " seconds "<<std::endl;


    //time how long it takes to iterate through (,,,) access operator
    timer.Start();
    for(uint64_t i=0; i<dim[0]; i++){
        for(uint64_t j=0; j<dim[1]; j++){
            for(uint64_t k=0; k<dim[2]; k++){
                for(uint64_t l=0; l<dim[3]; l++){
                    vis(i,j,k,l) = val;
                }
            }
        }
    }
    timer.Stop();
    delta = timer.GetDurationAsDouble();
    std::cout<<"time to fill array with (,,,) operator: "<<delta<< " seconds "<<std::endl;

    //time how long it takes to iterate through at(,,,) access operator
    timer.Start();
    for(uint64_t i=0; i<dim[0]; i++){
        for(uint64_t j=0; j<dim[1]; j++){
            for(uint64_t k=0; k<dim[2]; k++){
                for(uint64_t l=0; l<dim[3]; l++){
                    vis.at(i,j,k,l) = val;
                }
            }
        }
    }
    timer.Stop();
    delta = timer.GetDurationAsDouble();
    std::cout<<"time to fill array with (bounds checked) at(,,,) operator: "<<delta<< " seconds "<<std::endl;

    //time how long it takes to iterate the array using SubView
    timer.Start();
    uint64_t z = 0; //this is annoying (plain old '0' is treated as an int by the compiler)
    for(uint64_t i=0; i<dim[0]; i++){
        auto sub = vis.SubView(i);
        for(uint64_t j=0; j<dim[1]; j++){
            for(uint64_t k=0; k<dim[2]; k++){
                for(uint64_t l=0; l<dim[3]; l++){
                    sub(j,k,l) = val;
                }
            }
        }
    }
    timer.Stop();
    delta = timer.GetDurationAsDouble();
    std::cout<<"time to fill array with 1st dimension SubView and (,,) operator: "<<delta<< " seconds "<<std::endl;



    return 0;
}
