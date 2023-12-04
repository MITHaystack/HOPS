#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "MHO_Message.hh"
#include "MHO_Snapshot.hh"

//parse_command_line
#include <getopt.h>


//data/config passing classes
#include "MHO_ParameterStore.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_JSONHeaderWrapper.hh"

//needed to read hops files and extract objects from scan dir
#include "MHO_ScanDataStore.hh"

//initialization
#include "MHO_OperatorBuilderManager.hh"
#include "MHO_ParameterConfigurator.hh"
#include "MHO_ParameterManager.hh"

#include "MHO_VexInfoExtractor.hh"


#ifdef HOPS_USE_FFTW3
#include "MHO_MultidimensionalFastFourierTransformFFTW.hh"
#else
#include "MHO_MultidimensionalFastFourierTransform.hh"
#endif


using namespace hops;


double dwin(double value, double lower, double upper)
{
    if (value < lower) return (lower);
    else if (value > upper) return (upper);
    else return (value);
}

int
parabola (double y[3], double lower, double upper, double* x_max, double* amp_max, double q[3])
{
    int i, rc;
    double x, range;
    //extern double dwin(double, double, double);
    range = std::fabs (upper - lower);

    q[0] = (y[0] - 2 * y[1] + y[2]) / 2;      /* This is trivial to derive,
    	                              or see rjc's 94.1.10 derivation */
    q[1] = (y[2] - y[0]) / 2;
    q[2] = y[1];


    if (q[0] < 0.0)
        x = -q[1] / (2 * q[0]);                      /* x value at maximum y */
    else                                         /* no max, pick higher side */
        x = (y[2] > y[0]) ? 1.0 : -1.0;

    *x_max = dwin (x, lower, upper);

    *amp_max = q[0] * *x_max * *x_max  +  q[1] * *x_max  +  q[2];

    // Test for error conditions

    rc = 0;                         // default: indicates error-free interpolation
    if (q[0] >= 0)                  // 0 or positive curvature is an interpolation error
        rc = 2;
                                    // Is maximum at either edge?
                                    // (simple floating point equality test can fail
                                    // in machine-dependent way)
    else if (std::fabs (*x_max - x) > (0.001 * range))
        rc = 1;

    return (rc);
}


