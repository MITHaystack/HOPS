#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "MHO_Message.hh"
#include "MHO_StationCodeMap.hh"
#include "MHO_Tokenizer.hh"

#include "MHO_ContainerDefinitions.hh"

#include "MHO_BinaryFileInterface.hh"
#include "MHO_BinaryFileStreamer.hh"
#include "MHO_ClassIdentityMap.hh"

#include "MHO_DiFXInterface.hh"
#include "MHO_DirectoryInterface.hh"

//option parsing and help text library
#include "CLI11.hpp"

using namespace hops;

int main(int argc, char** argv)
{
    std::vector< std::string > input_dirs;
    std::string output_dir = "";
    int message_level = 0;
    std::string station_codes_file = "";
    int exper_num = 1234;
    bool raw_mode = false;
    bool preserve = false;
    std::vector< std::tuple< std::string, double, double > > freq_bands; //user override
    std::vector< std::string > freq_groups;
    bool use_legacy_bands = false;
    bool use_legacy_stcodes = false;
    bool attach_difx_input = false;
    bool export_as_mark4 = false;
    double bandwidth = 0;

    //legacy frequency band set-up
    std::vector< std::tuple< std::string, double, double > > legacy_freq_bands;
    legacy_freq_bands.push_back(std::make_tuple(std::string("B"), 0.0, 999999.9));
    legacy_freq_bands.push_back(std::make_tuple(std::string("I"), 100.0, 150.0));
    legacy_freq_bands.push_back(std::make_tuple(std::string("G"), 150.0, 225.0));
    legacy_freq_bands.push_back(std::make_tuple(std::string("P"), 225.0, 390.0));
    legacy_freq_bands.push_back(std::make_tuple(std::string("L"), 390.0, 1750.0));
    legacy_freq_bands.push_back(std::make_tuple(std::string("S"), 1750.0, 3900.0));
    legacy_freq_bands.push_back(std::make_tuple(std::string("C"), 3900.0, 6200.0));
    legacy_freq_bands.push_back(std::make_tuple(std::string("X"), 6200.0, 10900.0));
    legacy_freq_bands.push_back(std::make_tuple(std::string("K"), 10900.0, 36000.0));
    legacy_freq_bands.push_back(std::make_tuple(std::string("Q"), 36000.0, 46000.0));
    legacy_freq_bands.push_back(std::make_tuple(std::string("V"), 46000.0, 56000.0));
    legacy_freq_bands.push_back(std::make_tuple(std::string("W"), 56000.0, 100000.0));

    std::stringstream ss;
    ss << "force the use of the legacy frequency band assignments (default: off) these are: \n";
    for(auto it = legacy_freq_bands.begin(); it != legacy_freq_bands.end(); it++)
    {
        ss << "    " << std::get< 0 >(*it) << ": (" << std::get< 1 >(*it) << ", " << std::get< 2 >(*it) << ") MHz \n";
    }
    ss << "during assignment, narrower bands will take precedence over wider bands";
    std::string legacy_freq_bands_help = ss.str();

    CLI::App app{"difx2hops"};

    app.add_option("input_dirs,-i,--input-dirs", input_dirs, "name of the input directory or directories to be coverted")
        ->required();
    app.add_option("output_dir,-o,--output-dir", output_dir,
                   "name of the output directory where results (hops) will be written (if unspecified defaults to "
                   "<cwd>/<exp-num>)");
    app.add_option("-m,--message-level", message_level, "message level to be used, range: -2 (debug) to 5 (silent)");
    app.add_option("-s,--scode", station_codes_file,
                   "name of the file containing the 2-char station codes to 1-char mk4 station IDs in form: X Xx");
    app.add_option("-e,--exp-num", exper_num, "experiment identification number");
    app.add_flag("-r,--raw-mode", raw_mode, "enable raw mode (do not apply auto-corr normalization)");
    app.add_flag("-P,--preserve-difx-names", preserve,
                 "use original difx scan names to name each scans (otherwise uses DOY-HHMM");
    app.add_option("-b,--band", freq_bands,
                   "set frequency band codes, must pass triplet as -b <code> <freq_low> <freq_high> (in MHz) if none specified "
                   "and '-L' flag not passed, no band assignment will be made");
    app.add_flag("-L,--legacy-bands", use_legacy_bands, legacy_freq_bands_help.c_str())->excludes("-b");
    app.add_flag("-C,--legacy-station-codes", use_legacy_stcodes,
                 "use the legacy station code map with assigning mk4 station IDs.");
    app.add_flag("-k,--mark4", export_as_mark4, "export data in the legacy mark4 format instead of hops format.");
    app.add_option("-g,--freq-groups", freq_groups, "include data only from the specified frequency groups")->delimiter(',');
    app.add_option("-w,--bandwidth", bandwidth, "include data only channels matching this bandwidth (in MHz)");
    app.add_flag("-a,--attach-difx-input", attach_difx_input, "attach the DiFX .input data to the visibility object tags");

    CLI11_PARSE(app, argc, argv);

    //clamp message level
    if(message_level > 5)
    {
        message_level = 5;
    }
    if(message_level < -2)
    {
        message_level = -2;
    }
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    if(use_legacy_bands) //enable legacy band assignment behavior
    {
        freq_bands.clear();
        freq_bands = legacy_freq_bands;
    }

    if(freq_bands.size() == 0)
    {
        msg_warn("main",
                 "no frequency band assignments were passed with either the '-b' or '-L' option, no assignments will be made."
                     << eom);
    }
    else
    {

        msg_info("main", "frequency band assignments are: " << eol);
        for(auto it = freq_bands.begin(); it != freq_bands.end();)
        {
            std::stringstream ssb;
            ssb << std::get< 0 >(*it) << ": (" << std::get< 1 >(*it) << ", " << std::get< 2 >(*it) << ") MHz ";
            it++;
            if(it != freq_bands.end())
            {
                msg_info("main", ssb.str() << eol);
            }
            else
            {
                msg_info("main", ssb.str() << eom);
            }
        }
    }

    if(freq_bands.size() == 0 && freq_groups.size() != 0)
    {
        msg_fatal("main", "no frequency band assignments were made, but data is excluded on the basis of frequency group by "
                          "'-g' option, quitting."
                              << eom);
        std::exit(1);
    }

    if(freq_groups.size() != 0)
    {
        std::stringstream ssg;
        ssg << "frequency groups selected are: {";
        for(auto it = freq_groups.begin(); it != freq_groups.end();)
        {
            ssg << *it;
            it++;
            if(it != freq_groups.end())
            {
                ssg << ", ";
            }
        }
        msg_info("main", ssg.str() << "}" << eom);
    }

    //if no output directory was specified assume we are going to dump the
    //converted data into <cwd>/<exper_num>
    if(output_dir == "")
    {
        std::stringstream ss;
        ss << exper_num;
        output_dir = "./" + ss.str();
    }

    //if the output directory doesn't exist, then create it
    MHO_DirectoryInterface dirInterface;
    std::string output_directory = dirInterface.GetDirectoryFullPath(output_dir);
    bool ok = dirInterface.DoesDirectoryExist(output_directory);
    if(!ok)
    {
        ok = dirInterface.CreateDirectory(output_directory);
    }
    if(!ok)
    {
        msg_fatal("difx_interface", "Could not locate or create output directory: " << output_directory << eom);
        std::exit(1);
    }

    MHO_StationCodeMap stcode_map;
    if(use_legacy_stcodes)
    {
        stcode_map.UseLegacyCodes();
    }                                                      //use legacy d2m4 station code map
    stcode_map.InitializeStationCodes(station_codes_file); //if no file passed, auto assignement will take place

    for(std::size_t n = 0; n < input_dirs.size(); n++)
    {
        std::string input_dir = input_dirs[n];

        if(MHO_DirectoryInterface::IsDirectory(input_dir))
        {
            MHO_DiFXInterface difxInterface;
            difxInterface.SetTryLocalDirectoryTrue();
            difxInterface.SetInputDirectory(input_dir);
            difxInterface.SetOutputDirectory(output_directory);
            difxInterface.SetStationCodes(&stcode_map);
            difxInterface.SetExperimentNumber(exper_num);
            difxInterface.SetNormalizeFalse();

            if(!raw_mode)
            {
                difxInterface.SetNormalizeTrue();
            }

            if(preserve)
            {
                difxInterface.SetPreserveDiFXScanNamesTrue();
            }

            if(attach_difx_input)
            {
                difxInterface.SetAttachDiFXInputTrue();
            }
            else
            {
                difxInterface.SetAttachDiFXInputFalse();
            }

            if(bandwidth != 0)
            {
                difxInterface.SetOnlyBandwidth(bandwidth);
            }

            if(freq_bands.size() != 0)
            {
                difxInterface.SetFrequencyBands(freq_bands);
            }

            if(freq_groups.size() != 0)
            {
                difxInterface.SetFreqGroups(freq_groups);
            }

            if(export_as_mark4)
            {
                difxInterface.SetExportAsMark4True();
            }

            difxInterface.Initialize();
            difxInterface.ProcessScans();
        }
    }

    return 0;
}
