#ifndef MHO_AFileInfoExtractor_HH__
#define MHO_AFileInfoExtractor_HH__

#include <cmath>
#include <complex>
#include <iomanip>
#include <sstream>
#include <string>

#include "MHO_Clock.hh"
#include "MHO_Message.hh"

#include "MHO_JSONHeaderWrapper.hh"
#include "MHO_ParameterStore.hh"

namespace hops
{

/*!
 *@file MHO_AFileInfoExtractor.hh
 *@class MHO_AFileInfoExtractor
 *@author
 *Email:
 *@date Wed Sep 20 16:12:23 2023 -0400
 *@brief extract useful inpformation from .cor, .frng. and root files for afile generation
 */

enum par_type
{
    int_type,
    int64_type,
    double_type,
    string_type,
    bool_type,
    unknown_type
};

/**
 * @brief Class MHO_AFileInfoExtractor
 */
class MHO_AFileInfoExtractor
{

    public:
        MHO_AFileInfoExtractor(){};
        virtual ~MHO_AFileInfoExtractor(){};

    public:
        /**
         * @brief Summarizes a fringe file and populates the json object fsum with its data.
         *
         * @param filename The path to the fringe file to summarize.
         * @param fsum (mho_json&)
         * @return True if the file is successfully summarized, false otherwise.
         */
        bool SummarizeFringeFile(std::string filename, mho_json& fsum);

        //version 5 or 6 only
        /**
         * @brief Converts mho_json data to alist row string for versions 5 or 6.
         *
         * @param data Input mho_json data object
         * @param version ALIST format version (5 or 6)
         * @return Generated ALIST row string
         */
        std::string ConvertToAlistRow(const mho_json& data, int version);

        /**
         * @brief Getter for alist header (row text)
         *
         * @param version Version number for which to retrieve the header
         * @param type Type of header to retrieve
         * @param comment_char Comment character used in the header
         * @return Alist header as a string
         */
        std::string GetAlistHeader(int version, int type, char comment_char);

    protected:
        /**
         * @brief Retrieves a parameter from the store and populates it into a json object.
         *
         * @param obj Reference to an mho_json object where the retrieved parameter will be stored
         * @param name Name of the parameter to retrieve
         * @param paramStore Constant reference to the MHO_ParameterStore containing the parameters
         * @param path Path to the parameter in the store
         * @param type Type of the parameter
         */
        void RetrieveParameter(mho_json& obj, const std::string& name, const MHO_ParameterStore& paramStore,
                               const std::string& path, const std::string& type);

        /**
         * @brief Retrieve and convert a parameter from a json object to string based on its type.
         *
         * @param obj Input json object containing the parameter
         * @param name Name of the parameter to retrieve
         * @param type Type of the parameter (int, int64, double, string, bool)
         * @param pformat Format for converting numeric types to string
         * @return Parameter value as a string in specified format
         */
        std::string RetrieveParameterAsString(const mho_json& obj, const std::string& name, const std::string& type,
                                              const std::string& pformat);

        /**
         * @brief Determines parameter type based on input string.
         *
         * @param etype Input string representing parameter type.
         * @return par_type enum value corresponding to the determined parameter type.
         */
        par_type DetermineParameterType(std::string etype);

        /**
         * @brief Converts a value to string using specified format or default precision.
         *
         * @param value Input value of type XValueType
         * @param pformat Optional format string for value conversion
         * @return String representation of the input value
         */
        template< typename XValueType > std::string ConvertToString(XValueType value, const std::string& pformat)
        {
            std::string output;
            std::stringstream ss;
            char tmp[80] = {0};
            if(pformat != "")
            {
                snprintf(tmp, sizeof(tmp), pformat.c_str(), value);
                ss << tmp;
                output = ss.str();
            }
            else
            {
                ss << std::setprecision(15); //default to full double precision
                ss << value;
                output = ss.str();
            }
            return output;
        }
};

} // namespace hops

#endif /*! end of include guard: MHO_AFileInfoExtractor_HH__ */
