#include "MHO_Message.hh"

#include "MHO_OpenCLBatchedMultidimensionalFastFourierTransform.hh"
#include "MHO_FastFourierTransform.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"

#include <cmath>
#include <iomanip>
#include <iostream>

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
    const size_t p = 1;
    const size_t stride = ((p + 1) * (p + 2)) / 2;
    const size_t d = 3;
    const size_t z = 1;
    const size_t div_size = 8;// 2 * d * (z + 1);


    const size_t batch_size = 1;// stride;
    const size_t ndim = 4;
    const size_t dim_size[ndim] = {batch_size, div_size, div_size, div_size};

    const size_t total_size = dim_size[0] * dim_size[1] * dim_size[2] * dim_size[3];

    std::cout<<"total size = "<<total_size<<std::endl;

    double spatial_size = dim_size[1] * dim_size[2] * dim_size[3];

    auto* raw_data = new std::complex<double>[total_size];
    typedef MHO_NDArrayWrapper<std::complex<double>, ndim> arrType;


    arrType input;
    input.Resize(dim_size);


    //fill up the array with a signal
    for (size_t i = 0; i < total_size; i++) {
        raw_data[i] = i;
    }

    size_t index[ndim];
    size_t count = 0;

    for (size_t a = 0; a < batch_size; a++) {
        index[0] = a;
        count = 0;
        for (size_t i = 0; i < dim_size[1]; i++) {
            index[1] = i;
            for (size_t j = 0; j < dim_size[2]; j++) {
                index[2] = j;

                for (size_t k = 0; k < dim_size[3]; k++) {
                    index[3] = k;
                    input(a,i,j,k) = std::complex<double>(count % 13, count % 3);
                    count++;
                }
                
            }
            
        }
    }

    auto* fft_eng = new MHO_OpenCLBatchedMultidimensionalFastFourierTransform< arrType >();

    fft_eng->SetForward();
    fft_eng->SetInput(&input);
    fft_eng->SetOutput(&input);

    fft_eng->Initialize();

    fft_eng->Execute();


    fft_eng->SetBackward();
    fft_eng->Execute();

    count = 0;
    double l2_norm = 0;
    double norm = spatial_size;

    for (size_t a = 0; a < batch_size; a++) {
        l2_norm = 0;
        count = 0;
        index[0] = a;
        for (size_t i = 0; i < dim_size[1]; i++) {
            index[1] = i;
            for (size_t j = 0; j < dim_size[2]; j++) {
                index[2] = j;

                for (size_t k = 0; k < dim_size[3]; k++) {
                    index[3] = k;
                    std::complex<double> del = input(a,i,j,k) / norm;
                    del -= std::complex<double>(count % 13, count % 3);

                    l2_norm += std::real(del) * std::real(del) + std::imag(del) * std::imag(del);

                    count++;
                }
                
            }
            
        }

        //std::cout << "L2 norm difference = " << std::sqrt(l2_norm) << std::endl;
    }


    delete fft_eng;

    return 0;
}
