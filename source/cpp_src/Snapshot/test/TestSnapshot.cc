#include <iostream>
#include <string>

#include "MHO_Snapshot.hh"

using namespace hops;

int main(int argc, char** argv)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    MHO_Snapshot::GetInstance().AcceptAllKeys();
    MHO_Snapshot::GetInstance().SetExecutableName(argv[0]);

    size_t dim[VIS_NDIM];
    dim[POLPROD_AXIS] = 4;
    dim[CHANNEL_AXIS] = 8;
    dim[TIME_AXIS] = 3;
    dim[FREQ_AXIS] = 3;

    visibility_type vis;
    vis.Resize(dim);
    uint64_t total_size = vis.GetSize();

    std::complex<double> unit(1.0, 0.0);
    std::complex<double> i_unit(0.0, 1.0);

    vis.SetArray(unit);

    for(std::size_t i=0; i<dim[0]; i++)
    {
        auto vis_slice = vis.SliceView(i, ":", ":", ":");
        std::complex<double> tmp = ( (double)i )*i_unit;
        vis_slice *= tmp;
    }

    MHO_Snapshot::GetInstance().DumpObject(&vis, "test", "visib");

    return 0;
}
