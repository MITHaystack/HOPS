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

using namespace hops;

using elem_type = std::complex<double>;
using ax_type = MHO_Axis<double>;
using ax_pack = MHO_AxisPack< ax_type >;
using data_type = MHO_TableContainer< elem_type, ax_pack >;



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

    data_type noise_samples; noise_samples.Resize(n_samples);
    data_type tone_samples; tone_samples.Resize(n_samples);
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
        tone_samples(i) = value;
        std::get<0>(tone_samples)(i) = time;
    }

    for(std::size_t i=0; i<n_samples; i++)
    {
        std::cout<<noise_samples(i)<<std::endl;
    }

    std::cout<<"--------------"<<std::endl;

    for(std::size_t i=0; i<n_samples; i++)
    {
        std::cout<<tone_samples(i)<<std::endl;
    }
    
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
    c->Divide(1,2);

    MHO_RootGraphManager gMan;
    auto gr = gMan.GenerateComplexGraph1D(noise_samples, std::get<0>(noise_samples), 0);
    auto gg = gMan.GenerateComplexGraph1D(tone_samples, std::get<0>(tone_samples), 0 );

    c->cd(1);
    gr->Draw("APL");
    c->Update();
    c->cd(2);
    gg->Draw("APL");
    c->Update();
    // c->cd(3);
    // gb->Draw("PCOL");
    // c->Update();
    
    App->Run();
    
    #endif


}
