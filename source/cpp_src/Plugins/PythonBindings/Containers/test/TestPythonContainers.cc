#include <iostream>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
namespace py = pybind11;

#include "MHO_Message.hh"

#include "MHO_Message.hh"
#include "MHO_FileKey.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_PyContainerStoreInterface.hh"

#include "MHO_ParameterStore.hh"
#include "MHO_PyParameterStoreInterface.hh"

using namespace hops;


int main()
{
    py::scoped_interpreter guard{}; // start the interpreter and keep it alive

    std::cout<<"the python path directories: "<<std::endl;
    py::exec(R"(
        import sys
        import numpy
        print(sys.path)
        import pyMHO_Containers
    )");


    //for now just create a vis object
    std::size_t dim[4] = {2,2,2,2};
    visibility_type* visibilities = new visibility_type(dim);

    auto ax0 = &( std::get<0>(*visibilities) );
    (*ax0)[0] = "LL";
    (*ax0)[1] = "RR";

    auto ax1 = &( std::get<1>(*visibilities) );
    (*ax1)[0] = 1;
    (*ax1)[1] = 2;

    auto ax2 = &( std::get<2>(*visibilities) );
    (*ax2)[0] = 0.1;
    (*ax2)[1] = 0.2;

    auto ax3 = &( std::get<3>(*visibilities) );
    (*ax3)[0] = 10.0;
    (*ax3)[1] = 20.0;



    for(size_t i=0; i<2; i++)
    {
        for(size_t j=0; j<2; j++)
        {
            for(size_t k=0; k<2; k++)
            {
                for(size_t l=0; l<2;l++)
                {
                    (*visibilities)(i,j,k,l) = std::complex<double>(   (i+1)*10+(j+1),  (k+1)*10+(l+1) );
                }
            }
        }
    }

    //print out on the c++ side
    for(size_t i=0; i<2; i++)
    {
        for(size_t j=0; j<2; j++)
        {
            for(size_t k=0; k<2; k++)
            {
                for(size_t l=0; l<2;l++)
                {
                    std::cout<<"vis("<<i<<","<<j<<","<<k<<","<<l<<") = "<<(*visibilities)(i,j,k,l)<<std::endl;
                }
            }
        }
    }

    std::cout<<"*************** passing visibilities to python **************"<<std::endl;

    MHO_ContainerStore store;

    MHO_FileKey key;
    key.fLabel = 0;
    strncpy(key.fName, "vis", 3);

    //stuff something in the container store
    store.AddObject(visibilities);
    store.SetObjectLabel(visibilities->GetObjectUUID(), key.fLabel);
    std::string shortname = std::string(key.fName, MHO_FileKeyNameLength ).c_str();
    store.SetShortName(visibilities->GetObjectUUID(), shortname);

    //now put the object uuid in the parameter store so we can look it up on the python side
    MHO_ParameterStore paramStore;
    std::string obj_uuid_string = visibilities->GetObjectUUID().as_string();

    paramStore.Set("vis_uuid", obj_uuid_string);

    //create the interfaces
    MHO_PyContainerStoreInterface conInter(&store);
    MHO_PyParameterStoreInterface parmInter(&paramStore);

    auto mho_test = py::module::import("mho_test");

    mho_test.attr("test_inter")(conInter, parmInter);

    //print out on the c++ side
    for(size_t i=0; i<2; i++)
    {
        for(size_t j=0; j<2; j++)
        {
            for(size_t k=0; k<2; k++)
            {
                for(size_t l=0; l<2;l++)
                {
                    std::cout<<"vis("<<i<<","<<j<<","<<k<<","<<l<<") = "<<visibilities->at(i,j,k,l)<<std::endl;
                }
            }
        }
    }


    // //try the same with a matrix object
    // std::size_t mxdims[2] = {2,2};
    // MHO_NDArrayWrapper<double, 2>* mx = new MHO_NDArrayWrapper<double, 2>(mxdims);
    // (*mx)(0,0) = 1;
    // (*mx)(0,1) = 2;
    // (*mx)(1,0) = 3;
    // (*mx)(1,1) = 4;
    //
    // auto strides = mx->GetStrides();
    // std::cout<<"arr strides for mx = "<<strides[0]<<", "<<strides[1]<<std::endl;
    //
    // auto bstrides = mx->GetByteStrides();
    // std::cout<<"byte strides for mx = "<<bstrides[0]<<", "<<bstrides[1]<<std::endl;
    //
    // //print out the python values
    // mho_test.attr("print_mx")(*mx);
    //
    // //try to modify the mx values using python
    // mho_test.attr("mod_mx")(*mx);
    //
    // std::cout<<(*mx)(0,0) <<", "<<(*mx)(0,1)<<", "<<(*mx)(1,0)<<", "<<(*mx)(1,1)<<std::endl;


    return 0;
}
