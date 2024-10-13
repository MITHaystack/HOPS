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
 *@brief extract useful information from .cor, .frng. and root files for afile generation
 */

class MHO_AFileDefinitions
{

    public:
        MHO_AFileDefinitions(){};
        virtual ~MHO_AFileDefinitions(){};

    public:
        static std::string GetFormatDirectory(const std::string& file_type);
        static std::vector< std::string > GetKeywordNames(const std::string& file_type);
        static mho_json GetAFileFormat(const std::string& file_type);
};

} // namespace hops

#endif /*! end of include guard: MHO_AFileDefinitions_HH__ */
