#include <iostream>
#include <string>

#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"

using namespace hops;


int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eDebug);
    MHO_Message::GetInstance().AcceptAllKeys();

    size_t dim[2] = {10, 10};

    MHO_NDArrayWrapper<double, 2> test(dim);

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

    MHO_NDArrayWrapper<double,2> test2(test);

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
    MHO_NDArrayWrapper<double, 2> test3(chunk, dim);

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

    std::cout<<"stride of dim 0 = "<<test3.GetStride(0)<<std::endl;
    std::cout<<"stride of dim 1 = "<<test3.GetStride(1)<<std::endl;

    std::cout<<"iterator increment operation"<<std::endl;
    arr_ind[0] = 0; arr_ind[1] = 0; arr_ind[2] = 0;
    do
    {
        std::cout<<"index iterator = ("<<arr_ind[0]<<", "<<arr_ind[1]<<", "<<arr_ind[2]<<")"<<std::endl;
    }
    while( MHO_NDArrayMath::IncrementIndices<3>(arr_dim, arr_ind));

    std::cout<<"iterator decrement operation"<<std::endl;
    arr_ind[0] = arr_dim[0]-1; arr_ind[1] = arr_dim[1]-1; arr_ind[2] = arr_dim[2]-1;
    do
    {
        std::cout<<"index iterator = ("<<arr_ind[0]<<", "<<arr_ind[1]<<", "<<arr_ind[2]<<")"<<std::endl;
    }
    while( MHO_NDArrayMath::DecrementIndices<3>(arr_dim, arr_ind));

    auto sit = test3.stride_begin(3);
    auto sit_end = test3.stride_end(3);

    std::cout<<"test strided access"<<std::endl;
    do
    {
        //std::array<std::size_t, 2> idx_ptr  = sit.GetIndexObject();
        //std::cout<<"index iterator = ("<<idx_ptr[0]<<", "<<idx_ptr[1]<<")"<<std::endl;
        sit++;
    }
    while(sit != sit_end && sit.IsValid() );

    std::size_t dim4[5] = {3,4,5,6,7};
    MHO_NDArrayWrapper<int, 5> test4(dim4);
    for(std::size_t i=0;i<5;i++)
    {
        std::cout<<"stride of dim "<<i<<" = "<<test4.GetStride(i)<<std::endl;
    }

    auto subview = test4.SubView(1,3);
    auto arrdim = subview.GetDimensionArray();
    for(std::size_t i=0; i<arrdim.size(); i++){std::cout<<"subview dim @"<<i<<" = "<<arrdim[i]<<std::endl;}


    return 0;
}
