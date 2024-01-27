#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <utility>
#include <map>
#include <getopt.h>

#include "MHO_Tokenizer.hh"
#include "MHO_MK4VexInterface.hh"
#include "MHO_MK4CorelInterface.hh"
#include "MHO_MK4FringeExport.hh"


using namespace hops;


int main(int argc, char** argv)
{

    char text[] = R"(
        {
        "cmdline": {
          "ap_per_seg": 0,
          "args": [
            "ffit",
            "-c",
            "./cf_test4a",
            "-b",
            "GE",
            "-P",
            "YY",
            "./1111/105-1800/"
          ],
          "baseline": "GE",
          "control_file": "./cf_test4a",
          "directory": "./1111/105-1800/",
          "first_plot_channel": 0,
          "frequency_group": "",
          "message_level": -1,
          "nplot_channels": 0,
          "output_file": "fdump.json",
          "polprod": "YY",
          "positional_args": [
            "./1111/105-1800/"
          ],
          "set_string": "",
          "show_plot": false,
          "test_mode": false
        },
        "config": {
          "ap_period": 1.0,
          "baseline": "GE",
          "channel_bandwidth": 32.0,
          "frt_offset": 15.0,
          "nchannels": 32,
          "nlags": 256,
          "polprod": "YY",
          "polprod_set": [
            "YY"
          ],
          "total_naps": 27
        },
        "control": {
          "config": {
            "mbd_anchor": "sbd",
            "pc_amp_hcode": 0.001,
            "ref_freq": 6000.0,
            "weak_channel": 0.1
          },
          "fit": {
            "dr_win": [
              -5e-06,
              5e-06
            ],
            "ion_npts": 1,
            "ion_win": [
              -0.0,
              0.0
            ]
          },
          "selection": {
            "stop": -3
          },
          "station": {
            "E": {
              "ionosphere": 0.0,
              "sampler_delay_x": [
                -20.0,
                -20.0,
                -20.0,
                -20.0
              ],
              "sampler_delay_y": [
                -20.0,
                -20.0,
                -20.0,
                -20.0
              ]
            },
            "G": {
              "ionosphere": 0.0,
              "sampler_delay_x": [
                -140.0,
                180.0,
                180.0,
                180.0
              ],
              "sampler_delay_y": [
                -140.0,
                180.0,
                180.0,
                180.0
              ]
            },
            "pc_mode": "multitone",
            "samplers": [
              "abcdefgh",
              "ijklmnop",
              "qrstuvwx",
              "yzABCDEF"
            ]
          }
        },
        "files": {
          "baseline_input_file": "/home/barrettj/work/projects/hops-git/build/test_data/vt9105/1111/105-1800/GE.3319AR.cor",
          "control_file": "./cf_test4a",
          "directory": "./1111/105-1800/",
          "output_file": "fdump.json",
          "ref_station_input_file": "/home/barrettj/work/projects/hops-git/build/test_data/vt9105/1111/105-1800/G.3319AR.sta",
          "rem_station_input_file": "/home/barrettj/work/projects/hops-git/build/test_data/vt9105/1111/105-1800/E.3319AR.sta",
          "root_file": "0016+731.3319AR.json"
        },
        "fringe": {
          "ambiguity": 0.03125,
          "aphase": -159.16134881973267,
          "average_frequency": 6456.399999999998,
          "coarse_search_max_amp": 4.220078499615456,
          "dr_win": [
            -5e-06,
            5e-06
          ],
          "drate": 1.4666666666666668e-07,
          "drate_error": 3.995003043617274e-08,
          "famp": 4.240562311573398,
          "frate": 0.00088,
          "freqrms_amp": 34.534840303518486,
          "freqrms_phase": 19.415486229892316,
          "frequency_spacing": 32.0,
          "frequency_spread": 2612.364446244053,
          "integration_time": 26.98268200457096,
          "legacy_frt_timestamp": "180015.00",
          "legacy_start_timestamp": "180000.00",
          "legacy_stop_timestamp": "180027.00",
          "max_dr_bin": 32,
          "max_mbd_bin": 559,
          "max_sbd_bin": 256,
          "mb_win": [
            -0.015625,
            0.015594482421875
          ],
          "mbd_error": 7.151674528526199e-07,
          "mbdelay": 0.001431306640625,
          "n_dr_points": 27,
          "n_drsp_points": 64,
          "n_frequency_points": 1024.0,
          "n_mbd_points": 1024,
          "n_sbd_points": 512,
          "phase_delay": -426.76577226655814,
          "phase_delay_error": 6.227593423143681e-07,
          "phase_error": 1.3451601793990349,
          "prob_false_detect": 0.0,
          "quality_code": "7",
          "raw_resid_phase": -68.93441672232294,
          "raw_resid_phase_rad": -1.2031325397463697,
          "relative_clock_offset": 96.58727777,
          "relative_clock_rate": 1.238e-06,
          "resid_ph_delay": -3.191408181589016e-05,
          "resid_phase": -68.93441672232275,
          "sb_win": [
            -2.0,
            1.9921875
          ],
          "sbd_error": 0.0002022470290831245,
          "sbd_separation": 0.0078125,
          "sbdelay": 0.001378,
          "snr": 85.18803989377658,
          "start_frequency": 3032.4,
          "theory_freqrms_amp": 6.640432455713356,
          "theory_freqrms_phase": 3.8046875385406818,
          "theory_timerms_amp": 3.1057778936616627,
          "theory_timerms_phase": 1.7794796541184383,
          "timerms_amp": 3.9051201988565047,
          "timerms_phase": 2.0952967894655803,
          "tot_phase": -228.0957655420554,
          "total_drate": 0.015468500316436918,
          "total_mbdelay": -426.7643090458357,
          "total_sbdelay": -426.7643623524763,
          "total_summed_weights": 863.4458241462708,
          "year_doy": "2019:105"
        },
        "model": {
          "aaccel": -2.351127226975069e-06,
          "adelay": -426.7657403524763,
          "arate": 0.01546835364977025
        },
        "ref_station": {
          "antenna_ref": "GGAO12M",
          "azimuth": 345.9565304132007,
          "clock_early_offset": -82.225,
          "clock_early_offset_units": "usec",
          "clock_offset_at_frt": -82.20531448,
          "clock_origin": "2019y106d00h00m0s",
          "clock_rate": -9.12e-13,
          "clock_ref": "Gs",
          "clock_validity": "2019y105d00h00m",
          "elevation": 52.285488532496586,
          "mk4id": "G",
          "parallactic_angle": 138.3187286377507,
          "position": {
            "x": {
              "units": "m",
              "value": 1130730.245
            },
            "y": {
              "units": "m",
              "value": -4831245.953
            },
            "z": {
              "units": "m",
              "value": 3994228.228
            }
          },
          "sample_bits": 0,
          "sample_rate": {
            "units": "Ms/sec",
            "value": 64.0
          },
          "site_id": "Gs",
          "site_name": "GGAO12M",
          "site_ref": "GGAO12M",
          "site_type": "fixed",
          "u": -2604423.1118670977,
          "v": 2920830.227377143,
          "w": -5026448.234686273
        },
        "rem_station": {
          "antenna_ref": "WESTFORD",
          "azimuth": 342.88706845583266,
          "clock_early_offset": 14.389,
          "clock_early_offset_units": "usec",
          "clock_offset_at_frt": 14.38196329,
          "clock_origin": "2019y106d00h00m0s",
          "clock_rate": 3.26e-13,
          "clock_ref": "Wf",
          "clock_validity": "2019y105d00h00m",
          "elevation": 54.66278220069358,
          "mk4id": "E",
          "parallactic_angle": 130.20672971383217,
          "position": {
            "x": {
              "units": "m",
              "value": 1492206.6
            },
            "y": {
              "units": "m",
              "value": -4458130.507
            },
            "z": {
              "units": "m",
              "value": 4296015.532
            }
          },
          "sample_bits": 0,
          "sample_rate": {
            "units": "Ms/sec",
            "value": 64.0
          },
          "site_id": "Wf",
          "site_name": "WESTFORD",
          "site_ref": "WESTFORD",
          "site_type": "fixed",
          "u": -2828864.7445050916,
          "v": 2386099.7998840506,
          "w": -5183345.403434753
        },
        "start_offset": 0.0,
        "status": {
          "is_finished": true,
          "skipped": false
        },
        "stop_offset": 27.0,
        "uuid": {
          "ref_coord": "2b43c27339934397b78d19598df9edb9",
          "ref_pcal": "8d38c139cdaa49b48f9e4a2fc40a59f4",
          "rem_coord": "ff2d84711a374cf597d8b68cb23af9b5",
          "rem_pcal": "0d83534e3e0146da839b2b5ab2fc0ccb",
          "visibilities": "2c7650f73f254e50aae0f44b3022cecd",
          "weights": "22883b5507924b3c87cf2c071b762316"
        },
        "vex": {
          "experiment_name": "VT9105",
          "experiment_number": "9999",
          "scan": {
            "fourfit_reftime": "2019y105d18h00m15s",
            "mode": "VGOS",
            "name": "105-1800",
            "sample_period": {
              "units": "s",
              "value": 1.5625e-08
            },
            "sample_rate": {
              "units": "Hz",
              "value": 64000000.0
            },
            "source": {
              "dec": "73d27'30.0174\"",
              "name": "0016+731",
              "ra": "00h19m45.78642s"
            },
            "start": "2019y105d18h00m00s"
          }
        }
      }
    )";
    
    mho_json params = mho_json::parse(text);

    MHO_ParameterStore pStore;
    pStore.FillData(params);
    pStore.Dump();


    char text2[] = R"(
        {
          "AP": 1.0,
          "Amp": 4.240562311573398,
          "AprioriAccel": -2.351127226975069e-06,
          "AprioriClock": 96.58727777,
          "AprioriClockrate": 1.238e-06,
          "AprioriDelay": -426.7657403524763,
          "AprioriRate": 0.01546835364977025,
          "BuildTime": "-",
          "CorrTime": "-",
          "CorrVers": "HOPS4/DiFX fourfit rev 4.0",
          "Dec": "73d27'30.0174\"",
          "DelayRate": 0.015468500316436918,
          "ExperName": "VT9105",
          "ExperNum": "9999",
          "FFTime": "-",
          "FRT": "180015.00",
          "FringeRate": 0.00088,
          "GroupDelaySBD": -426.7643090458357,
          "IntgTime": 26.98268200457096,
          "IonTEC": "-",
          "NPlots": 33,
          "NSeg": 7,
          "PFD": 0.0,
          "PhaseDelay": -426.76577226655814,
          "PolStr": "GGAO12M - WESTFORD, fgroup ?, pol YY",
          "Quality": "7",
          "RA": "00h19m45.78642s",
          "RefFreq": 6000.0,
          "ResPhase": -68.93441672232275,
          "ResidMbd": 0.001431306640625,
          "ResidMbdelay": 0.001431306640625,
          "ResidMbdelayError": 7.151674528526199e-07,
          "ResidPhase": -68.93441672232275,
          "ResidPhaseError": 1.3451601793990349,
          "ResidPhdelay": -3.191408181589016e-05,
          "ResidPhdelayError": 6.227593423143681e-07,
          "ResidRate": 1.4666666666666668e-07,
          "ResidRateError": 3.995003043617274e-08,
          "ResidSbd": 0.001378,
          "ResidSbdelay": 0.001378,
          "ResidSbdelayError": 0.0002022470290831245,
          "RootScanBaseline": "0016+731.3319AR.json, 105-1800, GE",
          "SNR": 85.18803989377658,
          "SbandDelay": -426.7643623524763,
          "Start": "180000.00",
          "StartPlot": 0,
          "Stop": "180027.00",
          "TotalPhase": -228.0957655420554,
          "YearDOY": "2019:105",
          "extra": {
            "ambiguity": 0.03125,
            "baseline_input_file": "/home/barrettj/work/projects/hops-git/build/test_data/vt9105/1111/105-1800/GE.3319AR.cor",
            "coarse_search_max_amp": 4.220078499615456,
            "control_file": "./cf_test4a",
            "data_rate": 0,
            "dr_win": [
              -0.005,
              0.005
            ],
            "freqrms_amp": 34.534840303518486,
            "freqrms_phase": 19.415486229892316,
            "grid_pts": 1024,
            "inc_avg_amp": 4.231201267323538,
            "inc_avg_amp_freq": 4.444880481811597,
            "ion_win": [
              0.0,
              0.0
            ],
            "mb_win": [
              -0.015625,
              0.015594482421875
            ],
            "n_dr_points": 27,
            "n_drsp_points": 64,
            "n_mbd_points": 1024,
            "n_sbd_points": 512,
            "nlags": 256,
            "output_file": "fdump.json",
            "pol_product": "YY",
            "ref_station": {
              "al": 52.285488532496586,
              "az": 345.9565304132007,
              "pa": 138.3187286377507,
              "u": -2604423.1118670977,
              "v": 2920830.227377143,
              "w": -5026448.234686273
            },
            "ref_station_input_file": "/home/barrettj/work/projects/hops-git/build/test_data/vt9105/1111/105-1800/G.3319AR.sta",
            "ref_station_mk4id": "G",
            "ref_station_sample_bits": 0,
            "rem_station": {
              "al": 54.66278220069358,
              "az": 342.88706845583266,
              "pa": 130.20672971383217,
              "u": -2828864.7445050916,
              "v": 2386099.7998840506,
              "w": -5183345.403434753
            },
            "rem_station_input_file": "/home/barrettj/work/projects/hops-git/build/test_data/vt9105/1111/105-1800/E.3319AR.sta",
            "rem_station_mk4id": "E",
            "rem_station_sample_bits": 0,
            "sample_rate": 64.0,
            "sb_win": [
              -2.0,
              1.9921875
            ],
            "theory_freqrms_amp": 6.640432455713356,
            "theory_freqrms_phase": 3.8046875385406818,
            "theory_timerms_amp": 3.1057778936616627,
            "theory_timerms_phase": 1.7794796541184383,
            "timerms_amp": 3.9051201988565047,
            "timerms_phase": 2.0952967894655803,
            "u": -21.77754151905981,
            "v": -51.88482167663629
          }
        }
    )";

    mho_json plot_data = mho_json::parse(text2);
    
    std::cout<<plot_data.dump(4)<<std::endl;

    MHO_MK4FringeExport fexporter;
    fexporter.SetParameterStore(&pStore);
    fexporter.SetPlotData(plot_data);

    fexporter.SetFilename("/home/barrettj/work/projects/hops-git/build/GE.X.1.ABCDEF");
    fexporter.ExportFringeFile();

    // std::string usage = "TestMK4FringeExport -r <root_filename> -f <corel_filename>";
    // 
    // std::string root_filename;
    // std::string corel_filename;
    // 
    // static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
    //                                       {"root (vex) file", required_argument, 0, 'r'},
    //                                       {"corel file", required_argument, 0, 'f'}};
    // 
    // static const char* optString = "hr:f:";
    // 
    // while(true)
    // {
    //     char optId = getopt_long(argc, argv, optString, longOptions, NULL);
    //     if (optId == -1)
    //         break;
    //     switch(optId)
    //     {
    //         case ('h'):  // help
    //             std::cout << usage << std::endl;
    //             return 0;
    //         case ('r'):
    //             root_filename = std::string(optarg);
    //             break;
    //         case ('f'):
    //             corel_filename = std::string(optarg);
    //             break;
    //         default:
    //             std::cout << usage << std::endl;
    //             return 1;
    //     }
    // }
    // 
    // MHO_Message::GetInstance().AcceptAllKeys();
    // MHO_Message::GetInstance().SetMessageLevel(eDebug);
    // 
    // MHO_MK4CorelInterface mk4inter;
    // 
    // mk4inter.SetCorelFile(corel_filename);
    // mk4inter.SetVexFile(root_filename);
    // mk4inter.ExtractCorelFile();
    // 
    // uch_visibility_store_type* bl_data = mk4inter.GetExtractedVisibilities();
    // uch_weight_store_type* bl_wdata = mk4inter.GetExtractedWeights(); 
    // 
    // if(bl_data == nullptr)
    // {
    //     msg_fatal("main", "Failed to extract mk4corel data." << eom);
    //     std::exit(1);
    // }

    return 0;
}
