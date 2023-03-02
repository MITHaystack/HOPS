#include <vector>
#include <string>
#include <iostream>
#include <stdint.h>
#include <getopt.h>

#include "MHO_SingleToneSignal.hh"
#include "MHO_GaussianWhiteNoiseSignal.hh"

using namespace hops;

int main(int argc, char** argv)
{
    std::size_t n_samples = 100;
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

    std::vector<double> noise_samples; noise_samples.resize(n_samples);
    std::vector<double> tone_samples; tone_samples.resize(n_samples);

    double sample_period = 1.0/sample_freq;
    double time;
    double value;

    for(std::size_t i=0; i<n_samples; i++)
    {
        time = i*sample_period;
        aNoiseSignal.GetSample(time, value);
        noise_samples[i] = value;
        aToneSignal.GetSample(time, value);
        tone_samples[i] = value;
    }

    // for(std::size_t i=0; i<n_samples; i++)
    // {
    //     std::cout<<noise_samples[i]<<std::endl;
    // }
    // 
    // std::cout<<"--------------"<<std::endl;

    // for(std::size_t i=0; i<n_samples; i++)
    // {
    //     std::cout<<tone_samples[i]<<std::endl;
    // }

    return 0;

}
