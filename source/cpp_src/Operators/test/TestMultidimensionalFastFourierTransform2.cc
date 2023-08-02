#include "MHO_Message.hh"
#include "MHO_Timer.hh"
#include "MHO_FastFourierTransform.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"

#include <cmath>
#include <iomanip>
#include <iostream>

using namespace hops;

typedef double FPTYPE;

#define ARRAY1_TYPE MHO_NDArrayWrapper< std::complex<FPTYPE>, 1 >
#define ARRAY2_TYPE MHO_NDArrayWrapper< std::complex<FPTYPE>, 2 > 
#define ARRAY3_TYPE MHO_NDArrayWrapper< std::complex<FPTYPE>, 3 > 
#define ARRAY4_TYPE MHO_NDArrayWrapper< std::complex<FPTYPE>, 4 > 

#define FFT1_TYPE MHO_MultidimensionalFastFourierTransform<ARRAY1_TYPE>
#define FFT2_TYPE MHO_MultidimensionalFastFourierTransform<ARRAY2_TYPE>
#define FFT3_TYPE MHO_MultidimensionalFastFourierTransform<ARRAY3_TYPE>
#define FFT4_TYPE MHO_MultidimensionalFastFourierTransform<ARRAY4_TYPE>

#define PRINT_DETAIL

template< typename XArrayType, typename XFFTType>
int 
run_test(const std::vector<std::size_t>& dim_size, const std::vector<std::size_t>& selected_axes)
{
    if(dim_size.size() != XArrayType::rank::value)
    {
        std::cout<<"dimension rank error: "<<dim_size.size()<<" != "<<XArrayType::rank::value<<std::endl; std::exit(1);
    }
    
    XArrayType* input = new XArrayType();
    XArrayType* original = new XArrayType();
    input->Resize(&(dim_size[0]));
    input->ZeroArray();
    std::size_t total_size = input->GetSize();
    
    //fill up the array with data 
    for(std::size_t i=0; i<total_size; i++)
    {
        (*input)[i] = std::complex<FPTYPE>(i % 13, i % 17);
    }
    
    //make a copy of the original input
    (*original) = (*input);
    
    //we do the FFT in two stages...first we transform the array along the axes specified in 
    //initial_selected_axes, then if any axes are left, we to those next.
    //Finally we do a single-pass (all-axes) inverse FFT and compare to the original array
    std::vector<std::size_t> first_pass_axes;
    std::vector<std::size_t> second_pass_axes;
    for(std::size_t i=0; i<XArrayType::rank::value; i++)
    {
        bool selected = false;
        for(std::size_t j=0; j<selected_axes.size(); j++){ if(i==selected_axes[j]){selected = true;} }
        if(selected){ first_pass_axes.push_back(i); }
        else{ second_pass_axes.push_back(i);}
    }

    XFFTType* fft_engine = new XFFTType();

    //do FFT on first set of axes 
    if(first_pass_axes.size() > 0)
    {
        fft_engine->SetForward();
        fft_engine->SetArgs(input);
        fft_engine->DeselectAllAxes();
        for(std::size_t i=0; i<first_pass_axes.size(); i++)
        {
            fft_engine->SelectAxis(first_pass_axes[i]);
        }
        fft_engine->Initialize();
        fft_engine->Execute();
    }

    //do FFT on remaining axes
    if(second_pass_axes.size() > 0)
    {
        fft_engine->SetForward();
        fft_engine->SetArgs(input);
        fft_engine->DeselectAllAxes();
        for(std::size_t i=0; i<second_pass_axes.size(); i++)
        {
            fft_engine->SelectAxis(second_pass_axes[i]);
        }
        fft_engine->Initialize();
        fft_engine->Execute();
    }
    
    
    //no do IFFT pass on all axes
    fft_engine->SetBackward();
    fft_engine->SetArgs(input);
    fft_engine->SelectAllAxes();
    fft_engine->Initialize();

    MHO_Timer timer;
    timer.MeasureWallclockTime();
    timer.Start();
    fft_engine->Execute();
    timer.Stop();
    double runtime = timer.GetDurationAsDouble();

    FPTYPE norm = total_size;
    FPTYPE l2_norm = 0;

    for(std::size_t i=0; i<total_size; i++)
    {
        
        std::complex<FPTYPE> del = (*input)[i] / norm;
        del -= (*original)[i];
        l2_norm += std::real(del) * std::real(del) + std::imag(del) * std::imag(del);
    }
    
    double err = std::sqrt(l2_norm);
    std::cout << "--------------------------" << std::endl;
    std::cout << "single pass wallclock time (s): " << runtime << std::endl;
    std::cout << "L2_diff = " << err << std::endl;
    std::cout << "L2_diff/N = "<< err/norm <<std::endl; //average error per point
    double tol = 1.5e-15;

    delete input;
    delete original;
    delete fft_engine;

    if(err/norm > tol){return 1;} //test failed
    return 0; //test passed
}


int main(int /*argc*/, char** /*argv*/)
{
    // set this up to do several multidimensional FFTs of various sizes
    std::vector<std::size_t> dim_sizes;
    std::vector<std::size_t> axes;
    
    dim_sizes.clear();
    axes.clear();
    dim_sizes.insert(dim_sizes.end(), { 8 });
    axes.insert(axes.end(), { 0 });
    int val0 = run_test<ARRAY1_TYPE, FFT1_TYPE>(dim_sizes, axes);

    dim_sizes.clear();
    axes.clear();
    dim_sizes.insert(dim_sizes.end(), { 1024 });
    axes.insert(axes.end(), { 0 });
    int val1 = run_test<ARRAY1_TYPE, FFT1_TYPE>(dim_sizes, axes);
    
    dim_sizes.clear();
    axes.clear();
    dim_sizes.insert(dim_sizes.end(), { 512, 19 });
    axes.insert(axes.end(), { 0 });
    int val2 = run_test<ARRAY2_TYPE, FFT2_TYPE>(dim_sizes, axes);
    
    dim_sizes.clear();
    axes.clear();
    dim_sizes.insert(dim_sizes.end(), { 79, 16, 128 });
    axes.insert(axes.end(), { 0, 2 });
    int val3 = run_test<ARRAY3_TYPE, FFT3_TYPE>(dim_sizes, axes);
    
    dim_sizes.clear();
    axes.clear();
    dim_sizes.insert(dim_sizes.end(), { 32, 64, 30, 160 });
    axes.insert(axes.end(), { 1, 3 });
    int val4 = run_test<ARRAY4_TYPE, FFT4_TYPE>(dim_sizes, axes);

    return val0 + val1 + val2 + val3 + val4;
}
