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
 *@brief
 */

class MHO_MK4ScanConverter
{
    public:
        MHO_MK4ScanConverter();
        virtual ~MHO_MK4ScanConverter();

        static int DetermineDirectoryType(const std::string& in_dir);
        static void ProcessScan(const std::string& input_dir, const std::string& output_dir);

    private:
        //convert a corel file
        static void ConvertCorel(const std::string& root_file, const std::string& input_file, const std::string& output_file);

        //convert a station file
        static void ConvertStation(const std::string& root_file, const std::string& input_file, const std::string& output_file);
};

} // namespace hops

#endif /*! end of include guard: MHO_MK4ScanConverter */
