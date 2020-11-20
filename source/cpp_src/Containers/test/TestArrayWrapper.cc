#include <iostream>
#include <string>

#include "MHOMessage.hh"
#include "MHOArrayWrapper.hh"

using namespace hops;


int main(int /*argc*/, char** /*argv*/)
{
    MHOMessage::GetInstance().SetMessageLevel(eDebug);
    MHOMessage::GetInstance().AcceptAllKeys();

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

    //now lets test it on a bit of pre-allocated memory

    double* chunk = new double[100];
    MHOArrayWrapper<double, 2> test3(chunk, dim);

    std::cout<<"ptr to data = "<<chunk<<" = "<< test3.GetData()<<std::endl;

    for(size_t i=0; i<dim[0]; i++)
    {
        for(size_t j=0; j<dim[1]; j++)
        {
            test3(i,j) = i*dim[0]+j;
        }
    }

    for(size_t i=0; i<dim[0]; i++)
    {

        for(size_t j=0; j<dim[1]; j++)
        {
            std::cout<<test3(i,j)<<", ";
        }
        std::cout<<std::endl;
    }

    test3.Resize(20,20);

    std::size_t arr_dim[3];
    arr_dim[0] = 2; arr_dim[1] = 1; arr_dim[2] = 3;
    std::size_t arr_ind[3];


    std::cout<<"iterator increment operation"<<std::endl;
    arr_ind[0] = 0; arr_ind[1] = 0; arr_ind[2] = 0;
    do
    {
        std::cout<<"index iterator = ("<<arr_ind[0]<<", "<<arr_ind[1]<<", "<<arr_ind[2]<<")"<<std::endl;
    }
    while( MHOArrayMath::IncrementIndices<3>(arr_dim, arr_ind));

    std::cout<<"iterator decrement operation"<<std::endl;
    arr_ind[0] = arr_dim[0]-1; arr_ind[1] = arr_dim[1]-1; arr_ind[2] = arr_dim[2]-1;
    do
    {
        std::cout<<"index iterator = ("<<arr_ind[0]<<", "<<arr_ind[1]<<", "<<arr_ind[2]<<")"<<std::endl;
    }
    while( MHOArrayMath::DecrementIndices<3>(arr_dim, arr_ind));




    return 0;
}
