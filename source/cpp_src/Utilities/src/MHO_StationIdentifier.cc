#include "MHO_StationIdentifier.hh"

namespace hops
{

MHO_StationIdentifier::MHO_StationIdentifier()
{
    fStationIds.clear();
    fCodeSet.clear();
    fCode2Name.clear();
}

MHO_StationIdentifier::~MHO_StationIdentifier() = default;

MHO_StationIdentifier& MHO_StationIdentifier::GetInstance()
{
    static MHO_StationIdentifier instance;
    return instance;
}

} // namespace hops
