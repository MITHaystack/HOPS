#ifndef MHO_VexInfoExtractor_HH__
#define MHO_VexInfoExtractor_HH__

/*!
*@file MHO_VexInfoExtractor.hh
*@class MHO_VexInfoExtractor
*@author
*Email:
*@date Tue Sep 19 04:11:24 PM EDT 2023
*@brief extract useful information from the vex file and place in parameter store
*/

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

class MHO_VexInfoExtractor
{

    public:
        MHO_VexInfoExtractor(){};
        virtual ~MHO_VexInfoExtractor(){};

    public:

        static void extract_clock_early(const mho_json& clk, double& clock_early, std::string& clock_early_units, double& clock_rate, std::string& clock_rate_units, std::string& origin, std::string& validity);
        static void extract_clock_model(const mho_json& vexInfo, MHO_ParameterStore* paramStore);
        static void extract_sampler_bits(const mho_json& vexInfo, MHO_ParameterStore* paramStore);
        static void extract_sample_rate(const mho_json& vexInfo, MHO_ParameterStore* paramStore);
        static void extract_vex_info(const mho_json& vexInfo, MHO_ParameterStore* paramStore);

};

}//end namespace

#endif /*! end of include guard: MHO_VexInfoExtractor_HH__ */
