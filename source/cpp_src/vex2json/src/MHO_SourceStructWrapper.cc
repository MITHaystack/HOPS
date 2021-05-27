#include "MHO_SourceStructWrapper.hh"
#include "MHO_SkyCoordStructWrapper.hh"
#include "MHO_DateStructWrapper.hh"

#include <string>
#include <iostream>

namespace hops{

void
MHO_SourceStructWrapper::DumpToJSON(json& json_obj)
{
    json_obj["source_name"] = std::string(fSourceStruct.source_name);
    json_obj["iau_name"] = std::string(fSourceStruct.iau_name);
    json_obj["source_type"] = fSourceStruct.source_type;
    json_obj["calibrator"] = fSourceStruct.calibrator;

    MHO_SkyCoordStructWrapper position(fSourceStruct.position);
    position.DumpToJSON(json_obj["position"]);

    MHO_DateStructWrapper epoch(fSourceStruct.position_epoch);
    epoch.DumpToJSON(json_obj["position_epoch"]);

    json_obj["position_ref_frame"] = fSourceStruct.position_ref_frame;

    if(!is_float_unset(fSourceStruct.ra_rate)){json_obj["ra_rate"] = fSourceStruct.ra_rate;}
    if(!is_float_unset(fSourceStruct.dec_rate)){json_obj["dec_rate"] = fSourceStruct.dec_rate;}
}

}