int main(int argc, char** argv)
{
    //TODO allow messaging keys to be set via command line arguments
    MHO_Message::GetInstance().AcceptAllKeys();

    //TODO make this conform/support most of the command line options of fourfit
    std::string usage = "TestPCalExtraction -d <directory> -s <station> -P <polarizaton>";

    std::string directory = "";
    std::string station = "";
    std::string pol = "";
    std::string output_file = "pcal.json"; //for testing
    double lower_freq = -100.0;
    double upper_freq = -100.0;
    int message_level = -1;
    bool ok;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"directory", required_argument, 0, 'd'},
                                          {"station", required_argument, 0, 's'},
                                          {"polarization", required_argument, 0, 'P'},
                                          {"message-level", required_argument, 0, 'm'},
                                          {"lower-frequency", required_argument, 0, 'l'},
                                          {"upper-frequency", required_argument, 0, 'u'},
                                          {"output", required_argument, 0, 'o'}};

    static const char* optString = "hd:s:P:o:m:l:u:";

    while(true)
    {
        char optId = getopt_long(argc, argv, optString, longOptions, NULL);
        if (optId == -1)
            break;
        switch(optId)
        {
            case ('h'):  // help
                std::cout << usage << std::endl;
                std::exit(0);
            case ('d'):
                directory = std::string(optarg);
                break;
            case ('s'):
                station = std::string(optarg);
                break;
            case ('P'):
                pol = std::string(optarg);
                break;
            case ('o'):
                output_file = std::string(optarg);
                break;
            case ('m'):
                message_level = std::atoi(optarg);
                break;
            case ('l'):
                lower_freq = std::atof(optarg);
                break;
            case ('u'):
                upper_freq = std::atof(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    if( directory == "" || station == "" || pol == "" )
    {
        msg_fatal("main", "usage: "<< usage << eom);
        return 1;
    }

    std::cout<<"freq bounds = "<<lower_freq<<", "<<upper_freq<<std::endl;

    if(lower_freq < 0 || upper_freq < 0)
    {
        msg_fatal("main", "lower/upper channel frequency limits must be non-zero and specificied in MHz." << eom );
        return 1;
    }


    //set the message level according to the fourfit style
    //where 3 is least verbose, and '-1' is most verbose
    switch (message_level)
    {
        case -2:
            //NOTE: debug messages must be compiled-in
            #ifndef HOPS_ENABLE_DEBUG_MSG
            MHO_Message::GetInstance().SetMessageLevel(eInfo);
            msg_warn("fringe", "debug messages are toggled via compiler flag, re-compile with ENABLE_DEBUG_MSG=ON to enable." << eom);
            #else
            MHO_Message::GetInstance().SetMessageLevel(eDebug);
            #endif
        break;
        case -1:
            MHO_Message::GetInstance().SetMessageLevel(eInfo);
        break;
        case 0:
            MHO_Message::GetInstance().SetMessageLevel(eStatus);
        break;
        case 1:
            MHO_Message::GetInstance().SetMessageLevel(eWarning);
        break;
        case 2:
            MHO_Message::GetInstance().SetMessageLevel(eError);
        break;
        case 3:
            MHO_Message::GetInstance().SetMessageLevel(eFatal);
        break;
        case 4:
            MHO_Message::GetInstance().SetMessageLevel(eSilent);
        break;
        default:
            //for now default is most verbose, eventually will change this to silent
            MHO_Message::GetInstance().SetMessageLevel(eDebug);
    }

    if(station.size() != 1)
    {
        msg_fatal("main", "station must be passed as 1-char mk4-id code."<< eom);
        return 1;
    }

    //data objects
    MHO_ParameterStore paramStore; //stores various parameters using string keys
    MHO_ScanDataStore scanStore; //provides access to data associated with this scan
    MHO_ContainerStore containerStore; //stores data containers for in-use data


    //initialize the scan store from this directory
    scanStore.SetDirectory(directory);
    scanStore.Initialize();
    if( !scanStore.IsValid() )
    {
        msg_fatal("fringe", "cannot initialize a valid scan store from this directory: " << directory << eom);
        std::exit(1);
    }

    //set the root file name
    paramStore.Set("/files/root_file", scanStore.GetRootFileBasename() );

    //load root file and extract useful vex info
    auto vexInfo = scanStore.GetRootFileData();
    //MHO_VexInfoExtractor::extract_vex_info(vexInfo, &paramStore);

    //load station data
    scanStore.LoadStation(station, &containerStore);

    station_coord_type* sdata = containerStore.GetObject<station_coord_type>(std::string("sta"));
    multitone_pcal_type* pcal_data = containerStore.GetObject<multitone_pcal_type>(std::string("pcal"));

    std::cout<<sdata<<std::endl;
    std::cout<<pcal_data<<std::endl;

    if(sdata == nullptr || pcal_data == nullptr)
    {
        msg_fatal("main", "failed to load station or pcal data" << eom );
        std::exit(1);
    }


    std::size_t rank = pcal_data->GetRank();
    auto dims = pcal_data->GetDimensionArray();

    for(std::size_t i=0; i<rank; i++)
    {
        std::cout<<"pcal dim @"<<i<<" = "<<dims[i]<<std::endl;
    }

    auto tone_freq_ax = std::get<MTPCAL_FREQ_AXIS>(*pcal_data);
    double start_tone_frequency = 0;
    std::size_t start_tone_index = 0;
    std::size_t ntones = 0;
    for(std::size_t j=0; j<tone_freq_ax.GetSize(); j++)
    {
        if( tone_freq_ax(j) < upper_freq && lower_freq <= tone_freq_ax(j) )
        {
            if(ntones == 0)
            {
                start_tone_frequency = tone_freq_ax(j) ;
                start_tone_index = j;
            }
            ntones++;
            std::cout<<"tone: "<<j<<" = "<<tone_freq_ax(j)<<std::endl;
        }
    }
    std::cout<<"start tone = "<<start_tone_frequency<<", start tone index = "<<start_tone_index<<", ntones = "<<ntones<<std::endl;

    //should we channelize the pca-data? yes...but for now just do an FFT on one chunk to test
    //extract the p-cal data between the start/stop indices for this particular pol from the first AP
    using pcal_axis_pack = MHO_AxisPack< frequency_axis_type >;
    using pcal_type = MHO_TableContainer< std::complex<double>, pcal_axis_pack >;

    int FFTSIZE = 256; //default x-form size in pcalibrate
    pcal_type test;
    test.Resize(FFTSIZE);
    test.ZeroArray();

    for(std::size_t j=0; j<ntones; j++)
    {
        std::complex<double> phasor = pcal_data->at(0, 0, start_tone_index+j);
        double tone_freq = tone_freq_ax(start_tone_index+j);
        test.at(j) = phasor;
        std::get<0>(test).at(j) = tone_freq;
    }

    double pc_tone_delta = 1e6*(std::get<0>(test)(1) - std::get<0>(test)(0));
    std::cout<<"PCAL TONE DELTA = "<<pc_tone_delta << std::endl;

    for(std::size_t i=0; i<FFTSIZE; i++)
    {
        std::get<0>(test).at(i) = i*pc_tone_delta; //actual freq is irrelevant
    }

    #ifdef HOPS_USE_FFTW3
    using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransformFFTW< pcal_type >;
    #else
    using FFT_ENGINE_TYPE = MHO_MultidimensionalFastFourierTransform< pcal_type >;
    #endif

    FFT_ENGINE_TYPE fFFTEngine;


    fFFTEngine.SetArgs(&test);
    fFFTEngine.DeselectAllAxes();
    fFFTEngine.SelectAxis(0); //only perform padded fft on frequency (to lag) axis
    fFFTEngine.SetForward();//forward DFT

    ok = fFFTEngine.Initialize();
    ok = fFFTEngine.Execute();

    double max_val = 0;
    int max_idx = 0;
    double max_del = 0;
    for(std::size_t i=0; i<FFTSIZE; i++)
    {
        std::complex<double> phasor = test.at(i);
        double abs_val = std::abs(phasor);
        if(abs_val > max_val)
        {
            max_val = abs_val;
            max_idx = i;
            max_del = std::get<0>(test)(i);
        }
    }

    double delay_delta = (std::get<0>(test)(1) - std::get<0>(test)(0));
    std::cout<<"delay_delta = "<<delay_delta<<std::endl;

    std::cout<<"max, max_idx, max_del = "<<max_val<<", "<<max_idx<<", "<<max_del<<std::endl;

    double ymax, ampmax;
    double y[3];
    double q[3];
    y[1] = max_val;
    y[0] =std::abs (test[(max_idx+FFTSIZE-1)%FFTSIZE]);
    y[2] = std::abs (test[(max_idx+FFTSIZE+1)%FFTSIZE]);
    parabola (y, -1.0, 1.0, &ymax, &ampmax, q);

                        // DC is in 0th element
    double delay = (max_idx+ymax) / 256.0 / pc_tone_delta;

    double delay2 = (max_idx+ymax)*delay_delta;
    std::cout<<"ymax = "<<ymax<<std::endl;
    std::cout<<"DELAY = "<<delay<<std::endl;
    std::cout<<"DELAY2 = "<<delay2<<std::endl;

                        // find corresponding delay in suitable range
    double pc_amb = 1 / pc_tone_delta;


    return 0;
}
