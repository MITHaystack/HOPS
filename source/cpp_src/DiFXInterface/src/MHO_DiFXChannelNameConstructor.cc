#include "MHO_DiFXChannelNameConstructor.hh"
#include "MHO_MK4ChanId.hh"
#include "MHO_VexHelpers.hh"

#include <algorithm>

namespace hops
{

MHO_DiFXChannelNameConstructor::MHO_DiFXChannelNameConstructor()
{
    fScanID = "";
    fHasGlobalGrid = false;
};

MHO_DiFXChannelNameConstructor::~MHO_DiFXChannelNameConstructor(){};

//add a frequency range for a specific band label
void MHO_DiFXChannelNameConstructor::AddBandLabel(std::string band_label, double freq_low, double freq_high)
{
    //note that we are not checking for overlap or other error conditions
    band_range br;
    br.fLow = freq_low;
    br.fHigh = freq_high;
    br.fLabel = band_label;
    fBandRangeLabels.push_back(br);
}

void MHO_DiFXChannelNameConstructor::AddChannelNames(mho_json& vex_root)
{
    //assume we now have all ovex/vex in the vex_root object, and that we only have a single scan
    //should only have a single 'scan' element under the schedule section, so find it
    auto sched = vex_root["$SCHED"];
    mho_json scan;
    if(sched.size() != 1 && fScanID == "")
    {
        bool found = false;
        //loop over sched, looking for the scan which matches fScanID;
        for(auto it = sched.begin(); it != sched.end(); it++)
        {
            if(it.key() == fScanID)
            {
                scan = it.value();
                found = true;
                break;
            }
        }
        if(!found)
        {
            msg_error("mk4interface", "could not locate scan: " << fScanID << " in schedule, using first scan." << eom);
            scan = sched.begin().value();
        }
    }
    else
    {
        scan = sched.begin().value();
    }

    //TODO FIXME --for complicated schedules and/or zoom bands,
    //different stations may have different modes
    std::string mode_key = scan["mode"].get< std::string >();

    //per-section station_code -> table_name maps. Section entries with no station
    //qualifiers fall back to the first entry in the section (handled by TableForStation).
    auto stationCodeToFreqTableName = MHO_VexHelpers::StationToKeywordMap(vex_root, mode_key, "$FREQ");
    auto stationCodeToBBCTableName = MHO_VexHelpers::StationToKeywordMap(vex_root, mode_key, "$BBC");
    auto stationCodeToIFTableName = MHO_VexHelpers::StationToKeywordMap(vex_root, mode_key, "$IF");

    //order stations by their mk4 site id for stable iteration (mirrors the legacy behavior)
    std::map< std::string, std::string > mk4IDToStationCode;
    for(const auto& kv : stationCodeToFreqTableName)
    {
        std::string mk4_id = MHO_VexHelpers::StationMk4SiteId(vex_root, kv.first);
        if(!mk4_id.empty())
            mk4IDToStationCode[mk4_id] = kv.first;
    }

    for(auto it = mk4IDToStationCode.begin(); it != mk4IDToStationCode.end(); it++)
    {
        std::string st = it->first;
        std::string station_code = it->second;
        std::string freq_table = stationCodeToFreqTableName[station_code];
        std::string bbc_name =
            MHO_VexHelpers::TableForStation(vex_root, mode_key, "$BBC", station_code, &stationCodeToBBCTableName);
        std::string if_name =
            MHO_VexHelpers::TableForStation(vex_root, mode_key, "$IF", station_code, &stationCodeToIFTableName);
        if(bbc_name.empty() || if_name.empty())
        {
            msg_warn("mk4interface", "could not resolve BBC or IF table for station: "
                                         << st << ", channel polarization may be unknown." << eom);
        }

        //figure out the ordered (ascending) unique sky frequencies. When a global
        //grid has been set we use it directly so chan_def chidx matches the chidx
        //that MHO_DiFXBaselineProcessor writes into mark4 t101 chan_ids across every
        //station in the scan. But: if NONE of this freq_table's chan_defs are on the
        //global grid (e.g. a station whose channels are all filtered out of this scan
        //by -w/--bandwidth or zoom-vs-native selection), fall back to per-station
        //numbering so the table still gets sensible (if scan-unused) channel_names
        //instead of having every chan_def stripped -- fourfit3 trips over freq_tables
        //with zero chan_defs even when it's not asked to process that station's baselines.
        const auto& chan_defs = vex_root["$FREQ"][freq_table]["chan_def"];

        bool use_global_for_this_table = fHasGlobalGrid;
        if(use_global_for_this_table)
        {
            bool any_on_grid = false;
            for(std::size_t nch = 0; nch < chan_defs.size(); nch++)
            {
                double sky_freq = chan_defs[nch]["sky_frequency"]["value"].get< double >();
                if(fGlobalGrid.Contains(sky_freq))
                {
                    any_on_grid = true;
                    break;
                }
            }
            if(!any_on_grid)
                use_global_for_this_table = false;
        }

        //grid we'll actually use for this station's freq table. Either the scan-wide
        //grid (preferred) or a per-station grid rebuilt from this table's chan_defs.
        MHO_SkyFreqGrid local_grid;
        const MHO_SkyFreqGrid* active_grid = nullptr;
        if(use_global_for_this_table)
        {
            active_grid = &fGlobalGrid;
        }
        else
        {
            for(std::size_t nch = 0; nch < chan_defs.size(); nch++)
            {
                local_grid.Add(chan_defs[nch]["sky_frequency"]["value"].get< double >());
            }
            local_grid.Finalize();
            active_grid = &local_grid;
        }

        //when running on the global grid for this table, drop chan_defs whose sky_freq
        //isn't on the grid (i.e., declared in VEX but never exported). fourfit3 looks
        //chan_defs up by chan_name from t101; an unreferenced chan_def at best wastes
        //space and at worst collides with a real chan_name elsewhere in the table.
        //Skip when use_global_for_this_table is false -- we're already in the legacy
        //per-station path and dropping would zero out a table fourfit3 still expects.
        if(use_global_for_this_table)
        {
            mho_json keep = mho_json::array();
            for(std::size_t nch = 0; nch < chan_defs.size(); nch++)
            {
                double sky_freq = chan_defs[nch]["sky_frequency"]["value"].get< double >();
                if(active_grid->Contains(sky_freq))
                {
                    keep.push_back(chan_defs[nch]);
                }
            }
            vex_root["$FREQ"][freq_table]["chan_def"] = keep;
        }

        //assign chan_name = band + chidx + sb + pol for each (surviving) chan_def
        for(std::size_t nch = 0; nch < vex_root["$FREQ"][freq_table]["chan_def"].size(); nch++)
        {
            const auto& cd = vex_root["$FREQ"][freq_table]["chan_def"][nch];
            double sky_freq = cd["sky_frequency"]["value"].get< double >();
            std::string net_sb = cd["net_sideband"].get< std::string >();
            std::string bbc_id = cd["bbc_id"].get< std::string >();
            std::string pol = MHO_VexHelpers::ResolvePolarization(vex_root, bbc_name, if_name, bbc_id);

            //determine the band
            std::string band = BandLabelFromSkyFreq(sky_freq);

            //channel number from the active grid (global scan-wide, or per-table fallback)
            std::size_t channel_index = 0;
            active_grid->FindIndex(sky_freq, channel_index);

            vex_root["$FREQ"][freq_table]["chan_def"][nch]["channel_name"] =
                MHO_MK4ChanId::Make(band, (int)channel_index, net_sb, pol);
        }
    }
}

std::string MHO_DiFXChannelNameConstructor::BandLabelFromSkyFreq(double sky_freq)
{
    std::string label = ""; //no match, then band label is empty
    for(auto it = fBandRangeLabels.begin(); it != fBandRangeLabels.end(); it++)
    {
        //assume no overlapping band assignments
        if(sky_freq < it->fHigh && sky_freq > it->fLow)
        {
            label = it->fLabel;
            return label;
        }
    }
    return label;
}

} // namespace hops
