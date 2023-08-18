#include "MHO_LegacyDateConverter.hh"
#include "MHO_Clock.hh"

namespace hops 
{

std::string 
MHO_LegacyDateConverter::ConvertToISO8601Format(legacy_hops_date a_date)
{
    auto mstart = hops_clock::from_legacy_hops_date(a_date);
    return hops_clock::to_iso8601_format(mstart);
}

}
