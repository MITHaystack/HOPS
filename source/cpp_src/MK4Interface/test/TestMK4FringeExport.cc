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
            "total_naps": 30
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
            "average_frequency": 6456.399999999998,
            "frequency_spacing": 32.0,
            "frequency_spread": 2612.364446244053,
            "n_frequency_points": 1024.0,
            "start_frequency": 3032.4,
            "total_summed_weights": 948.035391330719
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
            "is_finished": false,
            "skipped": false
          },
          "stop_offset": 30.0,
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
    
    MHO_MK4FringeExport fexporter;
    fexporter.SetParameterStore(&pStore);

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
