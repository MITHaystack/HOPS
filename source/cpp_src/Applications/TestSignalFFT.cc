#include <vector>
#include <string>
#include <iostream>
#include <stdint.h>
#include <getopt.h>
#include <complex>

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
    std::size_t n_samples = 1001;
    double sample_freq = 1e6;

    double mean = 0.0;
    double stddev = 1.0;
    MHO_GaussianWhiteNoiseSignal aNoiseSignal;
    aNoiseSignal.SetRandomSeed(123);
    aNoiseSignal.SetMean(mean);
    aNoiseSignal.SetStandardDeviation(stddev);
    aNoiseSignal.Initialize();

    double tone_freq = 25000.0;
    double tone_freq2 = 3000.0;
    double tone_freq3 = 400000.0;
    double phase_offset = 0.0;

    MHO_SingleToneSignal aToneSignal;
    aToneSignal.SetToneFrequency(tone_freq);
    aToneSignal.SetPhaseOffset(phase_offset);

    MHO_SingleToneSignal aToneSignal2;
    aToneSignal2.SetToneFrequency(tone_freq2);
    aToneSignal2.SetPhaseOffset(phase_offset);

    MHO_SingleToneSignal aToneSignal3;
    aToneSignal3.SetToneFrequency(tone_freq3);
    aToneSignal3.SetPhaseOffset(phase_offset);

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
        tone_samples(i) = 1*value;
        aToneSignal2.GetSample(time, value);
        tone_samples(i) += 2*value;
        aToneSignal3.GetSample(time, value);
        tone_samples(i) += 3*value;
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
    data_type ft_noise_samples; ft_noise_samples.Copy(noise_samples); //ft_noise_samples.Resize(n_samples);
    data_type ft_tone_samples; ft_tone_samples.Copy(tone_samples); //ft_tone_samples.Resize(n_samples);
    data_type ft_sum_samples; ft_sum_samples.Copy(sum_samples); //ft_sum_samples.Resize(n_samples);

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

    return 0;

}
