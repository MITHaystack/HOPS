#include <array>
#include <complex>
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

    MHO_NDArrayWrapper< double, 2 > test(dim);

    std::cout << "dimension @ 0 =" << test.GetDimension(0) << std::endl;
    std::cout << "total array size = " << test.GetSize() << std::endl;

    double* data = test.GetData();
    std::cout << "ptr to data = " << data << std::endl;

    for(size_t i = 0; i < dim[0]; i++)
    {
        for(size_t j = 0; j < dim[1]; j++)
        {
            test(i, j) = i * dim[0] + j;
        }
    }

    MHO_NDArrayWrapper< double, 2 > test2(test);

    for(size_t i = 0; i < dim[0]; i++)
    {

        for(size_t j = 0; j < dim[1]; j++)
        {
            std::cout << test2(i, j) << ", ";
        }
        std::cout << std::endl;
    }

    //now lets test it on a bit of pre-allocated memory

    double* chunk = new double[100];
    MHO_NDArrayWrapper< double, 2 > test3(chunk, dim);

    std::cout << "ptr to data = " << chunk << " = " << test3.GetData() << std::endl;

    for(size_t i = 0; i < dim[0]; i++)
    {
        for(size_t j = 0; j < dim[1]; j++)
        {
            test3(i, j) = i * dim[0] + j;
        }
    }

    for(size_t i = 0; i < dim[0]; i++)
    {

        for(size_t j = 0; j < dim[1]; j++)
        {
            std::cout << test3(i, j) << ", ";
        }
        std::cout << std::endl;
    }

    test3.Resize(20, 20);

    std::size_t arr_dim[3];
    arr_dim[0] = 2;
    arr_dim[1] = 1;
    arr_dim[2] = 3;
    std::size_t arr_ind[3];

    std::cout << "stride of dim 0 = " << test3.GetStride(0) << std::endl;
    std::cout << "stride of dim 1 = " << test3.GetStride(1) << std::endl;

    std::cout << "iterator increment operation" << std::endl;
    arr_ind[0] = 0;
    arr_ind[1] = 0;
    arr_ind[2] = 0;
    do
    {
        std::cout << "index iterator = (" << arr_ind[0] << ", " << arr_ind[1] << ", " << arr_ind[2] << ")" << std::endl;
    }
    while(MHO_NDArrayMath::IncrementIndices< 3 >(arr_dim, arr_ind));

    std::cout << "iterator decrement operation" << std::endl;
    arr_ind[0] = arr_dim[0] - 1;
    arr_ind[1] = arr_dim[1] - 1;
    arr_ind[2] = arr_dim[2] - 1;
    do
    {
        std::cout << "index iterator = (" << arr_ind[0] << ", " << arr_ind[1] << ", " << arr_ind[2] << ")" << std::endl;
    }
    while(MHO_NDArrayMath::DecrementIndices< 3 >(arr_dim, arr_ind));

    auto sit = test3.stride_begin(3);
    auto sit_end = test3.stride_end(3);

    std::cout << "test strided access" << std::endl;
    do
    {
        //std::array<std::size_t, 2> idx_ptr  = sit.GetIndexObject();
        //std::cout<<"index iterator = ("<<idx_ptr[0]<<", "<<idx_ptr[1]<<")"<<std::endl;
        sit++;
    }
    while(sit != sit_end && sit.IsValid());

    std::size_t dim4[5] = {3, 4, 5, 6, 7};
    MHO_NDArrayWrapper< int, 5 > test4(dim4);

    auto test4_strides = test4.GetStrideArray();
    std::size_t test4_idx[5] = {1, 0, 3, 4, 2};

    std::size_t offset_method1 = MHO_NDArrayMath::OffsetFromRowMajorIndex< 5 >(dim4, test4_idx);
    std::size_t offset_method2 = MHO_NDArrayMath::OffsetFromStrideIndex< 5 >(&(test4_strides[0]), test4_idx);

    std::cout << "offset from row-major, stride index access = " << offset_method1 << ", " << offset_method2 << std::endl;

    std::size_t test4_idx2[5] = {2, 3, 1, 5, 0};
    offset_method1 = MHO_NDArrayMath::OffsetFromRowMajorIndex< 5 >(dim4, test4_idx2);
    offset_method2 = MHO_NDArrayMath::OffsetFromStrideIndex< 5 >(&(test4_strides[0]), test4_idx2);

    std::cout << "offset from row-major, stride index access = " << offset_method1 << ", " << offset_method2 << std::endl;

    for(std::size_t i = 0; i < 5; i++)
    {
        std::cout << "stride of dim " << i << " = " << test4.GetStride(i) << std::endl;
    }

    auto subview = test4.SubView(1, 3);
    auto arrdim = subview.GetDimensionArray();
    for(std::size_t i = 0; i < arrdim.size(); i++)
    {
        std::cout << "subview dim @" << i << " = " << arrdim[i] << std::endl;
    }

    subview += 1;
    subview *= 4;

    std::cout << "subview @ (0,1,2) = " << subview(0, 1, 2) << std::endl;

    std::size_t dim5[3] = {2, 3, 4};
    MHO_NDArrayWrapper< std::complex< double >, 3 > test5(dim5);
    for(std::size_t i = 0; i < test5.GetSize(); i++)
    {
        test5[i] = std::complex< double >(1.0, 2.0);
    }

    test5 += 3.1;
    test5 *= 2.0;

    std::cout << "test5(1,1,1) = " << test5(1, 1, 1) << std::endl;

    test5 -= test5;

    std::cout << "test5(1,1,1) = " << test5(1, 1, 1) << std::endl;

    //test the slice functionality
    auto slice = test4.SliceView(":", 1, 3, ":", 5);

    std::size_t dim6[3] = {5, 5, 5};
    MHO_NDArrayWrapper< int, 3 > test6(dim6);
    test6.ZeroArray();

    std::cout << "==============" << std::endl;

    for(int i = 0; i < dim6[0]; i++)
    {
        for(int j = 0; j < dim6[1]; j++)
        {
            for(int k = 0; k < dim6[2]; k++)
            {
                std::cout << test6(i, j, k) << ",";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
        std::cout << "-----------" << std::endl;
    }
    std::cout << std::endl;

    std::cout << "==============" << std::endl;

    auto slice1 = test6.SliceView(":", 2, 2);
    auto slice2 = test6.SliceView(2, ":", ":");

    MHO_NDArrayWrapper< int, 2 > tmp_copy;
    tmp_copy.Copy(slice2);

    for(auto itr = slice2.begin(); itr != slice2.end(); itr++)
    {
        *itr = 2;
    }
    for(auto itr = slice1.begin(); itr != slice1.end(); itr++)
    {
        *itr = 1;
    }

    slice1 *= 4.0;

    std::cout << "==============" << std::endl;

    for(int i = 0; i < dim6[0]; i++)
    {
        for(int j = 0; j < dim6[1]; j++)
        {
            for(int k = 0; k < dim6[2]; k++)
            {
                std::cout << test6(i, j, k) << ",";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
        std::cout << "-----------" << std::endl;
    }
    std::cout << std::endl;

    std::cout << "==============" << std::endl;

    return 0;
}
