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
 *@brief Constructs a formatted line for an data element in VEX format - needed to reconstruct VEX files from json representation
 */

/**
 * @brief Class MHO_VexElementLineGenerator
 */
class MHO_VexElementLineGenerator
{
    public:
        MHO_VexElementLineGenerator();
        virtual ~MHO_VexElementLineGenerator();

        /**
         * @brief Constructs a formatted line for an element in VEX format.
         * 
         * @param element_name Name of the element as a string.
         * @param element Reference to the mho_json object containing element data.
         * @param format Reference to the mho_json object containing formatting rules.
         * @return Formatted line for the given element as a string.
         */
        std::string ConstructElementLine(std::string element_name, mho_json& element, mho_json& format);

        /**
         * @brief Generates a string representation of an integer value from a JSON object.
         * 
         * @param element_name Name of the element to retrieve as a string.
         * @param obj (mho_json&)
         * @return String containing the integer value and spaces.
         */
        std::string GenerateInt(std::string element_name, mho_json& obj);
        /**
         * @brief Generates a string list of integers from a given JSON object.
         * 
         * @param element_name Name of the element to search for in the JSON object.
         * @param obj (mho_json&)
         * @return A string containing the list of integers separated by spaces, or an empty string if no matching elements are found.
         */
        std::string GenerateListInt(std::string element_name, mho_json& obj);
        /**
         * @brief Generates a real value string from a JSON object with optional units.
         * 
         * @param element_name Input element name as std::string
         * @param obj JSON object reference containing 'value' and optionally 'units'
         * @return Generated real value string
         */
        std::string GenerateReal(std::string element_name, mho_json& obj);
        /**
         * @brief Generates a formatted string from a JSON object containing real values.
         * 
         * @param element_name Name of the element to generate the list for.
         * @param obj (mho_json&)
         * @return Formatted string representing the list of real values with optional units.
         */
        std::string GenerateListReal(std::string element_name, mho_json& obj);
        /**
         * @brief Generates a keyword string from an element name and a JSON object.
         * 
         * @param element_name Input element name as a string
         * @param obj Reference to a mho_json object containing the keyword value
         * @return Generated keyword string with appropriate spacing
         */
        std::string GenerateKeyword(std::string element_name, mho_json& obj);
        /**
         * @brief Generates a string representation of an object's value with given element name.
         * 
         * @param element_name Name of the element to generate string for.
         * @param obj (mho_json&)
         * @return Generated string representation of the object's value.
         */
        std::string GenerateString(std::string element_name, mho_json& obj);
        /**
         * @brief Generates a string list from a JSON object using given element name.
         * 
         * @param element_name Name of the element to extract from the JSON object.
         * @param obj (mho_json&)
         * @return A string containing the extracted elements separated by space and delimited by ElementDelim().
         */
        std::string GenerateListString(std::string element_name, mho_json& obj);
        /**
         * @brief Generates epoch string for given element name using provided JSON object.
         * 
         * @param element_name Name of the element to generate epoch for
         * @param obj Reference to a mho_json object containing element data
         * @return Generated epoch string as std::string
         */
        std::string GenerateEpoch(std::string element_name, mho_json& obj);
        /**
         * @brief Generates Right Ascension and Declination string for given element name from mho_json object.
         * 
         * @param element_name Name of the element to retrieve data from mho_json object
         * @param obj (mho_json&)
         * @return String containing Right Ascension and Declination values separated by space
         */
        std::string GenerateRaDec(std::string element_name, mho_json& obj);
        /**
         * @brief Generates a link string for an element using its name and associated JSON object.
         * 
         * @param element_name Name of the element to generate a link for
         * @param obj (mho_json&)
         * @return Generated link string as std::string
         */
        std::string GenerateLink(std::string element_name, mho_json& obj);
        /**
         * @brief Generates a compound string from an element and format using specified types.
         * 
         * @param element_name Input element name as std::string
         * @param element Reference to input mho_json object containing element data
         * @param format Reference to input mho_json object containing format data
         * @return Generated compound string
         */
        std::string GenerateCompound(std::string element_name, mho_json& element, mho_json& format);

    private:
        std::string fSpace;

        /**
         * @brief Checks if a field is the last optional field in the given format fields.
         * 
         * @param field_name Name of the field to check.
         * @param fields (mho_json&)
         * @return True if it's the last optional field, false otherwise.
         */
        bool IsTrailingOptionalField(std::string field_name, mho_json& fields);
};

} // namespace hops

#endif /*! end of include guard: MHO_VexElementLineGenerator */
