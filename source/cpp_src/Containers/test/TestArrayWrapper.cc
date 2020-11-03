#include <iostream>
#include <string>

#include "MHOArrayWrapper.hh"

using namespace hops;


int main(int /*argc*/, char** /*argv*/)
{

    size_t dim[2] = {10, 10};

    MHOArrayWrapper<double, 2> test(dim);

    std::cout<<"dimension @ 0 ="<<test.GetDimension(0)<<std::endl;
    std::cout<<"total array size = "<<test.GetSize()<<std::endl;

    double* data = test.GetData();
    std::cout<<"ptr to data = "<<data<<std::endl;

    for(size_t i=0; i<dim[0]; i++)
    {
        for(size_t j=0; j<dim[1]; j++)
        {
            test(i,j) = i*dim[0]+j;
        }
    }

    MHOArrayWrapper<double,2> test2(test);

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
