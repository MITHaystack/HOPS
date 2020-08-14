#include <iostream>
#include <string>

#include "HkTensorContainer.hh"
#include "HkVectorContainer.hh"


using namespace hops;

typedef HkAxisPack< HkVectorContainer<double>, HkVectorContainer<int> > axis_pack;

int main(int /*argc*/, char** /*argv*/)
{

    size_t* dim = new size_t[2];
    dim[0] = 100;
    dim[1] = 100;



    HkTensorContainer<double, 2, axis_pack >* test = new HkTensorContainer<double, 2, axis_pack >(dim);

    std::cout<<"dimension @ 0 ="<<test->GetDimension(0)<<std::endl;
    std::cout<<"dimension @ 1 ="<<test->GetDimension(1)<<std::endl;
    std::cout<<"total array size = "<<test->GetSize()<<std::endl;

    for(unsigned int i=0; i<dim[0]; i++)
    {
        for(unsigned int j=0; j<dim[1]; j++)
        {
            (*test)(i,j) = i+j;
        }
    }

    std::cout<<"data(2,3) = "<<test->at(2,3)<<std::endl;

    // //std::cout<<"data(101,3) = "<<test->at(101,3)<<std::endl;
    // HkVectorContainer<double> axis0(dim);
    // HkVectorContainer<int> axis1(&(dim[1]));
    //
    // test->fAxisMap.insert(0,axis0);
    // test->fAxisMap.insert(1,axis1);

    std::cout<<"size of axis 0 = "<<std::get<0>(*test).GetSize()<<std::endl;
    // size_t* dim2 = new size_t[2];
    // dim2[0] = 50;
    // dim2[1] = 50;

    size_t* dim2 = new size_t[2];
    dim2[0] = 5;
    dim2[1] = 5;
    test->Resize(dim2);

    // std::get<0>( *test ).Resize(5);
    // std::get<1>( *test ).Resize(5);

    std::cout<<"size of axis 0 = "<<std::get<0>(*test).GetSize()<<std::endl;



    //
    // test->fAxes.resize_axis_pack(dim2);
    //
    // std::cout<<"size of axis 0 = "<<std::get<0>(test->fAxes).GetSize()<<std::endl;
    // std::cout<<"size of axis 1 = "<<std::get<1>(test->fAxes).GetSize()<<std::endl;
    //

    delete test;
    delete dim;

    return 0;
}
