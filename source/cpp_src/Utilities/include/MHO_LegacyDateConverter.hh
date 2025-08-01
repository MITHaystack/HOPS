#ifndef MHO_LegacyDateConverter_HH__
#define MHO_LegacyDateConverter_HH__

#include "legacy_hops_date.hh"
#include <string>

namespace hops
{

/*!
 *@file MHO_LegacyDateConverter.hh
 *@class MHO_LegacyDateConverter
 *@date Fri Aug 18 11:08:08 2023 -0400
 *@brief this class is necessary to prevent a collision between the "namespace date"
 *and the legacy HOPS3 "struct date"
 *@author J. Barrett - barrettj@mit.edu
 */

class MHO_LegacyDateConverter
{
    public:
        MHO_LegacyDateConverter(){};
        virtual ~MHO_LegacyDateConverter(){};

        /**
         * @brief Converts a legacy date to ISO8601 format.
         *
         * @param a_date Input legacy date.
         * @return ISO8601 formatted string representation of the input date.
         * @note This is a static function.
         */
        static std::string ConvertToISO8601Format(legacy_hops_date a_date);
        /**
         * @brief Converts a legacy date to VEX format.
         *
         * @param a_date Input legacy date
         * @return Date in VEX format as string
         * @note This is a static function.
         */
        static std::string ConvertToVexFormat(legacy_hops_date a_date);

        /**
         * @brief Converts a VEX date string to legacy_hops_date.
         *
         * @param vex_date Input VEX date string.
         * @return legacy_hops_date converted from input vex_date.
         * @note This is a static function.
         */
        static legacy_hops_date ConvertFromVexFormat(std::string vex_date);
        /**
         * @brief Converts ISO8601 date string to legacy_hops_date format.
         *
         * @param iso_date Input date string in ISO8601 format
         * @return legacy_hops_date representation of the input ISO8601 date
         * @note This is a static function.
         */
        static legacy_hops_date ConvertFromISO8601Format(std::string iso_date);
        /**
         * @brief Returns current date-time as legacy_hops_date.
         *
         * @return Current date-time in legacy_hops_date format.
         * @note This is a static function.
         */
        static legacy_hops_date Now();
        /**
         * @brief Calculates and returns the legacy date for the current Hops epoch.
         *
         * @return legacy_hops_date representing the current Hops epoch in legacy format.
         * @note This is a static function.
         */
        static legacy_hops_date HopsEpoch();
};

} // namespace hops

#endif /*! end of include guard: MHO_LegacyDateConverter_HH__ */
