#ifndef MHO_FringeCommandLineParser_HH__
#define MHO_FringeCommandLineParser_HH__

#include "hops_version.hh"

//utilities
#include "MHO_DirectoryInterface.hh"
#include "MHO_Message.hh"
#include "MHO_Profiler.hh"
#include "MHO_Tokenizer.hh"

//data/config passing classes
#include "MHO_ContainerStore.hh"
#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_OperatorToolbox.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_ScanDataStore.hh"

//initialization
#include "MHO_OperatorBuilderManager.hh"

namespace hops
{

/*!
 *@file MHO_FringeCommandLineParser.hh
 *@class MHO_FringeCommandLineParser
 *@author J. Barrettj - barrettj@mit.edu
 *@date Wed Sep 20 16:40:35 2023 -0400 Tue Sep 19 04:11:24 PM EDT 2023
 *@brief collection of helper functions for fringe fitter start-up
 */

/**
 * @brief Class MHO_FringeCommandLineParser
 */
class MHO_FringeCommandLineParser
{

    public:
        MHO_FringeCommandLineParser(){};
        virtual ~MHO_FringeCommandLineParser(){};

    public:
        /**
         * @brief Parses baseline_freqgrp string into separate baseline and freqgrp strings (expects ':' as separator).
         *
         * @param baseline_freqgrp Input string containing baseline and frequency group information.
         * @param baseline (std::string&)
         * @param freqgrp (std::string&)
         * @note This is a static function.
         */
        static void parse_baseline_freqgrp(std::string baseline_freqgrp, std::string& baseline, std::string& freqgrp);

        /**
         * @brief Parses a vector of strings to extract and concatenate control file syntax after 'set' command.
         *
         * @param arglist Input vector of strings containing commands and arguments
         * @param set_arg_index Output index of 'set' command position in arglist
         * @return Concatenated string of control file syntax after 'set' command
         * @note This is a static function.
         */
        static std::string parse_set_string(const std::vector< std::string >& arglist, int& set_arg_index);

        /**
         * @brief Sanitizes a directory path by ensuring it ends with '/' and exists.
         *
         * @param dir Input directory path to sanitize.
         * @return Sanitized directory path (preserves symlinks).
         * @note This is a static function.
         */
        static std::string sanitize_directory(std::string dir);

        /**
         * @brief Performs a sanity check on command line parameters after parsing.
         *
         * @param paramStore Pointer to MHO_ParameterStore containing parsed command line arguments.
         * @return 0 if parameters are valid, 1 otherwise.
         * @note This is a static function.
         */
        static int sanity_check(MHO_ParameterStore* paramStore);

        /**
         * @brief Parses command line arguments and stores them in a parameter store for later use by fourfit
         *
         * @param argc Number of command line arguments
         * @param argv Array of command line argument strings
         * @param paramStore Pointer to MHO_ParameterStore object to store parsed arguments
         * @return 0 if successful, non-zero otherwise
         * @note This is a static function.
         */
        static int parse_fourfit_command_line(int argc, char** argv, MHO_ParameterStore* paramStore);

        static void initialize_messaging(int message_level, const std::vector< std::string >& message_categories);
};

} // namespace hops

#endif /*! end of include guard: MHO_FringeCommandLineParser_HH__ */
