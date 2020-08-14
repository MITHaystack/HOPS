#include <iostream>
#include <string>

#include "HkVectorContainer.hh"


using namespace hops;


int main(int /*argc*/, char** /*argv*/)
{

    size_t dim = 100;

    HkVectorContainer<double>* test = new HkVectorContainer<double>(&dim);

    std::cout<<"dimension @ 0 ="<<test->GetDimension(0)<<std::endl;
    std::cout<<"total array size = "<<test->GetSize()<<std::endl;

    double* data = test->GetRawData();

    for(unsigned int i=0; i<dim; i++)
    {
        data[i] = i%10;
    }

    std::cout<<"data @ 23 = "<<data[23]<<std::endl;

    delete test;

    return 0;
}
