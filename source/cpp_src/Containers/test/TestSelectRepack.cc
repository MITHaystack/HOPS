#include "MHO_Message.hh"
#include "MHO_SelectRepack.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_Axis.hh"

#include "MHO_TableContainer.hh"
#include "MHO_ContainerDefinitions.hh"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <complex>


using namespace hops;


int main(int /*argc*/, char** /*argv*/)
{

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    visibility_type in;
    visibility_type out;

    in.Resize(4,2,5,4);

    std::vector<std::size_t> ax0_idx;
    ax0_idx.push_back(0); 
    ax0_idx.push_back(2);

    std::vector<std::size_t> ax2_idx;
    ax2_idx.push_back(0); 
    ax2_idx.push_back(1);
    ax2_idx.push_back(3);

    MHO_SelectRepack<visibility_type> spack;
    spack.SelectAxisItems(0,ax0_idx);
    spack.SelectAxisItems(2,ax2_idx);
    spack.SetArgs(&in, &out);
    bool init = spack.Initialize();
    bool exe = spack.Execute();

    size_t odim_size[4];
    out.GetDimensions(odim_size);

    for(size_t i=0;i<4;i++)
    {
        std::cout<<"out dim @ "<<i<<" = "<<odim_size[i]<<std::endl;
    }

    return 0;
}
