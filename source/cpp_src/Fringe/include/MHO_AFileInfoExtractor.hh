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

class MHO_AFileInfoExtractor
{

    public:
        MHO_AFileInfoExtractor(){};
        virtual ~MHO_AFileInfoExtractor(){};

    public:
        // static mho_json summarize_root_file(std::string filename);
        // static mho_json summarize_corel_file(std::string filename);
        // static mho_json summarize_station_file(std::string filename);
        bool SummarizeFringeFile(std::string filename, mho_json& fsum);

        //version 5 or 6 only
        std::string ConvertToAlistRow(const mho_json& data, int version);

        std::string GetAlistHeader(int version, int type, char comment_char);

    protected:
        //retrieve from the parameter store
        void RetrieveParameter(mho_json& obj, const std::string& name, const MHO_ParameterStore& paramStore,
                               const std::string& path, const std::string& type);

        //retrieve from a json object, convert to string
        std::string RetrieveParameterAsString(const mho_json& obj, const std::string& name, const std::string& type,
                                              const std::string& pformat);

        par_type DetermineParameterType(std::string etype);

        //convert a type to a string using the specified pformat
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
