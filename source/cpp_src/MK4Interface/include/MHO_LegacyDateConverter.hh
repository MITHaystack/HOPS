#ifndef MHO_LegacyDateConverter_HH__
#define MHO_LegacyDateConverter_HH__

#include <string>
#include "legacy_hops_date.hh"

namespace hops
{

//this class is necessary to prevent a collision between the "namespace date"
//and the legacy HOPS3 "struct date"

class MHO_LegacyDateConverter
{
    public:
        MHO_LegacyDateConverter(){};
        virtual ~MHO_LegacyDateConverter(){};

        static std::string ConvertToISO8601Format(legacy_hops_date a_date);
        static std::string ConvertToVexFormat(legacy_hops_date a_date);

        static legacy_hops_date ConvertFromVexFormat(std::string vex_date);
        static legacy_hops_date Now();
        static legacy_hops_date HopsEpoch();

};


}

#endif /*! end of include guard: MHO_LegacyDateConverter_HH__ */
