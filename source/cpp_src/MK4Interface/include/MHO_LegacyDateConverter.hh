#ifndef MHO_LegacyDateConverter_HH__
#define MHO_LegacyDateConverter_HH__

#include <string>
#include "legacy_hops_date.hh"

namespace hops 
{

class MHO_LegacyDateConverter
{
    public:
        MHO_LegacyDateConverter(){};
        virtual ~MHO_LegacyDateConverter(){};

        static std::string ConvertToISO8601Format(legacy_hops_date a_date);

};


}

#endif /* end of include guard: MHO_LegacyDateConverter_HH__ */
