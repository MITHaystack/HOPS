#include "MHO_SkyCoordStructWrapper.hh"

namespace hops
{

void
MHO_SkyCoordStructWrapper::DumpToJSON(json& json_obj)
{
    json_obj["ra_hrs"] = fSkyCoord.ra_hrs;
    json_obj["ra_mins"] = fSkyCoord.ra_mins;
    json_obj["ra_secs"] = fSkyCoord.ra_secs;
    json_obj["dec_degs"] = fSkyCoord.dec_degs;
    json_obj["dec_mins"] = fSkyCoord.dec_mins;
    json_obj["dec_secs"] = fSkyCoord.dec_secs;
}


}
