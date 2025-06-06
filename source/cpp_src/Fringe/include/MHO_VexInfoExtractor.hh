#ifndef MHO_VexInfoExtractor_HH__
#define MHO_VexInfoExtractor_HH__

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
 *@file MHO_VexInfoExtractor.hh
 *@class MHO_VexInfoExtractor
 *@author
 *Email:
 *@date Wed Sep 20 16:12:23 2023 -0400
 *@brief extract useful information from the vex file and place in parameter store
 */

class MHO_VexInfoExtractor
{

    public:
        MHO_VexInfoExtractor(){};
        virtual ~MHO_VexInfoExtractor(){};

    public:
        static void extract_clock_early(const mho_json& clk, double& clock_early, std::string& clock_early_units,
                                        double& clock_rate, std::string& clock_rate_units, std::string& origin,
                                        std::string& validity);
        static void extract_clock_model(const mho_json& vexInfo, MHO_ParameterStore* paramStore);
        static void extract_sampler_bits(const mho_json& vexInfo, MHO_ParameterStore* paramStore);
        static void extract_sample_rate(const mho_json& vexInfo, MHO_ParameterStore* paramStore);
        static void extract_vex_info(const mho_json& vexInfo, MHO_ParameterStore* paramStore);

        static double calculate_ra_hrs(std::string ra);
        static double calculate_dec_deg(std::string dec);

        static int64_t calculate_start_time_seconds_since_1980(std::string start_date);
};

} // namespace hops

#endif /*! end of include guard: MHO_VexInfoExtractor_HH__ */
