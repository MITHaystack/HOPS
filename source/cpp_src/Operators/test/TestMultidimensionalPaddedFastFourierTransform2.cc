#include "MHO_Message.hh"
#include "MHO_FastFourierTransform.hh"
#include "MHO_MultidimensionalPaddedFastFourierTransform.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"
#include "MHO_EndZeroPadder.hh"
#include "MHO_TableContainer.hh"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <getopt.h>


using namespace hops;

typedef double FPTYPE;
#define NDIM 2
#define AXIS_PACK_TYPE MHO_AxisPack< MHO_Axis<double>, MHO_Axis<double> >
#define ARRAY_TYPE MHO_TableContainer< std::complex<FPTYPE>, AXIS_PACK_TYPE > 
#define PADDER_TYPE MHO_EndZeroPadder< ARRAY_TYPE >
#define PADDED_FFT_TYPE MHO_MultidimensionalPaddedFastFourierTransform< ARRAY_TYPE >
#define FFT_TYPE MHO_MultidimensionalFastFourierTransform< ARRAY_TYPE >

int main(int argc, char** argv)
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    const size_t ndim = NDIM;
    const size_t dval = 4;
    const size_t pfactor = 4;
    size_t dim_size[ndim];
    size_t dim2_size[ndim];
    for(std::size_t i=0;i<NDIM;i++){dim_size[i] = dval;};
    for(std::size_t i=0;i<NDIM;i++){dim2_size[i] = dval;};
    ARRAY_TYPE* input1 = new ARRAY_TYPE(dim_size);
    ARRAY_TYPE* input2 = new ARRAY_TYPE(dim_size);
    dim2_size[0] *= pfactor;
    ARRAY_TYPE* output1 = new ARRAY_TYPE(dim2_size);
    ARRAY_TYPE* output2 = new ARRAY_TYPE(dim2_size);

    size_t idim_size[NDIM];
    input1->GetDimensions(idim_size);

    for(size_t i=0;i<NDIM;i++)
    {
        std::cout<<" in dim @ "<<i<< " = "<<idim_size[i]<<std::endl;
    }


    std::cout << "--------------------------------------------------------------" << std::endl;

    for(std::size_t i=0; i<input1->GetSize(); i++)
    {
        (*input1)[i] = std::complex<FPTYPE>( i%5, i%17); 
        (*input2)[i] = std::complex<FPTYPE>( i%5, i%17); 
    }

    for(size_t i=0; i<dval; i++)
    {
        (&std::get<0>(*input1) )->at(i) = i;
        (&std::get<1>(*input1) )->at(i) = i;
        (&std::get<0>(*input2) )->at(i) = i;
        (&std::get<1>(*input2) )->at(i) = i;
    }

    
    //compare the results
    for(std::size_t i=0; i<dval; i++)
    {
        std::cout<<std::get<0>(*input1)(i)<<" ? " << std::get<0>(*input2)(i)<<std::endl;
    }

    for(std::size_t i=0; i<dval; i++)
    {
        std::cout<<std::get<1>(*input1)(i)<<" ? " << std::get<1>(*input2)(i)<<std::endl;
    }


    bool init, exe;

    std::cout<<"flag0"<<std::endl;

    //execute the padder followed by an FFT 
    PADDER_TYPE padder;
    padder.DeselectAllAxes();
    padder.SelectAxis(0);
    padder.SetPaddingFactor(pfactor);
    //padder.SetEndPadded();
    padder.SetReverseEndPadded();
    padder.EnableNormFXMode();
    padder.SetArgs(input1, output1);
    init = padder.Initialize();
    exe = padder.Execute();


    //compare the results
    for(std::size_t i=0; i<std::get<0>(*output1).GetSize(); i++)
    {
        std::cout<<std::get<0>(*output1)(i)<<" ? " << std::get<0>(*output2)(i)<<std::endl;
    }

    for(std::size_t i=0; i<std::get<1>(*output1).GetSize(); i++)
    {
        std::cout<<std::get<1>(*output1)(i)<<" ? " << std::get<1>(*output2)(i)<<std::endl;
    }

 

    std::cout<<"flag1"<<std::endl;

    FFT_TYPE fft_engine;
    fft_engine.SetArgs(output1);
    fft_engine.DeselectAllAxes();
    fft_engine.SelectAxis(0); //only perform padded fft on frequency (to lag) axis
    fft_engine.SetForward();//forward DFT
    init = fft_engine.Initialize();
    exe = fft_engine.Execute();

    std::cout<<"flag2"<<std::endl;

    //excute just the padded FFT 
    PADDED_FFT_TYPE pfft_engine;
    pfft_engine.SetArgs(input2, output2);
    pfft_engine.DeselectAllAxes();
    pfft_engine.SelectAxis(0); //only peform on the first axis
    pfft_engine.SetForward();//forward DFT
    pfft_engine.SetPaddingFactor(pfactor);
    //pfft_engine.SetEndPadded(); //for both LSB and USB (what about DSB?)
    pfft_engine.SetReverseEndPadded();
    init = pfft_engine.Initialize();
    exe = pfft_engine.Execute();

    std::cout<<"flag3"<<std::endl;

    //compare the results
    for(std::size_t i=0; i<std::get<0>(*output1).GetSize(); i++)
    {
        std::cout<<(*output1)[i]<<" ? " << (*output2)[i]<<std::endl;
    }

    //compare the results
    for(std::size_t i=0; i<std::get<0>(*output1).GetSize(); i++)
    {
        std::cout<<std::get<0>(*output1)(i)<<" ? " << std::get<0>(*output2)(i)<<std::endl;
    }

    for(std::size_t i=0; i<std::get<1>(*output1).GetSize(); i++)
    {
        std::cout<<std::get<1>(*output1)(i)<<" ? " << std::get<1>(*output2)(i)<<std::endl;
    }



    return 0;
}
