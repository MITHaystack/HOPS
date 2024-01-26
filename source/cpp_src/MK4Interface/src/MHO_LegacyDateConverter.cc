#include "MHO_LegacyDateConverter.hh"

//isolate the MHO_Clock.hh include to this .cc file so we don't collide with 
//legacy mk4util "struct date"
#include "MHO_Clock.hh"

namespace hops 
{

std::string 
MHO_LegacyDateConverter::ConvertToISO8601Format(legacy_hops_date a_date)
{
    auto mstart = hops_clock::from_legacy_hops_date(a_date);
    return hops_clock::to_iso8601_format(mstart);
}

std::string 
MHO_LegacyDateConverter::ConvertToVexFormat(legacy_hops_date a_date)
{
    auto mstart = hops_clock::from_legacy_hops_date(a_date);
    return hops_clock::to_vex_format(mstart);
}

legacy_hops_date 
MHO_LegacyDateConverter::ConvertFromVexFormat(std::string vex_date)
{
    auto mstart = hops_clock::from_vex_format(vex_date);
    return hops_clock::to_legacy_hops_date(mstart);
}


legacy_hops_date 
MHO_LegacyDateConverter::Now()
{
    //get the current time
    auto tnow = hops_clock::now();
    std::string tnow_vex = hops_clock::to_vex_format(tnow);
    legacy_hops_date now_date;
    now_date = ConvertFromVexFormat(tnow_vex);
    return now_date;
}

legacy_hops_date 
MHO_LegacyDateConverter::HopsEpoch()
{
    auto epoch_start = hops_clock::get_hops_epoch();
    return hops_clock::to_legacy_hops_date(epoch_start);
}


}//end namespace
