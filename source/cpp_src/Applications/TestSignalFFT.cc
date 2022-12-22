#include <vector>
#include <string>
#include <iostream>
#include <stdint.h>
#include <getopt.h>
#include <complex>

#ifdef USE_ROOT
    #include "TApplication.h"
    #include "MHO_RootCanvasManager.hh"
    #include "MHO_RootGraphManager.hh"
#endif

#include "MHO_SingleToneSignal.hh"
#include "MHO_GaussianWhiteNoiseSignal.hh"
#include "MHO_ContainerDefinitions.hh"


#include "MHO_FastFourierTransform.hh"
#include "MHO_MultidimensionalFastFourierTransform.hh"
#ifdef HOPS_USE_FFTW3
#include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#endif

#include "MHO_CyclicRotator.hh"



using namespace hops;

using elem_type = std::complex<double>;
using ax_type = MHO_Axis<double>;
using ax_pack = MHO_AxisPack< ax_type >;
using data_type = MHO_TableContainer< elem_type, ax_pack >;


#ifdef HOPS_USE_FFTW3
#define FFT_TYPE MHO_MultidimensionalFastFourierTransformFFTW<data_type>
#else
#define FFT_TYPE MHO_MultidimensionalFastFourierTransform<data_type>
#endif



int main(int argc, char** argv)
{
    std::size_t n_samples = 1000;
    double sample_freq = 1e6;

    double mean = 0.0;
    double stddev = 1.0;
    MHO_GaussianWhiteNoiseSignal aNoiseSignal;
    aNoiseSignal.SetRandomSeed(123);
    aNoiseSignal.SetMean(mean);
    aNoiseSignal.SetStandardDeviation(stddev);
    aNoiseSignal.Initialize();

    double tone_freq = 25000.0;
    double phase_offset = 0.0;

    MHO_SingleToneSignal aToneSignal;
    aToneSignal.SetToneFrequency(tone_freq);
    aToneSignal.SetPhaseOffset(phase_offset);

    data_type noise_samples; noise_samples.Resize(n_samples);
    data_type tone_samples; tone_samples.Resize(n_samples);
    data_type sum_samples; sum_samples.Resize(n_samples);
    // std::vector<double> noise_samples; noise_samples.resize(n_samples);
    // std::vector<double> tone_samples; tone_samples.resize(n_samples);

    double sample_period = 1.0/sample_freq;
    double time;
    double value;

    for(std::size_t i=0; i<n_samples; i++)
    {
        time = i*sample_period;
        aNoiseSignal.GetSample(time, value);
        noise_samples(i) = value;
        std::get<0>(noise_samples)(i) = time;
        aToneSignal.GetSample(time, value);
        tone_samples(i) = 8*value;
        std::get<0>(tone_samples)(i) = time;
        sum_samples(i) = noise_samples(i) + tone_samples(i);
        std::get<0>(sum_samples)(i) = time;
    }

    // for(std::size_t i=0; i<n_samples; i++)
    // {
    //     std::cout<<noise_samples(i)<<std::endl;
    // }
    // 
    // std::cout<<"--------------"<<std::endl;
    // 
    // for(std::size_t i=0; i<n_samples; i++)
    // {
    //     std::cout<<tone_samples(i)<<std::endl;
    // }
    
    //now execute an FFT on the samples
    data_type ft_noise_samples = noise_samples; //ft_noise_samples.Resize(n_samples);
    data_type ft_tone_samples = tone_samples; //ft_tone_samples.Resize(n_samples);
    data_type ft_sum_samples = sum_samples; //ft_sum_samples.Resize(n_samples);

    // for(std::size_t i=0; i<n_samples; i++)
    // {
    //     std::get<0>(ft_noise_samples)(i) = i;
    //     std::get<0>(ft_tone_samples)(i) = i;
    //     std::get<0>(ft_sum_samples)(i) = i;
    // }

    bool status = false;

    FFT_TYPE* fft_engine = new FFT_TYPE();
    fft_engine->SetForward();
    fft_engine->SetArgs(&ft_noise_samples);
    status = fft_engine->Initialize();
    status = fft_engine->Execute();

    fft_engine->SetForward();
    fft_engine->SetArgs(&ft_tone_samples);
    status = fft_engine->Initialize();
    status = fft_engine->Execute();

    fft_engine->SetForward();
    fft_engine->SetArgs(&ft_sum_samples);
    status = fft_engine->Initialize();
    status = fft_engine->Execute();


    MHO_CyclicRotator<data_type> aCyclicRotator;
    aCyclicRotator.SetOffset(0, n_samples/2);
    aCyclicRotator.SetArgs(&ft_noise_samples);
    status = aCyclicRotator.Initialize();
    status = aCyclicRotator.Execute();
    
    aCyclicRotator.SetOffset(0, n_samples/2);
    aCyclicRotator.SetArgs(&ft_tone_samples);
    status = aCyclicRotator.Initialize();
    status = aCyclicRotator.Execute();
    
    aCyclicRotator.SetOffset(0, n_samples/2);
    aCyclicRotator.SetArgs(&ft_sum_samples);
    status = aCyclicRotator.Initialize();
    status = aCyclicRotator.Execute();

    #ifdef USE_ROOT
    
    std::cout<<"starting root plotting"<<std::endl;
    
    //ROOT stuff for plots
    int dummy_argc = 0;
    char tmp = '\0';
    char* argv_placeholder = &tmp;
    char** dummy_argv = &argv_placeholder;
    TApplication* App = new TApplication("test",&dummy_argc,dummy_argv);
    
    MHO_RootCanvasManager cMan;
    auto c = cMan.CreateCanvas(std::string("test"), 800, 800);
    c->Divide(1,3);

    MHO_RootGraphManager gMan;
    auto g1 = gMan.GenerateComplexGraph1D(noise_samples, std::get<0>(noise_samples), 0);
    auto g2 = gMan.GenerateComplexGraph1D(tone_samples, std::get<0>(tone_samples), 0 );
    auto g3 = gMan.GenerateComplexGraph1D(sum_samples, std::get<0>(sum_samples), 0 );

    c->cd(1);
    g1->Draw("APL");
    c->Update();
    c->cd(2);
    g2->Draw("APL");
    c->Update();
    c->cd(3);
    g3->Draw("APL");
    c->Update();


    auto c2= cMan.CreateCanvas(std::string("ft_test"), 800, 800);
    c2->Divide(1,3);
    c2->cd(1);

    //plot magnitude squared
    auto f1 = gMan.GenerateComplexGraph1D(ft_noise_samples, std::get<0>(ft_noise_samples), 4);
    auto f2 = gMan.GenerateComplexGraph1D(ft_tone_samples, std::get<0>(ft_tone_samples), 2 );
    auto f3 = gMan.GenerateComplexGraph1D(ft_sum_samples, std::get<0>(ft_sum_samples), 4 );

    c2->cd(1);
    f1->Draw("APL");
    c2->Update();
    c2->cd(2);
    f2->Draw("APL");
    c2->Update();
    c2->cd(3);
    f3->Draw("APL");
    c2->Update();
    
    App->Run();
    
    #endif


}
