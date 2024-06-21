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
*@brief extract useful information from .cor, .frng. and root files for afile generation
*/

class MHO_AFileInfoExtractor
{

    public:
        MHO_AFileInfoExtractor(){};
        virtual ~MHO_AFileInfoExtractor(){};

    public:

        // static mho_json summarize_root_file(std::string filename);
        // static mho_json summarize_corel_file(std::string filename);
        // static mho_json summarize_station_file(std::string filename);
        static mho_json summarize_fringe_file(std::string filename);

        // static void extract_clock_early(const mho_json& clk, double& clock_early, std::string& clock_early_units, double& clock_rate, std::string& clock_rate_units, std::string& origin, std::string& validity);
        // static void extract_clock_model(const mho_json& vexInfo, MHO_ParameterStore* paramStore);
        // static void extract_sampler_bits(const mho_json& vexInfo, MHO_ParameterStore* paramStore);
        // static void extract_sample_rate(const mho_json& vexInfo, MHO_ParameterStore* paramStore);
        // static void extract_vex_info(const mho_json& vexInfo, MHO_ParameterStore* paramStore);

};

}//end namespace

#endif /*! end of include guard: MHO_AFileInfoExtractor_HH__ */
