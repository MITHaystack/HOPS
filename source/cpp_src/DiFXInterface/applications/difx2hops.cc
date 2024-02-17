#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <set>
#include <utility>
#include <map>

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"
#include "MHO_StationCodeMap.hh"

#include "MHO_ContainerDefinitions.hh"
#include "MHO_VisibilityChannelizer.hh"
#include "MHO_WeightChannelizer.hh"

#include "MHO_BinaryFileStreamer.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_ClassIdentityMap.hh"

#include "MHO_DirectoryInterface.hh"
#include "MHO_DiFXInterface.hh"

//option parsing and help text library
#include "CLI11.hpp"

using namespace hops;


//TODO add frequency band labelling

// fprintf (stderr, "  -b <code> <flo> <fhi>     Override freq band codes\n");
//   fprintf (stderr, "                            (can have multiple triplets)\n");
//   fprintf (stderr, "  -s or --scode <file>      Specify new VEX to mk4 station code mappings\n");
//   fprintf (stderr, "                            via a file with lines of the form:   X Xx\n");
//   fprintf (stderr, "  -g <freq-groups>          include data only from these freq groups\n");
//   fprintf (stderr, "  -w <bandwidth in MHz>     include data only for this bandwidth\n");
//   fprintf (stderr, "\n");
//
//   return 0;
//   }
//
//                                   // global table of frequency bands
// struct fbands fband[MAX_FBANDS] = {{'B',      0.0, 999999.9},  // default to band B
//                                  {'I',    100.0,    150.0},
//                                  {'G',    150.0,    225.0},
//                                  {'P',    225.0,    390.0},
//                                  {'L',    390.0,   1750.0}, // ITU: 1400-1722.2, EVN/arrays: 1150-1750
//                                  {'S',   1750.0,   3900.0},
//                                  {'C',   3900.0,   6200.0},
//                                  {'X',   6200.0,  10900.0},
//                                  {'K',  10900.0,  36000.0},
//                                  {'Q',  36000.0,  46000.0},
//                                  {'V',  46000.0,  56000.0},
//                                  {'W',  56000.0, 100000.0},
//                                  {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0},
//                                  {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};



int main(int argc, char** argv)
{

    std::string input_dir = "";
    std::string output_dir = "";
    int message_level = 0;
    std::string station_codes_file = "";
    int exper_num = 1234;
    bool raw_mode = false;
    bool preserve = false;
    std::vector< std::tuple<std::string, double, double> > freq_bands;
    std::vector< std::string > freq_groups;
    double bandwidth = 0;


    CLI::App app{"difx2hops"};

    app.add_option("input_dir,-i,--input-dir", input_dir, "name of the input directory (difx) file to be converted")->required();
    app.add_option("output_dir,-o,--output-dir", output_dir, "name of the output directory where results (hops) will be written (if unspecified defaults to <input_dir>/<experiment>)");
    app.add_option("-m,--message-level", message_level, "message level to be used, range: -2 (debug) to 5 (silent)");
    app.add_option("-s,--scode", station_codes_file, "name of the file containing the 2-char station codes to 1-char mk4 station IDs in form: X Xx");
    app.add_option("-e,--exp-num", exper_num, "experiment identification number");
    app.add_option("-r,--raw-mode", raw_mode, "enable raw mode (do not apply auto-corr normalization)");
    app.add_option("-p,--preserve-difx-names", preserve, "use original difx scan names to name each scans (otherwise uses DOY-HHMM");
    app.add_option("-b,--band",freq_bands,"override frequency band codes, must pass triplet as -b <code> <freq_low> <freq_high> (in MHz)");
    app.add_option("-g,--freq-groups",freq_groups,"include data only from the specified frequency groups");
    app.add_option("-w,--bandwidth",bandwidth,"include data only channels matching this bandwidth (in MHz)");

    CLI11_PARSE(app, argc, argv);

    // for(auto it = freq_bands.begin(); it != freq_bands.end(); it++)
    // {
    //     std::cout<<std::get<0>(*it)<<", "<<std::get<1>(*it)<<", "<<std::get<2>(*it)<<std::endl;
    // }

    //clamp message level
    if(message_level > 5){message_level = 5;}
    if(message_level < -2){message_level = -2;}
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    //if no output directory was specified assume we are going to dump the
    //converted data into <input_directory>/<exper_num>
    if(output_dir == "")
    {
        std::stringstream ss;
        ss << exper_num;
        output_dir = input_dir + "/" + ss.str();
    }

    //if the output directory doesn't exist, then create it
    MHO_DirectoryInterface dirInterface;
    std::string output_directory = dirInterface.GetDirectoryFullPath(output_dir);
    bool ok = dirInterface.DoesDirectoryExist(output_directory);
    if(!ok){ ok = dirInterface.CreateDirectory(output_directory);}
    if(!ok)
    {
        msg_fatal("difx_interface", "Could not locate or create output directory: "<< output_directory << eom);
        std::exit(1);
    }

    //TODO add option to enable/disable legacy code map (disabled by default)
    MHO_StationCodeMap stcode_map;
    stcode_map.InitializeStationCodes(station_codes_file);

    MHO_DiFXInterface difxInterface;
    difxInterface.SetInputDirectory(input_dir);
    difxInterface.SetOutputDirectory(output_directory);
    difxInterface.SetStationCodes(&stcode_map);
    difxInterface.SetExperimentNumber(exper_num);
    difxInterface.SetNormalizeFalse();
    if(!raw_mode){difxInterface.SetNormalizeTrue();}
    if(preserve){difxInterface.SetPreserveDiFXScanNamesTrue();}

    difxInterface.Initialize();
    difxInterface.ProcessScans();

    return 0;
}
