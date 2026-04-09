#ifndef MHO_Mk4InputConverter_HH__
#define MHO_Mk4InputConverter_HH__

#include "hops_version.hh"

//utilities
#include "MHO_Message.hh"
#include "MHO_Profiler.hh"
#include "MHO_DirectoryInterface.hh"
#include "MHO_Tokenizer.hh"

//data/config passing classes
#include "MHO_ContainerStore.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_ParameterStore.hh"

namespace hops
{

/*!
 *@file MHO_Mk4InputConverter.hh
 *@class MHO_Mk4InputConverter
 *@author J. Barrettj - barrettj@mit.edu
 *@date Wed Sep 20 16:40:35 2023 -0400 Tue Sep 19 04:11:24 PM EDT 2023
 *@brief collection of helper functions for fringe fitter start-up
 */

/**
 * @brief Class MHO_Mk4InputConverter
 */
class MHO_Mk4InputConverter
{

    public:
        MHO_Mk4InputConverter(){};
        virtual ~MHO_Mk4InputConverter(){};

    public:
        /**
         * @brief Converts a mark4 scan or experiment directory to HOPS format in a temporary directory,
         *        then updates '/cmdline/directory' in paramStore to point at the converted output.
         *
         * @param paramStore Pointer to MHO_ParameterStore containing '/cmdline/directory'
         * @return Path of the created temp directory (caller is responsible for cleanup), or empty string on failure.
         * @note This is a static function. Uses POSIX mkdtemp; requires a Linux/POSIX system.
         */
        static std::string convert_mk4_input(MHO_ParameterStore* paramStore);

        /**
         * @brief Recursively removes the temporary directory created by convert_mk4_input.
         *
         * @param temp_dir Path returned by convert_mk4_input. No-op if empty.
         * @note This is a static function. Uses POSIX nftw; requires a Linux/POSIX system.
         */
        static void cleanup_mk4_temp_dir(const std::string& temp_dir);

};


//RAII guard for the temp directory created by -K conversion.
//to be declared static so its destructor is called even when std::exit() is used.
//makes sure the tmp files are cleaned up if they exist
struct MK4TempDirGuard
{
    std::string path;
    ~MK4TempDirGuard() { MHO_Mk4InputConverter::cleanup_mk4_temp_dir(path); }
};

} // namespace hops

#endif /*! end of include guard: MHO_Mk4InputConverter_HH__ */
