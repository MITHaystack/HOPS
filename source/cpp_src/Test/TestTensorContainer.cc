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

    HkTensorContainer<double, 2 >* test = new HkTensorContainer<double, 2 >(dim);

    std::cout<<"dimension @ 0 ="<<test->GetDimension(0)<<std::endl;
    std::cout<<"dimension @ 1 ="<<test->GetDimension(1)<<std::endl;
    std::cout<<"total array size = "<<test->GetSize()<<std::endl;

    double* data = test->GetRawData();

    for(unsigned int i=0; i<dim[0]; i++)
    {
        for(unsigned int j=0; j<dim[1]; j++)
        {
            (*test)(i,j) = i+j;
        }
    }

    std::cout<<"data(2,3) = "<<(*test)(2,3)<<std::endl;

    delete test;
    delete dim;

    return 0;
}
