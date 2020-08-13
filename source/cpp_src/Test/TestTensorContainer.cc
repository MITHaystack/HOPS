#include <iostream>
#include <string>

#include "HkTensorContainer.hh"
#include "HkVectorContainer.hh"


using namespace hops;


int main(int /*argc*/, char** /*argv*/)
{

    size_t* dim = new size_t[2];
    dim[0] = 100;
    dim[1] = 100;

    HkTensorContainer<double, HkEmptyUnit, 2 >* test =
        new HkTensorContainer<double, HkEmptyUnit, 2 >(dim);

    std::cout<<"dimension @ 0 ="<<test->GetArrayDimension(0)<<std::endl;
    std::cout<<"dimension @ 1 ="<<test->GetArrayDimension(1)<<std::endl;
    std::cout<<"total array size = "<<test->GetArraySize()<<std::endl;

    double* data = test->GetRawData();

    for(unsigned int i=0; i<dim[0]; i++)
    {
        for(unsigned int j=0; j<dim[1]; j++)
        data[i*dim[0] + j ] = i%10;
    }

    std::cout<<"data @ 23 = "<<data[23]<<std::endl;

    delete test;
    delete dim;

    return 0;
}
