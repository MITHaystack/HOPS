#ifndef MHO_AFileInfoExtractor_HH__
#define MHO_AFileInfoExtractor_HH__


#include <string>
#include <cmath>
#include <complex>
#include <sstream>
#include <iomanip>

#include "MHO_Message.hh"
#include "MHO_Clock.hh"

#include "MHO_ParameterStore.hh"
#include "MHO_JSONHeaderWrapper.hh"

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
        mho_json summarize_fringe_file(std::string filename);


    protected:


        void RetrieveParameter(mho_json& obj, const std::string& name, const MHO_ParameterStore& paramStore, const std::string& path, const std::string& type);

        par_type DetermineParameterType(std::string etype);

        //convert a type to a string using the specified pformat
        template< typename XValueType >
        std::string ConvertToString(XValueType value, const std::string& pformat)
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

}//end namespace

#endif /*! end of include guard: MHO_AFileInfoExtractor_HH__ */
