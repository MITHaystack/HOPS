#include "MHO_DiFXChannelNameConstructor.hh"

#include <algorithm>

namespace hops
{

MHO_DiFXChannelNameConstructor::MHO_DiFXChannelNameConstructor()
{
    fScanID = "";
    fChanTol = 0.001;
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
    int nst = scan["station"].size(); // number of stations;

    //maps to resolve links
    std::map< std::string, std::string > stationCodeToSiteID;
    std::map< std::string, std::string > stationCodeToMk4ID;
    std::map< std::string, std::string > stationCodeToFreqTableName;
    std::map< std::string, std::string > mk4IDToFreqTableName;

    auto mode = vex_root["$MODE"][mode_key];
    //TODO FIXME -- this is incorrect if there are multple BBC/IFs defined
    std::string bbc_name =
        vex_root["$MODE"][mode_key]["$BBC"][0]["keyword"].get< std::string >(); //TODO FIXME if stations have different bbcs
    std::string if_name =
        vex_root["$MODE"][mode_key]["$IF"][0]["keyword"].get< std::string >(); //TODO FIXME if stations have different ifs

    for(int ist = 0; ist < nst; ist++)
    {
        //find the frequency table for this station
        //first locate the mode info
        std::string freq_key;
        for(auto it = mode["$FREQ"].begin(); it != mode["$FREQ"].end(); ++it)
        {
            std::string keyword = (*it)["keyword"].get< std::string >();
            std::size_t n_qual = (*it)["qualifiers"].size();
            for(std::size_t q = 0; q < n_qual; q++)
            {
                std::string station_code = (*it)["qualifiers"][q].get< std::string >();
                if(vex_root["$STATION"].contains(station_code))
                {
                    //std::cout<<"station code = "<<station_code<<std::endl;
                    //std::string site_key =
                    stationCodeToFreqTableName[station_code] = keyword;
                    std::string site_key = vex_root["$STATION"][station_code]["$SITE"][0]["keyword"].get< std::string >();
                    std::string mk4_id = vex_root["$SITE"][site_key]["mk4_site_ID"].get< std::string >();
                    mk4IDToFreqTableName[mk4_id] = keyword;
                }
            }
        }
    }

    for(auto it = mk4IDToFreqTableName.begin(); it != mk4IDToFreqTableName.end(); it++)
    {
        //now loop over all stations, filling in the channel names
        std::string st = it->first;
        std::string freq_table = it->second;

        //first we figure out the unique channel sky_frequencies, so we can assign
        //them ordered indices
        fOrderedSkyFrequencies.clear();

        //get the channel information of the reference station
        std::vector< double > all_freqs;
        for(std::size_t nch = 0; nch < vex_root["$FREQ"][freq_table]["chan_def"].size(); nch++)
        {
            double sky_freq = vex_root["$FREQ"][freq_table]["chan_def"][nch]["sky_frequency"]["value"].get< double >();
            all_freqs.push_back(sky_freq);
        }
        std::sort(all_freqs.begin(), all_freqs.end());

        //eliminate duplicates within some tolerance (stupid brute force N^2 comparison)
        std::size_t nfreqs = all_freqs.size();
        for(std::size_t i = 0; i < nfreqs; i++)
        {
            bool ok_to_add = true;
            for(std::size_t j = 0; j < fOrderedSkyFrequencies.size(); j++)
            {
                double delta = all_freqs[i] - fOrderedSkyFrequencies[j];
                if(std::fabs(delta) < fChanTol)
                {
                    ok_to_add = false;
                }
            }
            if(ok_to_add)
            {
                fOrderedSkyFrequencies.push_back(all_freqs[i]);
            }
        }

        std::sort(fOrderedSkyFrequencies.begin(), fOrderedSkyFrequencies.end());

        //get the channel information of the reference station
        for(std::size_t nch = 0; nch < vex_root["$FREQ"][freq_table]["chan_def"].size(); nch++)
        {
            double sky_freq = vex_root["$FREQ"][freq_table]["chan_def"][nch]["sky_frequency"]["value"].get< double >();
            double bw = vex_root["$FREQ"][freq_table]["chan_def"][nch]["bandwidth"]["value"].get< double >();
            std::string net_sb = vex_root["$FREQ"][freq_table]["chan_def"][nch]["net_sideband"].get< std::string >();
            std::string bbc_id = vex_root["$FREQ"][freq_table]["chan_def"][nch]["bbc_id"].get< std::string >();
            std::string pol = "-";
            for(std::size_t nbbc = 0; nbbc < vex_root["$BBC"][bbc_name]["BBC_assign"].size(); nbbc++)
            {
                if(vex_root["$BBC"][bbc_name]["BBC_assign"][nbbc]["logical_bbc_id"].get< std::string >() == bbc_id)
                {
                    std::string if_id = vex_root["$BBC"][bbc_name]["BBC_assign"][nbbc]["logical_if"].get< std::string >();
                    //finally retrieve the polarization
                    for(std::size_t nif = 0; nif < vex_root["$IF"][if_name]["if_def"].size(); nif++)
                    {
                        if(vex_root["$IF"][if_name]["if_def"][nif]["if_id"].get< std::string >() == if_id)
                        {
                            pol = vex_root["$IF"][if_name]["if_def"][nif]["polarization"].get< std::string >();
                            break;
                        }
                    }
                    break;
                }
            }

            //determine the band
            std::string band = BandLabelFromSkyFreq(sky_freq);

            //channel number
            std::size_t channel_index = FindChannelIndex(sky_freq);
            std::stringstream ss;
            ss << std::setfill('0') << std::setw(2) << channel_index;
            std::string chan_no = ss.str();

            //now construct the channel name
            std::string chan_name;
            chan_name = band + chan_no + net_sb + pol;
            vex_root["$FREQ"][freq_table]["chan_def"][nch]["channel_name"] = chan_name;
        }
    }
}

std::string MHO_DiFXChannelNameConstructor::BandLabelFromSkyFreq(double sky_freq)
{
    std::string label = "X"; //defaults to X
    for(auto it = fBandRangeLabels.begin(); it != fBandRangeLabels.end(); it++)
    {
        //assume no over lapping band assignments
        if(sky_freq < it->fHigh && sky_freq > it->fLow)
        {
            label = it->fLabel;
            return label;
        }
    }
    return label;
}

std::size_t MHO_DiFXChannelNameConstructor::FindChannelIndex(double sky_freq)
{
    //brute force search
    for(std::size_t i = 0; i < fOrderedSkyFrequencies.size(); i++)
    {
        double delta = sky_freq - fOrderedSkyFrequencies[i];
        if(std::fabs(delta) < fChanTol)
        {
            return i;
        }
    }
    return 0;
}

} // namespace hops