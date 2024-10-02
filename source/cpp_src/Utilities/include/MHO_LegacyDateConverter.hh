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

        static std::string ConvertToISO8601Format(legacy_hops_date a_date);
        static std::string ConvertToVexFormat(legacy_hops_date a_date);

        static legacy_hops_date ConvertFromVexFormat(std::string vex_date);
        static legacy_hops_date ConvertFromISO8601Format(std::string iso_date);
        static legacy_hops_date Now();
        static legacy_hops_date HopsEpoch();
};

} // namespace hops

#endif /*! end of include guard: MHO_LegacyDateConverter_HH__ */
