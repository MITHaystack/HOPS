#include "MHO_DiFXOvexPatcher.hh"

#include "MHO_Clock.hh"
#include "MHO_Message.hh"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <set>
#include <sstream>
#include <vector>

namespace hops
{

void MHO_DiFXOvexPatcher::Patch(mho_json& vex_root, const std::string& mode_name)
{
    //all the OVEX/EVEX/IVEX/LVEX version info is handled explicity in the vex generator

    //add the experiment number
    (*(vex_root["$EXPER"].begin()))["exper_num"] = fExperNum;

    //always replace the target correlator with difx (HOPS3 ovex parser compatibility)
    (*(vex_root["$EXPER"].begin()))["target_correlator"] = "difx";

    //clear the $DAS section because HOPS3 ovex parser can't handle it
    for(auto it = vex_root["$DAS"].begin(); it != vex_root["$DAS"].end(); ++it)
    {
        it->clear();
    }
    //clear station links to the $DAS section as well
    for(auto it = vex_root["$STATION"].begin(); it != vex_root["$STATION"].end(); ++it)
    {
        if(it->contains("$DAS"))
            it->erase("$DAS");
    }

    //stamp mk4_site_id on every $SITE and build the DiFX(uppercase) -> VEX(case-preserving) maps
    fDiFX2VexStationCodes.clear();
    fDiFX2VexStationNames.clear();
    for(auto it = vex_root["$SITE"].begin(); it != vex_root["$SITE"].end(); ++it)
    {
        std::string vex_station_code = (*it)["site_ID"];
        std::string vex_station_name = (*it)["site_name"];
        (*it)["mk4_site_ID"] = fStationCodeMap->GetMk4IdFromStationCode(vex_station_code);
        //Careful!! this will break if there are two stations that only differ by case.
        //Needed because while the DiFX input-file makes all station codes upper-case the
        //vex file does not (and we want to preserve the vex codes) - jpb 02/15/24
        std::string difx_station_code = vex_station_code;
        std::transform(difx_station_code.begin(), difx_station_code.end(), difx_station_code.begin(), ::toupper);
        fDiFX2VexStationCodes[difx_station_code] = vex_station_code;
        fDiFX2VexStationNames[difx_station_code] = vex_station_name;
    }

    //wrap $ANTENNA.axis_offset in {axis_type, offset} struct (HOPS3 ovex parser quirk)
    for(auto it = vex_root["$ANTENNA"].begin(); it != vex_root["$ANTENNA"].end(); ++it)
    {
        mho_json offset = (*it)["axis_offset"];
        mho_json aoff_obj;
        aoff_obj["axis_type"] = "el";
        aoff_obj["offset"] = offset;
        it->erase("axis_offset");
        (*it)["axis_offset"] = aoff_obj;
    }

    //link $STATION entries to their $CLOCK keyword (again..HOPS3 ovex parser)
    for(auto& element : vex_root["$CLOCK"].items())
    {
        std::string key = element.key();
        if(vex_root["$STATION"].contains(key))
        {
            mho_json clock_ref;
            clock_ref["keyword"] = key;
            vex_root["$STATION"][key]["$CLOCK"].push_back(clock_ref);
        }
    }

    //synthesize $TRACKS from datastream quantBits. Assumes all datastreams at an antenna
    //share a bit depth; would need per-channel tracking otherwise.
    std::map< std::string, int > code2bits;
    std::map< int, std::string > id2code;
    if(fInput && fInput->contains("datastream") && fInput->contains("antenna"))
    {
        for(auto it : (*fInput)["datastream"].items())
        {
            if(it.value().contains("antennaId") && it.value().contains("quantBits"))
            {
                int antennaId = it.value()["antennaId"].get< int >();
                int bits = it.value()["quantBits"].get< int >();
                int antId = 0;
                for(auto ait = (*fInput)["antenna"].begin(); ait != (*fInput)["antenna"].end(); ait++)
                {
                    if(antId == antennaId && ait.value().contains("name"))
                    {
                        std::string antCode = ait.value()["name"].get< std::string >();
                        code2bits[antCode] = bits;
                        id2code[antennaId] = antCode;
                        break;
                    }
                    antId++;
                }
            }
        }
    }

    //fake it 'til we make it (just like difx2mark4) -- one trax_<N>bits entry per
    //distinct bit-depth we observed, then re-link the stations to it under $MODE.
    mho_json tracks_section;
    std::set< int > added_trax;
    std::map< std::string, std::vector< std::string > > trax2codes;
    for(auto it = code2bits.begin(); it != code2bits.end(); it++)
    {
        std::stringstream ss;
        ss << "trax_" << it->second << "bits";
        std::string trax_name = ss.str();

        auto iter_bool_pair = added_trax.insert(it->second);
        if(iter_bool_pair.second)
        {
            mho_json trax;
            trax["bits/sample"] = it->second;
            tracks_section[trax_name] = trax;
        }
        trax2codes[trax_name].push_back(fDiFX2VexStationCodes[it->first]);
    }

    if(vex_root.contains("$TRACKS"))
        vex_root["$TRACKS"].update(tracks_section);
    else
        vex_root["$TRACKS"] = tracks_section;

    if(vex_root["$MODE"][mode_name].contains("$TRACKS"))
        vex_root["$MODE"][mode_name]["$TRACKS"].clear();
    for(auto it = trax2codes.begin(); it != trax2codes.end(); it++)
    {
        mho_json trax_obj;
        trax_obj["keyword"] = it->first;
        trax_obj["qualifiers"] = it->second;
        vex_root["$MODE"][mode_name]["$TRACKS"].push_back(trax_obj);
    }

    //keep the original $MODE.$PHASE_CAL_DETECT references intact, they are required
    //by fourfit3 to resolve each station's pcal tone setup
    //this doesn't matter to fourfit4

    //link $GLOBAL.$EOP to the first $EOP (or a dummy) -- pedantic HOPS3 ovex parser
    vex_root["$GLOBAL"]["$EOP"].clear();
    if(vex_root["$EOP"].size() > 0)
    {
        std::string eop_key = vex_root["$EOP"].begin().key();
        mho_json eop_obj;
        eop_obj["keyword"] = eop_key;
        vex_root["$GLOBAL"]["$EOP"].push_back(eop_obj);
    }
    else
    {
        mho_json eop_obj;
        eop_obj["keyword"] = "EOP_DIFX_INPUT";
        vex_root["$GLOBAL"]["$EOP"].push_back(eop_obj);
    }

    //zoom-band $FREQ/$BBC/$IF synthesis (only if any datastream uses zoom bands and the
    //user hasn't asked for a different native bandwidth via -w). fDiFX2VexStationCodes
    //has just been populated above; hand it over.
    if(fZoomBandRebuilder != nullptr)
    {
        fZoomBandRebuilder->SetDiFX2VexStationCodes(fDiFX2VexStationCodes);
        if(fZoomBandRebuilder->HasZoomBands() && fZoomBandRebuilder->WantZoomChannels())
        {
            fZoomBandRebuilder->RebuildFreqSections(vex_root, mode_name);
        }
    }

    //NOTE: traditional mk4 channel naming (MHO_DiFXChannelNameConstructor::AddChannelNames)
    //runs later in MHO_DiFXScanProcessor::FinalizeAndWriteRootFile, but it needs the
    //visibility-side global sky-freq grid first.

    //rewrite chan_def.phase_cal_id entries so they reference a pcal_id defined in
    //$PHASE_CAL_DETECT for the station; keeps fourfit3's do_phase_cal_detect.c happy
    NormalizePhaseCalIds(vex_root, mode_name);

    //boilerplate $EVEX
    mho_json evex_obj;
    evex_obj["corr_exp#"] = fExperNum;
    evex_obj["ovex_file"] = "dummy";
    evex_obj["cvex_file"] = "dummy";
    evex_obj["svex_file"] = "dummy";
    double ap_len = 1.0;
    if(fInput && fInput->contains("config") && !(*fInput)["config"].empty() && (*fInput)["config"][0].contains("tInt"))
    {
        ap_len = (*fInput)["config"][0]["tInt"].get< double >();
    }
    evex_obj["AP_length"]["value"] = ap_len;
    evex_obj["AP_length"]["units"] = "sec";
    evex_obj["speedup_factor"]["value"] = 1.0;
    evex_obj["speedup_factor"]["units"] = "";
    mho_json dummy_obj1;
    mho_json dummy_obj2;
    dummy_obj1["keyword"] = "CDUM";
    dummy_obj2["keyword"] = "SDUM";
    evex_obj["$CORR_CONFIG"].push_back(dummy_obj1);
    evex_obj["$SU_CONFIG"].push_back(dummy_obj2);
    vex_root["$EVEX"]["evex_std"] = evex_obj;

    mho_json corr_obj;
    corr_obj["system_tempo"] = 1.00;
    corr_obj["bocf_period"] = 160000;
    mho_json pbs_obj;
    pbs_obj["keyword"] = "PBS_DUMMY";
    corr_obj["$PBS_INIT"].push_back(pbs_obj);
    vex_root["$CORR_INIT"]["corr_init_std"] = corr_obj;

    mho_json log_obj;
    vex_root["$LOG"]["log_std"] = log_obj;

    mho_json pbs_obj2;
    vex_root["$PBS_INIT"]["PBS_DUMMY"] = pbs_obj2;
}

std::string MHO_DiFXOvexPatcher::ComputeFourfitReftime(const mho_json& scan_obj)
{
    //tries to follow d2m4 method but uses the vex-file specified epoch + hops_clock
    //instead of the DiFX MJD value.

    double latest_start = -1.0;
    double earliest_stop = 1e30;
    for(std::size_t n = 0; n < scan_obj["station"].size(); n++)
    {
        double start = scan_obj["station"][n]["data_good"]["value"].get< double >();
        double stop = scan_obj["station"][n]["data_stop"]["value"].get< double >();
        if(start > latest_start)
            latest_start = start;
        if(stop < earliest_stop)
            earliest_stop = stop;
    }

    //truncate midpoint to integer second -- this is how difx2mark4 does it
    int itime = (latest_start + earliest_stop) / 2;
    std::string start_epoch = scan_obj["start"].get< std::string >();
    auto start_tp = hops_clock::from_vex_format(start_epoch);
    auto frt_tp = start_tp + std::chrono::seconds(itime);
    return hops_clock::to_vex_format(frt_tp);
}

void MHO_DiFXOvexPatcher::NormalizePhaseCalIds(mho_json& vex_root, const std::string& mode_name)
{
    //fourfit3 (do_phase_cal_detect.c) requires every pcal_id listed in $PHASE_CAL_DETECT to
    //match at least one chan_def.phase_cal_id in the station's $FREQ table, otherwise it
    //aborts. The source VEX can sometime have chan_defs naming an undefined pcal id (e.g. &L_cal)
    //while the referenced $PHASE_CAL_DETECT def declares a different one (e.g. &U_cal).
    //Rewrite each chan_def.phase_cal_id to a pcal_id that is actually defined for that station.

    if(!vex_root.contains("$MODE") || !vex_root["$MODE"].contains(mode_name))
        return;
    auto& mode = vex_root["$MODE"][mode_name];
    if(!mode.contains("$PHASE_CAL_DETECT") || !mode.contains("$FREQ"))
        return;

    //per-station preferred pcal_id (first one declared in the station's pcal-detect def)
    std::map< std::string, std::string > station_to_pcal_id;
    for(auto& pcd_ref : mode["$PHASE_CAL_DETECT"])
    {
        std::string pcd_name = pcd_ref["keyword"].get< std::string >();
        if(!vex_root.contains("$PHASE_CAL_DETECT") || !vex_root["$PHASE_CAL_DETECT"].contains(pcd_name))
            continue;
        auto& pcd_def = vex_root["$PHASE_CAL_DETECT"][pcd_name];
        if(!pcd_def.contains("phase_cal_detect") || pcd_def["phase_cal_detect"].empty())
            continue;
        std::string preferred = pcd_def["phase_cal_detect"][0]["pcal_id"].get< std::string >();
        for(auto& q : pcd_ref["qualifiers"])
        {
            station_to_pcal_id[q.get< std::string >()] = preferred;
        }
    }

    //rewrite chan_def.phase_cal_id under each freq table (avoid double-work when stations share a table)
    std::set< std::string > rewritten;
    for(auto& freq_ref : mode["$FREQ"])
    {
        std::string freq_name = freq_ref["keyword"].get< std::string >();
        if(rewritten.count(freq_name))
            continue;
        std::string preferred_pcal_id;
        for(auto& q : freq_ref["qualifiers"])
        {
            std::string st = q.get< std::string >();
            if(station_to_pcal_id.count(st))
            {
                preferred_pcal_id = station_to_pcal_id[st];
                break;
            }
        }
        if(preferred_pcal_id.empty())
            continue;
        if(!vex_root.contains("$FREQ") || !vex_root["$FREQ"].contains(freq_name))
            continue;
        auto& freq_def = vex_root["$FREQ"][freq_name];
        if(!freq_def.contains("chan_def"))
            continue;

        std::size_t rewrites = 0;
        for(auto& cd : freq_def["chan_def"])
        {
            std::string existing;
            if(cd.contains("phase_cal_id"))
                existing = cd["phase_cal_id"].get< std::string >();
            if(existing != preferred_pcal_id)
            {
                cd["phase_cal_id"] = preferred_pcal_id;
                rewrites++;
            }
        }
        rewritten.insert(freq_name);
        if(rewrites > 0)
        {
            msg_debug("difx_interface", "normalized phase_cal_id to " << preferred_pcal_id << " for " << rewrites
                                                                      << " chan_defs in $FREQ." << freq_name << eom);
        }
    }
}

} // namespace hops
