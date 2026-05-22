#include "MHO_DiFXZoomBandRebuilder.hh"

#include "MHO_MK4ChanId.hh"
#include "MHO_Message.hh"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace hops
{

bool MHO_DiFXZoomBandRebuilder::HasZoomBands() const
{
    if(fInput == nullptr || !fInput->contains("datastream")){return false;}
    for(const auto& ds : (*fInput)["datastream"])
    {
        if(ds.contains("nZoomFreq") && ds["nZoomFreq"].get< int >() > 0){ return true; }
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
    return indices;
}

void MHO_DiFXZoomBandRebuilder::RebuildFreqSections(mho_json& vex_root, const std::string& mode_name)
{
    if(fInput == nullptr || !fInput->contains("datastream") || !fInput->contains("antenna") || !fInput->contains("freq"))
    {
        return;
    }

    const mho_json& input = *fInput;

    for(const auto& ds : input["datastream"])
    {
        int nZoomFreq = ds.value("nZoomFreq", 0);
        if(nZoomFreq == 0){ continue; }

        //resolve station code
        int antennaId = ds["antennaId"].get< int >();
        if(antennaId >= (int)input["antenna"].size()){ continue; }
        std::string difx_code = input["antenna"][antennaId]["name"].get< std::string >();
        //DiFX names are uppercase; strip trailing nulls/spaces
        difx_code.erase(
            std::find_if(difx_code.begin(), difx_code.end(), [](unsigned char c) { return !std::isalnum(c) && c != '_'; }),
            difx_code.end());
        if(!fDiFX2VexStationCodes.count(difx_code))
        {
            msg_warn("difx_interface", "zoom-band rebuild: no VEX station code for DiFX antenna: " << difx_code << eom);
            continue;
        }
        std::string vex_code = fDiFX2VexStationCodes[difx_code];

        //skip if we already built tables for this station (multiple datastreams per antenna)
        std::string freq_table_name = "difx_zoom_freq_" + vex_code;
        if(vex_root["$FREQ"].contains(freq_table_name)){continue;}

        std::string bbc_table_name = "difx_zoom_bbc_" + vex_code;
        std::string if_table_name = "difx_zoom_if_" + vex_code;

        //build map: local zoom-freq index (0..nZoomFreq-1) -> list of polarizations
        //zoomBandFreqId[i] is a LOCAL index into zoomFreqId[], not a global freq table index
        int nZoomBand = ds.value("nZoomBand", 0);
        std::map< int, std::vector< std::string > > localIdx_to_pols;
        for(int i = 0; i < nZoomBand; i++)
        {
            int local_idx = ds["zoomBandFreqId"][i].get< int >();
            std::string pol = ds["zoomBandPolName"][i].get< std::string >();
            localIdx_to_pols[local_idx].push_back(pol);
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

        //pre-sort zoom freqs by ascending sky_freq so each chidx matches the channel
        //position MHO_DiFXBaselineProcessor uses when minting mark4 chan_ids (low->high
        //freq order). chidx becomes part of the chan_name (band+chidx+sb+pol), which is
        //how fourfit3 matches t101->ref_chan_id back to the chan_def.
        std::vector< std::pair< double, int > > j_by_freq;
        for(int j = 0; j < nZoomFreq; j++)
        {
            int gidx = ds["zoomFreqId"][j].get< int >();
            if(gidx >= (int)input["freq"].size()){continue;}
            double f = input["freq"][gidx]["freq"].get< double >();
            j_by_freq.emplace_back(f, j);
        }
        std::sort(j_by_freq.begin(), j_by_freq.end());
        std::map< int, int > j_to_chidx;
        for(int chidx = 0; chidx < (int)j_by_freq.size(); chidx++)
        {
            j_to_chidx[j_by_freq[chidx].second] = chidx;
        }

        for(int j = 0; j < nZoomFreq; j++)
        {
            int global_idx = ds["zoomFreqId"][j].get< int >();
            if(global_idx >= (int)input["freq"].size()){continue;}

            double sky_freq_MHz = input["freq"][global_idx]["freq"].get< double >();
            double bw_MHz = input["freq"][global_idx]["bw"].get< double >();
            std::string sb = input["freq"][global_idx]["sideband"].get< std::string >();

            //determine band label for this sky frequency (used as band_id link value)
            std::string band_label = "X";
            for(const auto& btup : fFreqBands)
            {
                double flow = std::get< 1 >(btup);
                double fhigh = std::get< 2 >(btup);
                if(sky_freq_MHz > flow && sky_freq_MHz < fhigh)
                {
                    band_label = std::get< 0 >(btup);
                    break;
                }
            }

            int chidx = j_to_chidx[j];
            std::vector< std::string >& pols = localIdx_to_pols[j];
            for(const auto& pol : pols)
            {
                std::string bbc_id_tag = "&zoom_bbc_" + std::to_string(j) + "_" + pol;
                std::string if_id_tag = "&zoom_if_" + pol;
                std::string channel_id_tag = "&zoom_ch_" + std::to_string(j) + "_" + pol;
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
                bbc_assign["physical_bbc_id"] = j + 1;
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

        //sample_rate = 2 x bandwidth (Nyquist), using the first zoom freq's bandwidth
        {
            int first_global = ds["zoomFreqId"][0].get< int >();
            if(first_global < (int)input["freq"].size())
            {
                double first_bw = input["freq"][first_global]["bw"].get< double >();
                freq_table["sample_rate"]["value"] = 2.0 * first_bw;
                freq_table["sample_rate"]["units"] = "Ms/sec";
            }
        }

        vex_root["$FREQ"][freq_table_name] = freq_table;
        vex_root["$BBC"][bbc_table_name] = bbc_table;
        vex_root["$IF"][if_table_name] = if_table;

        //update $MODE: repoint this station's $FREQ/$BBC/$IF qualifiers at the new tables
        for(const std::string& section : {"$FREQ", "$BBC", "$IF"})
        {
            std::string new_table_name;
            if(section == "$FREQ")
            {
                new_table_name = freq_table_name;
            }
            else if(section == "$BBC")
            {
                new_table_name = bbc_table_name;
            }
            else
            {
                new_table_name = if_table_name;
            }

            if(!vex_root["$MODE"][mode_name].contains(section))
            {
                continue;
            }

            auto& mode_section = vex_root["$MODE"][mode_name][section];
            mho_json updated_entries;
            for(auto& entry : mode_section)
            {
                mho_json new_quals;
                for(auto& q : entry["qualifiers"])
                {
                    if(q.get< std::string >() != vex_code)
                        new_quals.push_back(q);
                }
                if(!new_quals.empty())
                {
                    entry["qualifiers"] = new_quals;
                    updated_entries.push_back(entry);
                }
            }
            mho_json new_entry;
            new_entry["keyword"] = new_table_name;
            new_entry["qualifiers"] = mho_json::array({vex_code});
            updated_entries.push_back(new_entry);
            vex_root["$MODE"][mode_name][section] = updated_entries;
        }

        msg_debug("difx_interface",
                  "rebuilt VEX freq/bbc/if sections from " << nZoomFreq << " zoom bands for station " << vex_code << eom);
    }
}

} // namespace hops
