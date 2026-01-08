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

/**
 * @brief Class MHO_VexInfoExtractor
 */
class MHO_VexInfoExtractor
{

    public:
        MHO_VexInfoExtractor(){};
        virtual ~MHO_VexInfoExtractor(){};

    public:
        /**
         * @brief Extracts clock early data from a JSON object and populates output parameters.
         *
         * @param clk Input JSON object containing clock data
         * @param clock_early Output: Early clock value
         * @param clock_early_units Output: Units for early clock value
         * @param clock_rate Output: Clock rate value (optional)
         * @param clock_rate_units Output: Units for clock rate value
         * @param origin Output: Origin epoch string
         * @param validity Output: Validity epoch string
         * @note This is a static function.
         */
        static void extract_clock_early(const mho_json& clk, double& clock_early, std::string& clock_early_units,
                                        double& clock_rate, std::string& clock_rate_units, std::string& origin,
                                        std::string& validity);

        /**
         * @brief Extracts clock model information from vexInfo and stores it in paramStore for reference and remote stations.
         *
         * @param vexInfo Input vexInfo containing clock model data
         * @param paramStore Output parameter store to hold extracted clock model information
         * @note This is a static function.
         */
        static void extract_clock_model(const mho_json& vexInfo, MHO_ParameterStore* paramStore);


        static void extract_station_identities(const mho_json& vexInfo);

        /**
         * @brief Extracts and stores N sampler bits used at reference and remote stations from vexInfo.
         *
         * @param vexInfo Input mho_json containing vex information
         * @param paramStore MHO_ParameterStore to store extracted sample bits
         * @note This is a static function.
         */
        static void extract_sampler_bits(const mho_json& vexInfo, MHO_ParameterStore* paramStore);

        /**
         * @brief Extracts sample rates for reference and remote stations from VEX info.
         *
         * @param vexInfo Input VEX info as mho_json object
         * @param paramStore Output parameter store for sample rate values and units
         * @note This is a static function.
         */
        static void extract_sample_rate(const mho_json& vexInfo, MHO_ParameterStore* paramStore);

        /**
         * @brief Extracts VEX info from JSON and stores relevant parameters in MHO_ParameterStore.
         *
         * @param vexInfo Input JSON containing VEX information
         * @param paramStore MHO_ParameterStore object to store extracted parameters
         * @note This is a static function.
         */
        static void extract_vex_info(const mho_json& vexInfo, MHO_ParameterStore* paramStore);

        /**
         * @brief Converts right ascension string to decimal hours.
         *
         * @param ra Input right ascension in 'hh:mm:ss.sss' format.
         * @return Right ascension in decimal hours.
         * @note This is a static function.
         */
        static double calculate_ra_hrs(std::string ra);

        /**
         * @brief Calculates declination in decimal degrees from a string representation containing degrees, minutes, and seconds.
         *
         * @param dec Input declination string in format 'ddd°mm'mm.mmm"'
         * @return Declination in decimal degrees
         * @note This is a static function.
         */
        static double calculate_dec_deg(std::string dec);

        /**
         * @brief Calculates time in seconds since 1980-01-01T00:00:00.0Z for a given start date.
         *
         * @param start_date Input date string in VEX format
         * @return Time in seconds since 1980
         * @note This is a static function.
         */
        static int64_t calculate_start_time_seconds_since_1980(std::string start_date);
};

} // namespace hops

#endif /*! end of include guard: MHO_VexInfoExtractor_HH__ */
