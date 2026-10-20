#include "MHO_DiFXZoomBandRebuilder.hh"

#include "MHO_DiFXInputHelpers.hh"
#include "MHO_MK4ChanId.hh"
#include "MHO_Message.hh"
#include "MHO_VexHelpers.hh"

#include <algorithm>
#include <cctype>
#include <cmath>

#define OVEX_MAX_PHYSICAL_BBC_ID 16

namespace hops
{

bool MHO_DiFXZoomBandRebuilder::HasZoomBands() const
{
    if(fInput == nullptr || !fInput->contains("datastream"))
    {
        return false;
    }
    for(const auto& ds : (*fInput)["datastream"])
    {
        if(ds.contains("nZoomFreq") && ds["nZoomFreq"].get< int >() > 0)
        {
            return true;
        }
    }
    return false;
}

bool MHO_DiFXZoomBandRebuilder::WantZoomChannels() const
{
    if(!fSelectByBandwidth)
    {
        return true; //no bandwidth filter: default to zoom channels when zoom bands present
    }
    if(fInput == nullptr)
    {
        return true;
    }
    std::set< int > targets = CollectZoomTargetFreqIndices();
    if(!targets.empty() && *(targets.begin()) >= 0 && *(targets.begin()) < (int)(*fInput)["freq"].size())
    {
        double target_bw = (*fInput)["freq"][*(targets.begin())]["bw"].get< double >();
        return std::fabs(fOnlyBandwidth - target_bw) < 0.001;
    }
    //find the zoom bandwidth from the first zoom-enabled datastream
    for(const auto& ds : (*fInput)["datastream"])
    {
        if(ds.contains("nZoomFreq") && ds["nZoomFreq"].get< int >() > 0 && ds.contains("zoomFreqId") &&
           !ds["zoomFreqId"].empty())
        {
            int gidx = ds["zoomFreqId"][0].get< int >();
            double zoom_bw = (*fInput)["freq"][gidx]["bw"].get< double >();
            return std::fabs(fOnlyBandwidth - zoom_bw) < 0.001;
        }
    }
    return true;
}

std::set< int > MHO_DiFXZoomBandRebuilder::CollectZoomFreqIndices() const
{
    std::set< int > indices;
    if(fInput == nullptr || !fInput->contains("datastream"))
    {
        return indices;
    }
    for(const auto& ds : (*fInput)["datastream"])
    {
        if(ds.contains("nZoomFreq") && ds["nZoomFreq"].get< int >() > 0)
        {
            for(const auto& gidx : ds["zoomFreqId"])
                indices.insert(gidx.get< int >());
        }
    }
    std::set< int > targets = CollectZoomTargetFreqIndices();
    indices.insert(targets.begin(), targets.end());
    return indices;
}

std::set< int > MHO_DiFXZoomBandRebuilder::CollectZoomTargetFreqIndices() const
{
    std::set< int > targets;
    if(fInput == nullptr || !fInput->contains("baseline") || !fInput->contains("datastream"))
    {
        return targets;
    }
    const mho_json& input = *fInput;
    int nds = input["datastream"].size();
    for(int bl = 0; bl < (int)input["baseline"].size(); bl++)
    {
        const mho_json& base = input["baseline"][bl];
        int dsA = base.value("dsA", -1);
        int dsB = base.value("dsB", -1);
        if(dsA < 0 || dsA >= nds || dsB < 0 || dsB >= nds || !base.contains("bandA") || !base.contains("bandB"))
        {
            continue;
        }
        std::vector< int > target_freqs = MHO_DiFXInputHelpers::GetBaselineTargetFreqs(input, bl);
        for(int f = 0; f < (int)target_freqs.size(); f++)
        {
            if(target_freqs[f] < 0 || f >= (int)base["bandA"].size() || f >= (int)base["bandB"].size())
            {
                continue;
            }
            bool is_zoom = false;
            for(const auto& band : base["bandA"][f])
            {
                is_zoom |= MHO_DiFXInputHelpers::IsZoomBand(input["datastream"][dsA], band.get< int >());
            }
            for(const auto& band : base["bandB"][f])
            {
                is_zoom |= MHO_DiFXInputHelpers::IsZoomBand(input["datastream"][dsB], band.get< int >());
            }
            if(is_zoom)
            {
                targets.insert(target_freqs[f]);
            }
        }
    }
    return targets;
}

std::map< int, std::vector< std::string > >
MHO_DiFXZoomBandRebuilder::CollectDatastreamTargetPols(int ds_idx, const std::set< int >& targets) const
{
    std::map< int, std::vector< std::string > > freq_to_pols;
    const mho_json& input = *fInput;
    if(!input.contains("baseline"))
    {
        return freq_to_pols;
    }
    const mho_json& ds = input["datastream"][ds_idx];
    for(int bl = 0; bl < (int)input["baseline"].size(); bl++)
    {
        const mho_json& base = input["baseline"][bl];
        std::vector< std::string > band_keys;
        if(base.value("dsA", -1) == ds_idx)
        {
            band_keys.push_back("bandA");
        }
        if(base.value("dsB", -1) == ds_idx)
        {
            band_keys.push_back("bandB");
        }
        if(band_keys.empty())
        {
            continue;
        }
        std::vector< int > target_freqs = MHO_DiFXInputHelpers::GetBaselineTargetFreqs(input, bl);
        for(int f = 0; f < (int)target_freqs.size(); f++)
        {
            if(!targets.count(target_freqs[f]))
            {
                continue;
            }
            std::vector< std::string >& pols = freq_to_pols[target_freqs[f]];
            for(const auto& key : band_keys)
            {
                if(!base.contains(key) || f >= (int)base[key].size())
                {
                    continue;
                }
                for(const auto& band : base[key][f])
                {
                    std::string pol = MHO_DiFXInputHelpers::GetDatastreamBandPol(ds, band.get< int >());
                    if(!pol.empty() && std::find(pols.begin(), pols.end(), pol) == pols.end())
                    {
                        pols.push_back(pol);
                    }
                }
            }
        }
    }
    return freq_to_pols;
}

std::map< int, std::vector< std::string > > MHO_DiFXZoomBandRebuilder::CollectDatastreamZoomPols(const mho_json& ds) const
{
    std::map< int, std::vector< std::string > > freq_to_pols;
    int nZoomFreq = ds.value("nZoomFreq", 0);
    int nZoomBand = ds.value("nZoomBand", 0);
    for(int i = 0; i < nZoomBand; i++)
    {
        int local_idx = ds["zoomBandFreqId"][i].get< int >();
        if(local_idx < 0 || local_idx >= nZoomFreq)
        {
            continue;
        }
        int global_idx = ds["zoomFreqId"][local_idx].get< int >();
        std::string pol = ds["zoomBandPolName"][i].get< std::string >();
        std::vector< std::string >& pols = freq_to_pols[global_idx];
        if(std::find(pols.begin(), pols.end(), pol) == pols.end())
        {
            pols.push_back(pol);
        }
    }
    return freq_to_pols;
}

void MHO_DiFXZoomBandRebuilder::RebuildFreqSections(mho_json& vex_root, const std::string& mode_name)
{
    if(fInput == nullptr || !fInput->contains("datastream") || !fInput->contains("antenna") || !fInput->contains("freq"))
    {
        return;
    }

    const mho_json& input = *fInput;
    std::set< int > zoom_targets = CollectZoomTargetFreqIndices();

    for(int ds_idx = 0; ds_idx < (int)input["datastream"].size(); ds_idx++)
    {
        const mho_json& ds = input["datastream"][ds_idx];
        int nZoomFreq = ds.value("nZoomFreq", 0);
        if(nZoomFreq == 0)
        {
            continue;
        }

        //resolve station code
        int antennaId = ds.value("antennaId", -1);
        std::string difx_code = MHO_DiFXInputHelpers::GetDatastreamAntennaName(input, ds);
        if(difx_code.empty())
        {
            continue;
        }
        if(!fDiFX2VexStationCodes.count(difx_code))
        {
            msg_warn("difx_interface", "zoom-band rebuild: no VEX station code for DiFX antenna: " << difx_code << eom);
            continue;
        }
        std::string vex_code = fDiFX2VexStationCodes[difx_code];

        //skip if we already built tables for this station (multiple datastreams per antenna)
        std::string freq_table_name = "difx_zoom_freq_" + vex_code;
        if(vex_root["$FREQ"].contains(freq_table_name))
        {
            continue;
        }

        std::string bbc_table_name = "difx_zoom_bbc_" + vex_code;
        std::string if_table_name = "difx_zoom_if_" + vex_code;

        std::map< int, std::vector< std::string > > freq_to_pols;
        for(int other_idx = 0; other_idx < (int)input["datastream"].size(); other_idx++)
        {
            const mho_json& other_ds = input["datastream"][other_idx];
            if(other_ds.value("antennaId", -1) != antennaId)
            {
                continue;
            }
            std::map< int, std::vector< std::string > > ds_freq_to_pols = CollectDatastreamTargetPols(other_idx, zoom_targets);
            if(ds_freq_to_pols.empty() && other_ds.value("nZoomFreq", 0) > 0)
            {
                ds_freq_to_pols = CollectDatastreamZoomPols(other_ds);
            }
            for(const auto& fp : ds_freq_to_pols)
            {
                std::vector< std::string >& pols = freq_to_pols[fp.first];
                for(const auto& pol : fp.second)
                {
                    if(std::find(pols.begin(), pols.end(), pol) == pols.end())
                    {
                        pols.push_back(pol);
                    }
                }
            }
        }

        //look up the original IF table for this station (before we replace it) to inherit phase_cal_interval
        std::map< std::string, std::pair< double, std::string > > pol_to_pcal;
        {
            std::string orig_if_table_name;
            if(vex_root["$MODE"][mode_name].contains("$IF"))
            {
                for(auto& entry : vex_root["$MODE"][mode_name]["$IF"])
                {
                    for(auto& q : entry["qualifiers"])
                    {
                        if(q.get< std::string >() == vex_code)
                        {
                            orig_if_table_name = entry["keyword"].get< std::string >();
                            break;
                        }
                    }
                    if(!orig_if_table_name.empty())
                    {
                        break;
                    }
                }
                if(orig_if_table_name.empty() && !vex_root["$MODE"][mode_name]["$IF"].empty())
                {
                    orig_if_table_name = vex_root["$MODE"][mode_name]["$IF"][0]["keyword"].get< std::string >();
                }
            }
            if(!orig_if_table_name.empty() && vex_root["$IF"].contains(orig_if_table_name))
            {
                for(auto& ifd : vex_root["$IF"][orig_if_table_name]["if_def"])
                {
                    if(ifd.contains("polarization") && ifd.contains("phase_cal_interval"))
                    {
                        std::string p = ifd["polarization"].get< std::string >();
                        double val = ifd["phase_cal_interval"]["value"].get< double >();
                        std::string units = ifd["phase_cal_interval"]["units"].get< std::string >();
                        if(!pol_to_pcal.count(p))
                            pol_to_pcal[p] = {val, units};
                    }
                }
            }
        }

        //build synthetic $FREQ, $BBC, $IF tables
        mho_json freq_table, bbc_table, if_table;
        std::set< std::string > seen_pols;

        //pre-sort output freqs by ascending sky_freq so each chidx matches the channel
        //position MHO_DiFXBaselineProcessor uses when minting mark4 chan_ids (low->high
        //freq order). chidx becomes part of the chan_name (band+chidx+sb+pol), which is
        //how fourfit3 matches t101->ref_chan_id back to the chan_def.
        std::vector< std::pair< double, int > > gidx_by_freq;
        for(const auto& fp : freq_to_pols)
        {
            int gidx = fp.first;
            if(gidx < 0 || gidx >= (int)input["freq"].size())
            {
                continue;
            }
            gidx_by_freq.emplace_back(input["freq"][gidx]["freq"].get< double >(), gidx);
        }
        std::sort(gidx_by_freq.begin(), gidx_by_freq.end());

        for(int chidx = 0; chidx < (int)gidx_by_freq.size(); chidx++)
        {
            int global_idx = gidx_by_freq[chidx].second;

            double sky_freq_MHz = input["freq"][global_idx]["freq"].get< double >();
            double bw_MHz = input["freq"][global_idx]["bw"].get< double >();
            std::string sb = input["freq"][global_idx]["sideband"].get< std::string >();

            //determine band label for this sky frequency
            std::string band_label = MHO_DiFXInputHelpers::DetermineFreqBand(fFreqBands, sky_freq_MHz);
            if(band_label == "")
            {
                band_label = "X"; //default to "X" if none matches, so we don't get broken vex link strings
            }

            const std::vector< std::string >& pols = freq_to_pols[global_idx];
            for(const auto& pol : pols)
            {
                std::string bbc_id_tag = "&zoom_bbc_" + std::to_string(chidx) + "_" + pol;
                std::string if_id_tag = "&zoom_if_" + pol;
                std::string channel_id_tag = "&zoom_ch_" + std::to_string(chidx) + "_" + pol;
                std::string band_id_tag = "&" + band_label;

                mho_json chan_def;
                chan_def["channel_name"] = MHO_MK4ChanId::Make(band_label, chidx, sb, pol);
                chan_def["sky_frequency"]["value"] = sky_freq_MHz;
                chan_def["sky_frequency"]["units"] = "MHz";
                chan_def["bandwidth"]["value"] = bw_MHz;
                chan_def["bandwidth"]["units"] = "MHz";
                chan_def["net_sideband"] = sb;
                chan_def["band_id"] = band_id_tag;
                chan_def["bbc_id"] = bbc_id_tag;
                chan_def["channel_id"] = channel_id_tag;
                freq_table["chan_def"].push_back(chan_def);

                mho_json bbc_assign;
                bbc_assign["logical_bbc_id"] = bbc_id_tag;
                bbc_assign["physical_bbc_id"] = (chidx % OVEX_MAX_PHYSICAL_BBC_ID) + 1;
                bbc_assign["logical_if"] = if_id_tag;
                bbc_table["BBC_assign"].push_back(bbc_assign);

                if(!seen_pols.count(pol))
                {
                    seen_pols.insert(pol);
                    mho_json if_def;
                    if_def["if_id"] = if_id_tag;
                    if_def["physical_if_id"] = "Z" + pol;
                    if_def["polarization"] = pol;
                    if_def["effective_LO"]["value"] = sky_freq_MHz;
                    if_def["effective_LO"]["units"] = "MHz";
                    if_def["net_sideband"] = sb;
                    auto pcal_it = pol_to_pcal.count(pol) ? pol_to_pcal.find(pol) : pol_to_pcal.begin();
                    if(pcal_it != pol_to_pcal.end())
                    {
                        if_def["phase_cal_interval"]["value"] = pcal_it->second.first;
                        if_def["phase_cal_interval"]["units"] = pcal_it->second.second;
                    }
                    else
                    {
                        if_def["phase_cal_interval"]["value"] = 0.0;
                        if_def["phase_cal_interval"]["units"] = "MHz";
                    }
                    if_table["if_def"].push_back(if_def);
                }
            }
        }

        //sample_rate = 2 x bandwidth (Nyquist), using the lowest output freq's bandwidth
        if(!gidx_by_freq.empty())
        {
            double first_bw = input["freq"][gidx_by_freq[0].second]["bw"].get< double >();
            freq_table["sample_rate"]["value"] = 2.0 * first_bw;
            freq_table["sample_rate"]["units"] = "Ms/sec";
        }

        vex_root["$FREQ"][freq_table_name] = freq_table;
        vex_root["$BBC"][bbc_table_name] = bbc_table;
        vex_root["$IF"][if_table_name] = if_table;

        //update $MODE: repoint this station's $FREQ/$BBC/$IF qualifiers at the new tables
        const std::vector< std::pair< std::string, std::string > > section_tables = {
            {"$FREQ", freq_table_name},
            {"$BBC",  bbc_table_name },
            {"$IF",   if_table_name  }
        };
        for(const auto& st : section_tables)
        {
            if(vex_root["$MODE"][mode_name].contains(st.first))
            {
                MHO_VexHelpers::LinkStationToTable(vex_root, mode_name, st.first, vex_code, st.second);
            }
        }

        msg_debug("difx_interface", "rebuilt VEX freq/bbc/if sections from "
                                        << gidx_by_freq.size() << " output bands for station " << vex_code << eom);
    }
}

} // namespace hops
