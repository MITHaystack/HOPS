#include <iostream>
#include <string>
#include <cmath>
#include <sstream>

#include "MHO_Timer.hh"
#include "MHO_TableContainer.hh"

#include "MHO_ContainerDefinitions.hh"

using namespace hops;

int main(int argc, char** argv)
{
    size_t dim[VIS_NDIM];
    dim[POLPROD_AXIS] = 4;
    dim[CHANNEL_AXIS] = 8;
    dim[TIME_AXIS] = 3;
    dim[FREQ_AXIS] = 3;

    ch_visibility_type vis;
    vis.Resize(dim);
    uint64_t total_size = vis.GetSize();

    size_t pc_dim[2];
    pc_dim[0] = 2;
    pc_dim[1] = 8;

    manual_pcal_type pcal;
    pcal.Resize(pc_dim);
    uint64_t pc_total_size = pcal.GetSize();

    std::complex<double> unit(1.0, 0.0);
    std::complex<double> i_unit(0.0, 2.0);

    vis.SetArray(unit);
    pcal.SetArray(i_unit);

    for(std::size_t i=0; i<pc_dim[1]; i++)
    {
        auto vis_slice = vis.SliceView(0, i, ":", ":");
        vis_slice *= pcal(0, i);;
    }


    for(std::size_t i=0; i<pc_dim[1]; i++)
    {
        auto vis_slice = vis.SliceView(2, i, ":", ":");
        vis_slice *= pcal(0, i);;
    }

    std::cout<< vis << std::endl;

    return 0;
}
