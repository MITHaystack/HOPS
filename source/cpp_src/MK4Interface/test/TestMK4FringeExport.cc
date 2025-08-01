#include <algorithm>
#include <getopt.h>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "MHO_MK4CorelInterface.hh"
#include "MHO_MK4FringeExport.hh"
#include "MHO_MK4VexInterface.hh"
#include "MHO_Tokenizer.hh"

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
          "start_date": "2019y105d18h00m00s",
          "stop_date": "2019y105d18h00m27s",
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
          "PLOT_INFO": {
              "#Ch": [
                "a",
                "b",
                "c",
                "d",
                "e",
                "f",
                "g",
                "h",
                "i",
                "j",
                "k",
                "l",
                "m",
                "n",
                "o",
                "p",
                "q",
                "r",
                "s",
                "t",
                "u",
                "v",
                "w",
                "x",
                "y",
                "z",
                "A",
                "B",
                "C",
                "D",
                "E",
                "F",
                "All"
              ],
              "APsRf": [
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0
              ],
              "APsRm": [
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27,
                27
              ],
              "Ampl": [
                2.5253476876793197,
                3.240087969159678,
                2.8327097137122594,
                2.854307633333067,
                2.2366146162621368,
                2.745545575813742,
                2.994580535676276,
                3.2174645136850435,
                4.07945397945962,
                3.880172958698322,
                4.043479878711809,
                4.473972281640002,
                4.333549092108446,
                3.688935003624114,
                3.5741128373141864,
                3.201818516552665,
                6.124343243825467,
                6.680379959614295,
                7.2962721615506405,
                7.37880014641625,
                6.486166750223997,
                6.8038109105184645,
                6.371106619453965,
                6.034887072796819,
                4.835481941177453,
                4.9425478475702125,
                4.97216854657421,
                3.831435160405084,
                4.046285614312341,
                4.613779297075361,
                4.171687478300026,
                4.0356995538490565,
                4.230275480294504
              ],
              "ChIdRf": [
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-"
              ],
              "ChIdRm": [
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-"
              ],
              "Freq": [
                3032.4,
                3064.4,
                3096.4,
                3224.4,
                3320.4,
                3384.4,
                3448.4,
                3480.4,
                5272.4,
                5304.4,
                5336.4,
                5464.4,
                5560.4,
                5624.4,
                5688.4,
                5720.4,
                6392.4,
                6424.4,
                6456.4,
                6584.4,
                6680.4,
                6744.4,
                6808.4,
                6840.4,
                10232.4,
                10264.4,
                10296.4,
                10424.4,
                10520.4,
                10584.4,
                10648.4,
                10680.4,
                0.0
              ],
              "PCAmpRf": [
                98.62892623335726,
                101.15028226184765,
                98.13774689203208,
                103.39485862245498,
                89.99482677029394,
                98.8257884268168,
                95.38464559749251,
                83.07878808806763,
                101.57137659128156,
                99.9812941833532,
                89.9639947600905,
                92.25302748524429,
                79.76147752701422,
                87.54391982285645,
                87.94735035526983,
                76.8085573976883,
                88.80385532928292,
                90.69391131520231,
                86.45227792596745,
                92.719891036717,
                80.9875445738221,
                86.54323092989578,
                85.21033917394311,
                74.01676909959933,
                33.1667271837885,
                32.15300712640784,
                28.56276060353172,
                28.195470795668268,
                27.391234656612212,
                30.617104091561725,
                29.530709228376544,
                25.353031632976865
              ],
              "PCAmpRm": [
                72.42042786133769,
                73.08540498262565,
                63.80817428869598,
                73.7501191257351,
                89.27314106092345,
                111.21147942699626,
                122.43177822705843,
                103.98638040643772,
                59.482131382005925,
                55.52947433993164,
                51.6136235671573,
                44.19975846395193,
                33.65422869787205,
                24.19345497254863,
                53.47718602663222,
                26.430844508786823,
                35.325537738561,
                41.00949075582627,
                41.458011046324856,
                66.75450488875236,
                66.08163975569522,
                71.69851077099521,
                68.90629906652214,
                61.093172046496285,
                13.322974718942408,
                12.448478307413325,
                12.287136331923854,
                13.662934066223901,
                12.437594852343699,
                12.51609679307199,
                14.694906328895119,
                12.000018754214492
              ],
              "PCOffRf": [
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0
              ],
              "PCOffRm": [
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0,
                0.0
              ],
              "PCPhsRf": [
                -48.0380722468324,
                85.65184418677703,
                -120.73061067169952,
                -127.46040260187603,
                91.92427435802927,
                117.02275630996229,
                134.00619669163814,
                -43.82235226398092,
                -140.93470228839945,
                -178.42699530096706,
                172.06570035273774,
                -142.5177070270253,
                -66.63787372103742,
                -17.899185300281925,
                16.71179878939498,
                29.449833593237404,
                -51.46549397583412,
                -76.4267358677274,
                -70.55629270510498,
                58.35750767656835,
                -161.57592975644118,
                -63.45755534224837,
                29.269759892915484,
                66.94219762598108,
                -10.498550202604825,
                -33.826183744920684,
                -36.383030019789345,
                51.541086470283034,
                173.07487127572432,
                -115.85080400218928,
                -58.547670837115305,
                -33.53213931720273
              ],
              "PCPhsRm": [
                76.4350942028462,
                -87.74063101012526,
                143.3044480220337,
                123.15918956233021,
                -144.37392260858962,
                29.345128472867348,
                -159.18153824816886,
                90.54595908866939,
                -129.30381240955228,
                -83.43879443795961,
                -2.5476711174907067,
                58.68022318767146,
                53.40746114864846,
                -62.3780364920072,
                -175.1514087311383,
                -38.6005599243524,
                133.99280891570518,
                -172.21628716082975,
                -101.08224918711589,
                -89.33244510589121,
                -153.1906311907271,
                56.12188813905398,
                -90.84752282187809,
                -1.714580643672768,
                -169.15932209405835,
                -105.81005663426713,
                -2.567106451739612,
                109.72751205752482,
                136.81061649527223,
                51.647668025002574,
                -47.67400849844152,
                61.05511856881643
              ],
              "PCdlyRf": [
                -135.75962624527213,
                -138.13807776748115,
                -138.6545965982674,
                -141.88777542654782,
                -142.59611582256488,
                -142.14043813734534,
                -141.5346628836642,
                -140.65507468976077,
                192.14460855465595,
                188.9640208103074,
                187.27683641357257,
                185.10465270004394,
                184.57388683743014,
                185.52013587823012,
                185.35700311155824,
                186.40768827815393,
                191.46072283542892,
                187.40290142703915,
                185.88945519668846,
                183.4720988607667,
                182.78963311407568,
                182.8041675593297,
                183.2468576033386,
                184.2870347029594,
                190.96149031455033,
                187.87475492270767,
                186.45871776215822,
                183.26779759110528,
                183.35728724374735,
                184.54631087986027,
                184.4157150025422,
                185.6167837096026
              ],
              "PCdlyRm": [
                -15.32839976818953,
                -19.63993053794818,
                -21.014556442124128,
                -24.560504425437177,
                -24.14875400213084,
                -23.64171277355997,
                -22.45915181884368,
                -21.48542443903423,
                -1.075387076584445,
                -7.222090546831267,
                -8.42272719024551,
                -10.463924179459253,
                -12.325745077008731,
                -9.859511639728904,
                -11.009548675856117,
                -9.90712326844641,
                -4.388085392672469,
                -6.007714341617472,
                -6.98429275249348,
                -9.595735279870647,
                -9.652993309154725,
                -9.619210836272632,
                -9.548614034716165,
                -6.996155904352059,
                -3.9429484260955863,
                -8.603691727840598,
                -9.233000100468434,
                -11.328126235472224,
                -12.123614086792687,
                -12.982347565452038,
                -10.563982598062564,
                -10.706751189645097
              ],
              "Phase": [
                -51.53464846388195,
                -70.55520723934552,
                -63.1130623791761,
                -47.7284229245138,
                -62.13994572029716,
                -84.8739051093766,
                -111.90202085345597,
                -115.26077276874396,
                -79.49973976824616,
                -77.36989180665336,
                -95.45074409237202,
                -92.31905916351526,
                -87.50214730707947,
                -100.73250622610476,
                -80.40977836485402,
                -55.45962760032218,
                -53.615172044045195,
                -51.39836522022288,
                -43.67425085041076,
                -39.80703270792617,
                -53.875392475218966,
                -56.57358941836688,
                -57.124249250745606,
                -63.44917535362049,
                -79.74551691340724,
                -82.70886633207664,
                -78.09192144115852,
                -71.45280700973287,
                -80.52275583268509,
                -71.63279077469745,
                -69.14136737720307,
                -72.01998081972538,
                -68.93796461353986
              ],
              "SbdBox": [
                257.2029914426307,
                257.236360560187,
                257.1277601521503,
                257.029726895346,
                257.1302875037498,
                257.1162342065385,
                256.93858626542226,
                257.31754763607177,
                257.2270964704849,
                256.983072071625,
                256.9084867625016,
                257.17228018345196,
                257.09454826136397,
                256.95771947499657,
                257.41938741455726,
                257.4522251229956,
                257.1045377321725,
                257.1957253436541,
                257.3221305117621,
                257.2776609472599,
                257.35353868684047,
                257.19105205737384,
                257.17033159485777,
                257.4252920956084,
                257.0727080472312,
                257.19235734916725,
                256.79542652756265,
                257.21028580850844,
                256.93625684747974,
                257.1189795481388,
                256.9155860956016,
                257.2216928044274,
                257.176384
              ],
              "TrkRf": [
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-"
              ],
              "TrkRm": [
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-",
                "-"
              ],
              "header": [
                "#Ch",
                "Freq",
                "Phase",
                "Ampl",
                "SbdBox",
                "APsRf",
                "APsRm",
                "PCdlyRf",
                "PCdlyRm",
                "PCPhsRf",
                "PCPhsRm",
                "PCOffRf",
                "PCOffRm",
                "PCAmpRf",
                "PCAmpRm",
                "ChIdRf",
                "TrkRf",
                "ChIdRm",
                "TrkRm"
              ]
            },
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

    std::cout << plot_data.dump(4) << std::endl;

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
