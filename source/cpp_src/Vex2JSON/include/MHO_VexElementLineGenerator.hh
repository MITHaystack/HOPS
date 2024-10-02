#ifndef MHO_VexElementLineGenerator_HH__
#define MHO_VexElementLineGenerator_HH__

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_Message.hh"
#include "MHO_VexDefinitions.hh"
#include <sstream>
#include <string>

namespace hops
{

/*!
 *@file  MHO_VexElementLineGenerator.hh
 *@class  MHO_VexElementLineGenerator
 *@author  J. Barrett - barrettj@mit.edu
 *@date Fri Jun 17 10:45:26 2022 -0400
 *@brief
 */

class MHO_VexElementLineGenerator
{
    public:
        MHO_VexElementLineGenerator();
        virtual ~MHO_VexElementLineGenerator();

        std::string ConstructElementLine(std::string element_name, mho_json& element, mho_json& format);

        std::string GenerateInt(std::string element_name, mho_json& obj);
        std::string GenerateListInt(std::string element_name, mho_json& obj);
        std::string GenerateReal(std::string element_name, mho_json& obj);
        std::string GenerateListReal(std::string element_name, mho_json& obj);
        std::string GenerateKeyword(std::string element_name, mho_json& obj);
        std::string GenerateString(std::string element_name, mho_json& obj);
        std::string GenerateListString(std::string element_name, mho_json& obj);
        std::string GenerateEpoch(std::string element_name, mho_json& obj);
        std::string GenerateRaDec(std::string element_name, mho_json& obj);
        std::string GenerateLink(std::string element_name, mho_json& obj);
        std::string GenerateCompound(std::string element_name, mho_json& element, mho_json& format);

    private:
        std::string fSpace;

        bool IsTrailingOptionalField(std::string field_name, mho_json& fields);
};

} // namespace hops

#endif /*! end of include guard: MHO_VexElementLineGenerator */
