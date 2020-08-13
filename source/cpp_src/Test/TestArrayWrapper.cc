#include <iostream>
#include <string>

#include "HkArrayWrapper.hh"

using namespace hops;


int main(int /*argc*/, char** /*argv*/)
{

    size_t dim[2] = {10, 10};

    HkArrayWrapper<double, 2> test(dim);

    std::cout<<"dimension @ 0 ="<<test.GetArrayDimension(0)<<std::endl;
    std::cout<<"total array size = "<<test.GetArraySize()<<std::endl;

    double* data = test.GetRawData();
    std::cout<<"ptr to data = "<<data<<std::endl;

    for(size_t i=0; i<dim[0]; i++)
    {
        for(size_t j=0; j<dim[1]; j++)
        {
            test(i,j) = i*dim[0]+j;
        }
    }

    HkArrayWrapper<double,2> test2(test);

    for(size_t i=0; i<dim[0]; i++)
    {

        for(size_t j=0; j<dim[1]; j++)
        {
            std::cout<<test2(i,j)<<", ";
        }
        std::cout<<std::endl;
    }

    return 0;
}
