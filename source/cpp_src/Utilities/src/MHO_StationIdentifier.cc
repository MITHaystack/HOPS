#include "MHO_StationIdentifier.hh"

namespace hops
{
    
    
MHO_StationIdentifier* MHO_StationIdentifier::fStationIdentifier = nullptr;

MHO_StationIdentifier::MHO_StationIdentifier()
{
    fStationIds.clear();
    fCodeSet.clear();
    fCode2Name.clear();
}

MHO_StationIdentifier::~MHO_StationIdentifier() = default;

MHO_StationIdentifier* MHO_StationIdentifier::GetInstance()
{
    //singleton interface construction
    if(fStationIdentifier == nullptr)
    {
        fStationIdentifier = new MHO_StationIdentifier();
    }
    return fStationIdentifier;
}

} // namespace hops
