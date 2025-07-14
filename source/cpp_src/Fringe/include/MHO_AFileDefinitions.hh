#ifndef MHO_AFileDefinitions_HH__
#define MHO_AFileDefinitions_HH__

#include <string>
#include <vector>

#include "MHO_Clock.hh"
#include "MHO_Message.hh"

#include "MHO_ParameterStore.hh"

#include "MHO_DirectoryInterface.hh"
#include "MHO_JSONHeaderWrapper.hh"

namespace hops
{

/*!
 *@file MHO_AFileDefinitions.hh
 *@class MHO_AFileDefinitions
 *@author
 *Email:
 *@date Wed Sep 20 16:12:23 2023 -0400
 *@brief extract useful information from .cor, .frng. and root files for afile (ascii) generation
 */

/**
 * @brief Class MHO_AFileDefinitions
 */
class MHO_AFileDefinitions
{

    public:
        MHO_AFileDefinitions(){};
        virtual ~MHO_AFileDefinitions(){};

    public:
        /**
         * @brief Getter for afile format directory
         * 
         * @param file_type Input file type (root, frng, cor)
         * @return Format directory as string
         * @note This is a static function.
         */
        static std::string GetFormatDirectory(const std::string& file_type);
        
        /**
         * @brief Getter for afile item keyword names
         * 
         * @param file_type Input file type to search for keywords
         * @return Vector of keyword names as strings
         * @note This is a static function.
         */
        static std::vector< std::string > GetKeywordNames(const std::string& file_type);
        
        /**
         * @brief Getter for afile format (as a json object)
         * 
         * @param file_type Input file type to retrieve formats for
         * @return mho_json object containing combined format data
         * @note This is a static function.
         */
        static mho_json GetAFileFormat(const std::string& file_type);
};

} // namespace hops

#endif /*! end of include guard: MHO_AFileDefinitions_HH__ */
