#ifndef MHO_VexHelpers_HH__
#define MHO_VexHelpers_HH__

#include <map>
#include <string>

#include "MHO_JSONHeaderWrapper.hh"

namespace hops
{

/*!
 *@file  MHO_VexHelpers.hh
 *@author  J. Barrett - barrettj@mit.edu
 *@date Fri May 22 03:56:07 PM EDT 2026
 *@brief Small free-function helpers for the recurring (O)VEX traversal patterns:
 *  - per-station table-name resolution under $MODE.$FREQ / $MODE.$BBC / $MODE.$IF
 *  - the chained BBC->IF->polarization lookup that channel naming needs
 *  - station -> mk4_site_ID resolution via $STATION -> $SITE
 *
 * These wrap the vex mho_json structure, they only exist to keep the
 * all of mental deeply-nested literal-string traversal in one place.
 */
namespace MHO_VexHelpers
{

//build map: 2-char station_code -> keyword (table name) for the $MODE.<section> entries
//whose qualifiers list a station code present in $STATION. Section entries with no
//qualifiers (shared/fallback tables) are NOT inserted, callers handle that explicitly
//via FirstKeywordInModeSection().
inline std::map< std::string, std::string > StationToKeywordMap(const mho_json& vex_root, const std::string& mode_name,
                                                                const std::string& section)
{
    std::map< std::string, std::string > out;
    if(!vex_root.contains("$MODE") || !vex_root["$MODE"].contains(mode_name))
    {
        return out;
    }
    if(!vex_root["$MODE"][mode_name].contains(section))
    {
        return out;
    }
    for(auto& entry : vex_root["$MODE"][mode_name][section])
    {
        std::string keyword = entry["keyword"].get< std::string >();
        for(auto& qual : entry["qualifiers"])
        {
            std::string station_code = qual.get< std::string >();
            if(vex_root["$STATION"].contains(station_code))
            {
                out[station_code] = keyword;
            }
        }
    }
    return out;
}

//return the first entry's keyword in $MODE.<section> (the conventional shared-table
//fallback when no station-qualified entry exists). 
//Returns "" if missing/empty.
inline std::string FirstKeywordInModeSection(const mho_json& vex_root, const std::string& mode_name,
                                             const std::string& section)
{
    if(!vex_root.contains("$MODE") || !vex_root["$MODE"].contains(mode_name))
    {
        return "";
    }
    const auto& mode = vex_root["$MODE"][mode_name];
    if(!mode.contains(section) || mode[section].empty())
    {
        return "";
    }
    return mode[section][0]["keyword"].get< std::string >();
}

//resolve a per-station table name: prefer the station-qualified entry, fall back to
//the first entry in the section.
//returns "" if neither is available.
inline std::string TableForStation(const mho_json& vex_root, const std::string& mode_name, const std::string& section,
                                   const std::string& station_code,
                                   const std::map< std::string, std::string >* prebuilt_map = nullptr)
{
    if(prebuilt_map)
    {
        auto it = prebuilt_map->find(station_code);
        if(it != prebuilt_map->end())
            return it->second;
    }
    else
    {
        auto m = StationToKeywordMap(vex_root, mode_name, section);
        auto it = m.find(station_code);
        if(it != m.end())
            return it->second;
    }
    return FirstKeywordInModeSection(vex_root, mode_name, section);
}

//traverse the ridiculous the BBC -> IF -> polarization chain for a single chan_def.bbc_id link
//returns "-" (nonsense) when any link in the chain can't be resolved
inline std::string ResolvePolarization(const mho_json& vex_root, const std::string& bbc_table, const std::string& if_table,
                                       const std::string& bbc_id)
{
    if(bbc_table.empty() || if_table.empty())
    {
        return "-";
    }
    if(!vex_root.contains("$BBC") || !vex_root["$BBC"].contains(bbc_table))
    {
        return "-";
    }
    if(!vex_root.contains("$IF") || !vex_root["$IF"].contains(if_table))
    {
        return "-";
    }

    const auto& bbc_assigns = vex_root["$BBC"][bbc_table]["BBC_assign"];
    for(const auto& ba : bbc_assigns)
    {
        if(ba["logical_bbc_id"].get< std::string >() != bbc_id){continue;}
        std::string if_id = ba["logical_if"].get< std::string >();
        const auto& if_defs = vex_root["$IF"][if_table]["if_def"];
        for(const auto& ifd : if_defs)
        {
            if(ifd["if_id"].get< std::string >() == if_id)
            {
                return ifd["polarization"].get< std::string >();
            }
        }
        return "-"; //matched BBC but no IF def
    }
    return "-";
}

//resolve the mk4 1-char site_ID for a given 2-char vex station code (via $STATION -> $SITE link) 
//returns "" when any link is missing.
inline std::string StationMk4SiteId(const mho_json& vex_root, const std::string& station_code)
{
    if(!vex_root.contains("$STATION") || !vex_root["$STATION"].contains(station_code))
    {
        return "";
    }
    const auto& site_refs = vex_root["$STATION"][station_code]["$SITE"];
    if(site_refs.empty())
    {
        return "";
    }
    std::string site_key = site_refs[0]["keyword"].get< std::string >();
    if(!vex_root.contains("$SITE") || !vex_root["$SITE"].contains(site_key))
    {
        return "";
    }
    if(!vex_root["$SITE"][site_key].contains("mk4_site_ID"))
    {
        return "";
    }
    return vex_root["$SITE"][site_key]["mk4_site_ID"].get< std::string >();
}

} // namespace MHO_VexHelpers

} // namespace hops

#endif /*! end of include guard: MHO_VexHelpers */
