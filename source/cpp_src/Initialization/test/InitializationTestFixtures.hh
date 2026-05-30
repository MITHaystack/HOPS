#ifndef INITIALIZATION_TEST_FIXTURES_HH__
#define INITIALIZATION_TEST_FIXTURES_HH__

// Shared fixtures for the Initialization operator-builder unit tests:
// helpers to populate a container store with fake "vis"/"weight" objects and to
// assemble the minimal format/attribute json that the builders consume.

#include <string>

#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_JSONHeaderWrapper.hh"

namespace hops
{
namespace inittest
{

// add a fake visibility object to the store under the short name "vis"
inline visibility_type* add_vis(MHO_ContainerStore& store, std::size_t np = 1, std::size_t nch = 4, std::size_t nap = 2,
                                std::size_t nsp = 2)
{
    visibility_type* vis = new visibility_type();
    vis->Resize(np, nch, nap, nsp);
    auto& chax = std::get< CHANNEL_AXIS >(*vis);
    for(std::size_t i = 0; i < chax.GetSize(); i++)
    {
        chax.at(i) = 8000.0 + 16.0 * static_cast< double >(i);
    }
    if(std::get< POLPROD_AXIS >(*vis).GetSize() > 0)
    {
        std::get< POLPROD_AXIS >(*vis).at(0) = "XX";
    }
    store.AddObject(vis);
    store.SetShortName(vis->GetObjectUUID(), std::string("vis"));
    return vis;
}

// add a fake weight object to the store under the short name "weight"
inline weight_type* add_weight(MHO_ContainerStore& store, std::size_t np = 1, std::size_t nch = 4, std::size_t nap = 2,
                               std::size_t nsp = 1)
{
    weight_type* wt = new weight_type();
    wt->Resize(np, nch, nap, nsp);
    auto& chax = std::get< CHANNEL_AXIS >(*wt);
    for(std::size_t i = 0; i < chax.GetSize(); i++)
    {
        chax.at(i) = 8000.0 + 16.0 * static_cast< double >(i);
    }
    store.AddObject(wt);
    store.SetShortName(wt->GetObjectUUID(), std::string("weight"));
    return wt;
}

// add a minimal station coordinate object to the store under the given short name
// (e.g. "ref_sta"/"rem_sta"); builders only store the pointer, so contents are
// left default
inline station_coord_type* add_station(MHO_ContainerStore& store, const std::string& shortname)
{
    station_coord_type* sta = new station_coord_type();
    sta->Resize(NCOORD, 1, NCOEFF);
    store.AddObject(sta);
    store.SetShortName(sta->GetObjectUUID(), shortname);
    return sta;
}

// add a minimal multitone pcal object ([pol,time,freq]) to the store under the
// given short name (e.g. "ref_pcal"/"rem_pcal")
inline multitone_pcal_type* add_pcal(MHO_ContainerStore& store, const std::string& shortname)
{
    multitone_pcal_type* pc = new multitone_pcal_type();
    pc->Resize(1, 1, 1);
    std::get< MTPCAL_POL_AXIS >(*pc).at(0) = "X";
    store.AddObject(pc);
    store.SetShortName(pc->GetObjectUUID(), shortname);
    return pc;
}

// minimal operator "format": the builders read fFormat["priority"]
inline mho_json make_format(double priority = 1.0)
{
    mho_json f;
    f["priority"] = priority;
    return f;
}

// minimal operator "attributes": builders read fAttributes["name"] and ["value"]
inline mho_json make_attributes(const std::string& name, const mho_json& value)
{
    mho_json a;
    a["name"] = name;
    a["value"] = value;
    return a;
}

} // namespace inittest
} // namespace hops

#endif
