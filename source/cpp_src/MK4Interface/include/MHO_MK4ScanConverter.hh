#ifndef MHO_MK4ScanConverter_HH__
#define MHO_MK4ScanConverter_HH__

#include <algorithm>
#include <getopt.h>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

//needed for listing/navigating files/directories on *nix
#include <dirent.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_MK4CorelInterface.hh"
#include "MHO_MK4StationInterface.hh"
#include "MHO_MK4VexInterface.hh"
#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

#include "MHO_VisibilityChannelizer.hh"
#include "MHO_WeightChannelizer.hh"

#include "MHO_BinaryFileInterface.hh"
#include "MHO_BinaryFileStreamer.hh"
#include "MHO_ClassIdentityMap.hh"

#include "MHO_DirectoryInterface.hh"

//distinguish directory types
#define MK4_SCANDIR 0
#define MK4_EXPDIR 1
#define MK4_UNKNOWNDIR 2

namespace hops
{

/*!
 *@file MHO_MK4ScanConverter.hh
 *@class MHO_MK4ScanConverter
 *@author J. Barrett - barrettj@mit.edu
 *@date Thu Feb 8 13:15:14 2024 -0500
 *@brief Converts an entire Mark4 directory to HOPS4 format (station, corel and ovex data)
 */

/**
 * @brief Class MHO_MK4ScanConverter
 */
class MHO_MK4ScanConverter
{
    public:
        MHO_MK4ScanConverter();
        virtual ~MHO_MK4ScanConverter();

        /**
         * @brief Determines the type of a given directory (scan, experiment, unknown).
         *
         * @param in_dir Input directory path to analyze
         * @return Directory type as an integer (MK4_SCANDIR, MK4_EXPDIR, MK4_UNKNOWNDIR)
         * @note This is a static function.
         */
        static int DetermineDirectoryType(const std::string& in_dir);
        /**
         * @brief Processes scan data from input directory (MK4 format) to output directory (HOPS4 format)
         *
         * @param input_dir Input directory containing scan files
         * @param output_dir Output directory for processed files
         * @note This is a static function.
         */
        static void ProcessScan(const std::string& input_dir, const std::string& output_dir);

    private:
        //convert a corel file
        /**
         * @brief Converts Corel input file to HOPS4 format and writes output.
         *
         * @param root_file Path to root VEX file
         * @param input_file Input Corel file path
         * @param output_file Output file path
         * @note This is a static function.
         */
        static void ConvertCorel(const std::string& root_file, const std::string& input_file, const std::string& output_file);

        //convert a station file
        /**
         * @brief Converts a station input file to an output file HOPS4 format.
         *
         * @param root_file Path to the root VEX file
         * @param input_file Path to the input station file
         * @param output_file Path to the output SWIN file
         * @note This is a static function.
         */
        static void ConvertStation(const std::string& root_file, const std::string& input_file, const std::string& output_file);
};

} // namespace hops

#endif /*! end of include guard: MHO_MK4ScanConverter */
