#include "MHO_DateStructWrapper.hh"

namespace hops
{

void
MHO_DateStructWrapper::DumpToJSON(json& json_obj)
{
    json_obj["year"] = fDate.year;
    json_obj["day"] = fDate.day;
    json_obj["hour"] = fDate.hour;
    json_obj["minute"] = fDate.minute;
    json_obj["second"] = fDate.second;
}

}
