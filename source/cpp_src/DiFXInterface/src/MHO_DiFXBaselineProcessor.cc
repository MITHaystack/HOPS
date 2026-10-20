#include "MHO_DiFXBaselineProcessor.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_DiFXInputHelpers.hh"

#include "MHO_DiFXTimeUtilities.hh"
#include "MHO_DoubleSidebandChannelLabeler.hh"
#include "MHO_ElementTypeCaster.hh"
#include "MHO_MK4ChanId.hh"

#include "MHO_MK4CorelInterfaceReversed.hh"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <tuple>

#include <malloc.h>

#define DIFX_BASE2ANT 256
#define SCALE 10000.0

//tolerance to use when selecting by bandwidth
//and frequency band
#define FREQ_EPS 0.001

#define MERGED_WEIGHT_EPS 0.001

namespace hops
{

MHO_DiFXBaselineProcessor::MHO_DiFXBaselineProcessor(): fInput(nullptr), fV(nullptr), fW(nullptr), fStationCodeMap(nullptr)
{
    fRootCode = "unknown";
    fCorrDate = "";
    fRescale = true;
    fScaleFactor = 1.0;
    fBaselineDelim = "-";
    fIndex = 0;
    fAttachDiFXInput = false;
    fExportAsMark4 = false;

    /* The following coefficients are taken directly from difx2mark4 new_type1.c */

    // factors are sqrt (Van Vleck correction) for 1b, 2b case
    // quantization correction factor is pi/2 for
    // 1x1 bit, or ~1.13 for 2x2 bit (see TMS, p.300)
    // 1x2 bit uses harmonic mean of 1x1 and 2x2
    // Note that these values apply to the weak signal
    // (i.e. cross-correlation) case only.
    fNBitsToFactor.clear();
    fNBitsToFactor[1] = 1.25331;
    fNBitsToFactor[2] = 1.06448;

    fFreqBands.clear();
    fOnlyFreqGroups.clear();
    fSelectByBandwidth = false;
    fOnlyBandwidth = 0;
    fHasGlobalGrid = false;
    fSawZoomFrequencyRecord = false;
};

MHO_DiFXBaselineProcessor::~MHO_DiFXBaselineProcessor()
{
    Clear();
}

void MHO_DiFXBaselineProcessor::AddRecord(MHO_DiFXVisibilityRecord* record)
{
    //keep track of the baseline id on first insertion
    if(fRecords.size() == 0)
    {
        fBaselineID = record->baseline;
    }
    bool keep_record = true;

    //discard if this record does not match our baseline (how did it get here?)
    if(fBaselineID != record->baseline)
    {
        keep_record = false;
    }

    //figure out this records channel info (freq,bandwidth)
    int freqidx = record->freqindex;
    int points = record->nchan;
    double freq = 0;
    double bandwidth = 0;
    std::string sideband = "";

    //collect channel info
    if(fInput)
    {
        if(freqidx >= 0 && freqidx < static_cast< int >((*fInput)["freq"].size()))
        {
            freq = (*fInput)["freq"][freqidx]["freq"].get< double >();
            bandwidth = (*fInput)["freq"][freqidx]["bw"].get< double >();
        }
        else
        {
            msg_warn("difx_interface", "invalid DiFX freq index " << freqidx << " for baseline " << record->baseline << eom);
            keep_record = false;
        }
    }

    if(fSelectByBandwidth)
    {
        if(std::fabs(fOnlyBandwidth - bandwidth) > FREQ_EPS)
        {
            keep_record = false; //discard, does not match our bandwidth selection
        }
    }

    std::string fgroup = DetermineFreqGroup(freq);
    bool in_fgroups = true;
    if(fOnlyFreqGroups.size() != 0)
    {
        in_fgroups = false;
        for(std::size_t i = 0; i < fOnlyFreqGroups.size(); i++)
        {
            if(fgroup == fOnlyFreqGroups[i])
            {
                in_fgroups = true;
                break;
            }
        }
    }

    //discard if not in the list of frequency groups
    if(!in_fgroups)
    {
        keep_record = false;
    }

    if(keep_record && !fZoomFreqIndices.empty() && fZoomFreqIndices.find(record->freqindex) != fZoomFreqIndices.end())
    {
        fSawZoomFrequencyRecord = true;
    }

    if(keep_record)
    {
        std::string pp(record->polpair, 2);
        int freqidx = record->freqindex;
        int points = record->nchan;
        fPolPairSet.insert(pp);
        fFreqIndexSet.insert(freqidx);
        fSpecPointSet.insert(points);
        fBandwidthSet.insert(bandwidth);
        fRecords.push_back(record);
        fVisibilities[pp][freqidx].push_back(record);

        record->bandwidth = bandwidth;
        record->sky_freq = freq;
        record->freq_band = fgroup;
    }
    else
    {
        //doesn't match our selection criteria, dump it
        delete record;
    }
}

void MHO_DiFXBaselineProcessor::MergeSplitOutputBandRecords()
{
    typedef std::tuple< int, std::string, int, double, int, int > record_key;
    std::map< record_key, MHO_DiFXVisibilityRecord* > first_records;

    std::vector< MHO_DiFXVisibilityRecord* > merged_records;
    merged_records.reserve(fRecords.size());

    std::set< MHO_DiFXVisibilityRecord* > merged_targets;
    std::size_t n_merged = 0;

    for(auto* record : fRecords)
    {
        record_key key(record->freqindex, std::string(record->polpair, 2), record->mjd, record->seconds, record->sourceindex,
                       record->pulsarbin);

        auto it = first_records.find(key);
        if(it == first_records.end())
        {
            first_records[key] = record;
            merged_records.push_back(record);
            continue;
        }

        MHO_DiFXVisibilityRecord* target = it->second;
        if(target->nchan != record->nchan || target->visdata.size() != record->visdata.size())
        {
            msg_warn("difx_interface", "cannot merge duplicate visibility records with different spectral size on baseline "
                                           << fBaselineID << ", freq index " << record->freqindex << eom);
            merged_records.push_back(record);
            continue;
        }

        for(std::size_t i = 0; i < target->visdata.size(); i++)
        {
            target->visdata[i] += record->visdata[i];
        }
        target->dataweight += record->dataweight;
        merged_targets.insert(target);
        delete record;
        n_merged++;
    }

    if(n_merged != 0)
    {
        msg_debug("difx_interface",
                  "merged " << n_merged << " partial output band visibility records on baseline " << fBaselineID << eom);
    }

    std::size_t n_overweight = 0;
    double max_weight = 0.0;
    for(auto* target : merged_targets)
    {
        if(target->dataweight > 1.0 + MERGED_WEIGHT_EPS)
        {
            n_overweight++;
            max_weight = std::max(max_weight, target->dataweight);
        }
    }
    if(n_overweight != 0)
    {
        msg_warn("difx_interface", n_overweight << " merged visibility records on baseline " << fBaselineID
                                                << " have a total weight > 1.0 (max: " << max_weight
                                                << "), the SWIN file may contain duplicate (not partial output band) records!"
                                                << eom);
    }
    fRecords.swap(merged_records);
}

void MHO_DiFXBaselineProcessor::AddMissingPolFreqPairs()
{
    //every pol-product ought to get an entry for every channel on a baseline (if missing),
    //pairs without records get filled here so all of their APs become empty (zero-weighted) slots
    for(auto ppit = fPolPairSet.begin(); ppit != fPolPairSet.end(); ppit++)
    {
        auto& freq_map = fVisibilities[*ppit];
        for(auto fqit = fFreqIndexSet.begin(); fqit != fFreqIndexSet.end(); fqit++)
        {
            if(freq_map.find(*fqit) == freq_map.end())
            {
                msg_warn("difx_interface", "no visibility records on baseline: "
                                               << fBaselineID << " for pol-product: " << *ppit << ", freq index: " << *fqit
                                               << ", this channel will be zero weighted." << eom);
                freq_map[*fqit] = std::vector< MHO_DiFXVisibilityRecord* >();
            }
        }
    }
}

void MHO_DiFXBaselineProcessor::Organize()
{
    fHaveBaselineData = true;

    //MHO_DiFXScanProcessor calls Organize() once up-front (to harvest
    //fBaselineFreqs so the scan-wide global sky-freq grid can be computed) however it then calls it a second
    //time from ConstructVisibilityFileObjects, so we have to clear the per-call accumulators so the
    //second invocation rebuilds them rather than appending duplicates.
    fBaselineFreqs.clear();
    fFreqBandLabelSet.clear();

    if(fRecords.size() == 0)
    {
        msg_warn("difx_interface", "no visiblity records available for baseline: " << fBaselineID << eom);
        fHaveBaselineData = false;
        return;
    }

    //check if we are using zoom-bands
    if(!fZoomFreqIndices.empty() && fSawZoomFrequencyRecord)
    {
        std::vector< MHO_DiFXVisibilityRecord* > selected_records;
        selected_records.reserve(fRecords.size());
        for(auto* record : fRecords)
        {
            if(fZoomFreqIndices.find(record->freqindex) != fZoomFreqIndices.end())
            {
                selected_records.push_back(record);
            }
            else
            {
                delete record;
            }
        }
        fRecords.swap(selected_records);
    }
    else if(!fZoomFreqIndices.empty())
    {
        msg_debug("difx_interface",
                  "baseline " << fBaselineID << " has no zoom-frequency visibility records; retaining native frequency records"
                              << eom);
    }

    //catch and merge any split (across datastreams/baseline) records
    MergeSplitOutputBandRecords();

    fPolPairSet.clear();
    fFreqIndexSet.clear();
    fSpecPointSet.clear();
    fBandwidthSet.clear();
    fVisibilities.clear();
    for(auto* record : fRecords)
    {
        int freqidx = record->freqindex;
        std::string pp(record->polpair, 2);
        fPolPairSet.insert(pp);
        fFreqIndexSet.insert(freqidx);
        fSpecPointSet.insert(record->nchan);
        fBandwidthSet.insert(record->bandwidth);
        fVisibilities[pp][freqidx].push_back(record);
    }
    AddMissingPolFreqPairs();

    if(fInput == nullptr)
    {
        msg_warn("difx_interface", "no difx input data set for baseline: " << fBaselineID << eom);
        fHaveBaselineData = false;
        return;
    }

    //first figure out the baseline name
    /* The baseline number (256*A1 + A2, 1 indexed) */
    int ant1 = (fBaselineID / DIFX_BASE2ANT) - 1;
    int ant2 = (fBaselineID % DIFX_BASE2ANT) - 1;
    std::string difx_ref_station = (*fInput)["antenna"][ant1]["name"];
    std::string difx_rem_station = (*fInput)["antenna"][ant2]["name"];

    //resolve the (vex) 2-char station code from the DiFX (all-caps) code, with a mk4-style
    //capitalization fallback when the VEX file didn't provide a mapping.
    auto resolve_code = [&](const std::string& difx_code) -> std::string {
        auto it = fDiFX2VexStationCodes.find(difx_code);
        if(it != fDiFX2VexStationCodes.end())
            return it->second;
        std::string fallback = std::string() + (char)std::toupper(difx_code[0]) + (char)std::tolower(difx_code[1]);
        msg_warn("difx_interface", "could not locate vex 2-char ID associated with difx station: "
                                       << difx_code << ", assigning mk4 convention: " << fallback << eom);
        return fallback;
    };
    auto resolve_name = [&](const std::string& difx_code) -> std::string {
        auto it = fDiFX2VexStationNames.find(difx_code);
        if(it != fDiFX2VexStationNames.end())
            return it->second;
        msg_warn("difx_interface", "could not locate vex station name associated with difx station: "
                                       << difx_code << ", using: " << difx_code << eom);
        return difx_code;
    };
    fRefStation = resolve_code(difx_ref_station);
    fRemStation = resolve_code(difx_rem_station);
    fRefStationName = resolve_name(difx_ref_station);
    fRemStationName = resolve_name(difx_rem_station);

    fBaselineName = fRefStation + fBaselineDelim + fRemStation;
    fRefStationMk4Id = fStationCodeMap->GetMk4IdFromStationCode(fRefStation);
    fRemStationMk4Id = fStationCodeMap->GetMk4IdFromStationCode(fRemStation);
    fBaselineShortName = fRefStationMk4Id + fRemStationMk4Id;

    //get the AP length (which config should we use if there is more than one?)
    fAPLength = (*fInput)["config"][0]["tInt"];

    //check number of polpairs
    fNPolPairs = fPolPairSet.size();
    if(fNPolPairs > 4)
    {
        msg_warn("difx_interface",
                 "more than 4 pol-products, detected (" << fNPolPairs << ") on baseline: " << fBaselineName << eom);
    }
    fNChannels = fFreqIndexSet.size();

    //check if the number of spectral points is the same for every channel correlated
    fNSpectralPoints = 0;
    if(!fSpecPointSet.empty())
    {
        fNSpectralPoints = *(fSpecPointSet.rbegin());
    } //grab the max # of spectral points
    //can only use channelized visibilities if every channel has the same number of spectral points
    fCanChannelize = true;
    if(fSpecPointSet.size() > 1)
    {
        fCanChannelize = false;
        //collect the bandwidths corresponding to each distinct spectral point count
        //so the user can be informed if they need to pass to -w option
        std::string bw_list;
        for(auto freqit = fFreqIndexSet.begin(); freqit != fFreqIndexSet.end(); freqit++)
        {
            if(fInput && *freqit < (int)(*fInput)["freq"].size())
            {
                double bw = (*fInput)["freq"][*freqit]["bw"].get< double >();
                std::string bw_str = std::to_string(bw);
                if(bw_list.find(bw_str) == std::string::npos)
                {
                    if(!bw_list.empty())
                        bw_list += ", ";
                    bw_list += bw_str;
                }
            }
        }
        msg_warn("difx_interface",
                 "channels on baseline "
                     << fBaselineName << " have mixed channel bandwidths (" << bw_list
                     << " MHz). Only a single bandwidth is supported. Use -w <bandwidth> to select the desired channel "
                        "bandwidth and re-run. Skipping this baseline."
                     << eom);
        return;
    }

    //bin each record onto the scan-relative AP grid using its own (mid-AP) time stamp,
    //leading/trailing APs without data on this baseline are dropped
    fStartMJD = (*fInput)["scan"][fIndex]["mjdStart"].get< double >();
    int start_day = (int)std::floor(fStartMJD);
    double start_sec = (fStartMJD - start_day) * 86400.0;
    auto scan_ap = [&](const MHO_DiFXVisibilityRecord* r) {
        double dt = (r->mjd - start_day) * 86400.0 + (r->seconds - start_sec);
        return (int)std::floor(dt / fAPLength);
    };

    int first_ap = -1;
    int last_ap = -1;
    for(auto* record : fRecords)
    {
        int ap = scan_ap(record);
        if(ap < 0)
        {
            continue;
        }
        if(first_ap < 0 || ap < first_ap)
        {
            first_ap = ap;
        }
        last_ap = std::max(last_ap, ap);
    }
    if(first_ap < 0)
    {
        msg_warn("difx_interface", "no visibility records within the scan time range for baseline: " << fBaselineID << eom);
        fHaveBaselineData = false;
        return;
    }
    fAPOffset = first_ap;
    fNAPs = last_ap - first_ap + 1;

    for(auto& ppit : fVisibilities)
    {
        for(auto& frqit : ppit.second)
        {
            std::vector< MHO_DiFXVisibilityRecord* > dense(fNAPs, nullptr);
            std::size_t nmissing = fNAPs;
            for(auto* record : frqit.second)
            {
                int ap = scan_ap(record) - fAPOffset;
                if(ap < 0)
                {
                    msg_warn("difx_interface", "dropping visibility record preceding scan start on baseline: "
                                                   << fBaselineID << ", freq index: " << frqit.first << eom);
                    continue;
                }
                if(dense[ap] != nullptr)
                {
                    msg_warn("difx_interface", "dropping duplicate visibility record for AP "
                                                   << ap + fAPOffset << " on baseline: " << fBaselineID
                                                   << ", freq index: " << frqit.first << eom);
                    continue;
                }
                dense[ap] = record;
                nmissing--;
            }
            if(nmissing != 0)
            {
                msg_warn("difx_interface", nmissing << " APs missing on baseline: " << fBaselineName
                                                    << ", pol-product: " << ppit.first << ", freq index: " << frqit.first
                                                    << ", these will be zero-weighted." << eom);
            }
            frqit.second.swap(dense);
        }
    }

    //the time axis is relative to the scan start, so the first AP with data is at fAPOffset*fAPLength
    fStartTime = get_vexdate_from_mjd_sec(fStartMJD, 0.0); //zero implies no second-offset
    auto start_tp = hops_clock::from_vex_format(fStartTime);
    auto stop_tp = start_tp + hops_clock::duration((int64_t)std::llround(fAPLength * (fAPOffset + fNAPs) * SEC_TO_NANOSEC));
    fStopTime = hops_clock::to_vex_format(stop_tp);

    msg_debug("difx_interface", "scan start time is: " << fStartTime << ", baseline " << fBaselineName
                                                       << " first AP with data: " << fAPOffset << eom);

    //grab the source name (so we can add it to the data tags)
    fSourceName = (*fInput)["source"][fIndex]["name"].get< std::string >();

    //construct the table of frequencies for this baseline and sort in asscending order
    fNChannels = fFreqIndexSet.size();
    for(auto it = fFreqIndexSet.begin(); it != fFreqIndexSet.end(); it++)
    {
        int freqidx = *it;
        mho_json freq = (*fInput)["freq"][freqidx];
        fBaselineFreqs.push_back(std::make_pair(freqidx, freq));
    }
    std::sort(fBaselineFreqs.begin(), fBaselineFreqs.end(), fFreqPredicate);

    msg_debug("difx_interface", "data dimensions of baseline: " << fBaselineID << " - " << fBaselineName << " are ("
                                                                << fNPolPairs << ", " << fNChannels << ", " << fNAPs << ", "
                                                                << fNSpectralPoints << ")" << eom);

    //loop over the datastreams, find which ones correspond to the stations on
    //this baseline and figure out the #number of quantizations bits for
    //the reference (ant1) and remote station (ant2)
    //this implicity assumes all datastreams from one antenna are sampled with
    //the same number of bits
    fRemStationBits = 0;
    fRefStationBits = 0;
    std::size_t nDatastreams = (*fInput)["datastream"].size();

    for(std::size_t d = 0; d < nDatastreams; d++)
    {
        int ant_id = (*fInput)["datastream"][d]["antennaId"];
        if(ant_id == ant1) //reference station data stream
        {
            fRefStationBits = (*fInput)["datastream"][d]["quantBits"];
        }
        if(ant_id == ant2) //remote station data stream
        {
            fRemStationBits = (*fInput)["datastream"][d]["quantBits"];
        }
        if(fRefStationBits != 0 && fRemStationBits != 0)
        {
            break;
        }
    }

    fScaleFactor = SCALE;
    if(ant1 != ant2) //cross-correlation: apply Van Vleck per station's quantization bits
    {
        auto apply_vv = [&](int bits, const char* which) {
            auto it = fNBitsToFactor.find(bits);
            if(it != fNBitsToFactor.end())
                fScaleFactor *= it->second;
            else
                msg_warn("difx_interface",
                         which << " station quantization bits: " << bits << ", not in Van Vleck correction table" << eom);
        };
        apply_vv(fRefStationBits, "reference");
        apply_vv(fRemStationBits, "remote");
    }
}

void MHO_DiFXBaselineProcessor::SetStationCodes(MHO_StationCodeMap* code_map)
{
    fStationCodeMap = code_map;
}

void MHO_DiFXBaselineProcessor::ConstructVisibilityFileObjects()
{
    //fBaselineFreqs contains the ordered (ascending) list of channel frequencies
    //fVisibilities contains the pol-pair and time sorted visibilities
    Organize();

    //first construct a channelized visibility container
    if(fV)
    {
        delete fV;
        fV = nullptr;
    }
    if(fW)
    {
        delete fW;
        fW = nullptr;
    }

    if(fHaveBaselineData && fCanChannelize && fInput != nullptr)
    {
        if(fAttachDiFXInput)
        {
            //insert the difx input data as a json object attached to the tags object
            fTags.SetTagValue("difx_input_json", *fInput);
        }

        fV = new visibility_store_type();
        fW = new weight_store_type();
        fV->Resize(fNPolPairs, fNChannels, fNAPs, fNSpectralPoints);
        fV->ZeroArray();
        //fW: one weight value per AP, so the spectral-point axis has dimension 1
        fW->Resize(fNPolPairs, fNChannels, fNAPs, 1);
        fW->ZeroArray();

        //stamp the common baseline-identification keys on fTags, fV, and fW.
        const std::vector< std::pair< std::string, std::string > > string_kvs = {
            {"baseline",                fBaselineName     },
            {"baseline_shortname",      fBaselineShortName},
            {"reference_station",       fRefStation       },
            {"remote_station",          fRemStation       },
            {"reference_station_name",  fRefStationName   },
            {"remote_station_name",     fRemStationName   },
            {"reference_station_mk4id", fRefStationMk4Id  },
            {"remote_station_mk4id",    fRemStationMk4Id  },
            {"correlation_date",        fCorrDate         },
            {"root_code",               fRootCode         },
            {"origin",                  "difx"            },
            {"start",                   fStartTime        },
            {"stop",                    fStopTime         },
            {"source",                  fSourceName       },
        };
        fTags.SetTagValue("difx_baseline_index", fBaselineID);
        fTags.SetTagValue("name", std::string("object_tags"));
        for(const auto& kv : string_kvs)
        {
            fTags.SetTagValue(kv.first, kv.second);
        }
        fV->Insert(std::string("difx_baseline_index"), fBaselineID);
        fV->Insert(std::string("name"), std::string("visibilities"));
        for(const auto& kv : string_kvs)
        {
            fV->Insert(kv.first, kv.second);
        }
        fW->Insert(std::string("difx_baseline_index"), fBaselineID);
        fW->Insert(std::string("name"), std::string("weights"));
        for(const auto& kv : string_kvs)
        {
            fW->Insert(kv.first, kv.second);
        }

        //polarization product axis
        auto* polprod_axis = &(std::get< POLPROD_AXIS >(*fV));
        auto* wpolprod_axis = &(std::get< POLPROD_AXIS >(*fW));
        polprod_axis->Insert(std::string("name"), std::string("polarization_product"));
        wpolprod_axis->Insert(std::string("name"), std::string("polarization_product"));

        //channel axis
        auto* ch_axis = &(std::get< CHANNEL_AXIS >(*fV));
        auto* wch_axis = &(std::get< CHANNEL_AXIS >(*fW));
        ch_axis->Insert(std::string("name"), std::string("channel"));
        wch_axis->Insert(std::string("name"), std::string("channel"));
        ch_axis->Insert(std::string("units"), std::string("MHz"));
        wch_axis->Insert(std::string("units"), std::string("MHz"));

        //AP axis
        auto* ap_axis = &(std::get< TIME_AXIS >(*fV));
        auto* wap_axis = &(std::get< TIME_AXIS >(*fW));
        ap_axis->Insert(std::string("name"), std::string("time"));
        wap_axis->Insert(std::string("name"), std::string("time"));
        ap_axis->Insert(std::string("units"), std::string("s"));
        wap_axis->Insert(std::string("units"), std::string("s"));

        //(sub-channel) frequency axis
        auto* sp_axis = &(std::get< FREQ_AXIS >(*fV));
        auto* wsp_axis = &(std::get< FREQ_AXIS >(*fW));
        sp_axis->Insert(std::string("name"), std::string("frequency"));
        wsp_axis->Insert(std::string("name"), std::string("frequency"));
        sp_axis->Insert(std::string("units"), std::string("MHz"));
        wsp_axis->Insert(std::string("units"), std::string("MHz"));

        int ppidx = 0;
        fFreqIndexSet.clear();
        for(auto ppit = fPolPairSet.begin(); ppit != fPolPairSet.end(); ppit++)
        {
            std::string pp = *ppit;
            polprod_axis->at(ppidx) = pp;
            wpolprod_axis->at(ppidx) = pp;

            //loop through baseline freqs in order, grabbing the associated channel
            //and populating the visibility container
            //loop though in freq (low -> high) order
            int chidx = 0;
            for(auto fqit = fBaselineFreqs.begin(); fqit != fBaselineFreqs.end(); fqit++)
            {
                int freqidx = fqit->first;
                mho_json dfreq = fqit->second;
                double sky_freq = dfreq["freq"];
                double bw = dfreq["bw"];
                std::string sideband = dfreq["sideband"];

                if(ppidx == 0) //only one label needed for each channel
                {
                    mho_json ch_label;
                    ch_label["sky_freq"] = sky_freq;
                    ch_label["bandwidth"] = bw;
                    ch_label["net_sideband"] = sideband;
                    ch_label["difx_freqindex"] = freqidx; //probably ought to be more systematic about creating channel names
                    ch_label["channel"] = chidx;          //channel position index
                    std::string fband = DetermineFreqGroup(sky_freq);
                    fFreqBandLabelSet.insert(fband);    //keep track of all the freq-band labels we've inserted
                    ch_label["frequency_band"] = fband; //will be empty string if assignment can't be made

                    //chidx (above) is the dense position of this channel in fBaselineFreqs and is what
                    //drives indexing into fV/fW. The chidx that goes into the mark4 chan_id string,
                    //however, must match the chidx MHO_DiFXChannelNameConstructor stamps into
                    //chan_def.channel_name; when a scan-wide global sky-freq grid has been provided
                    //we use position-in-grid for the mark4 chan_id, otherwise fall back to chidx.
                    int mk4_chidx = chidx;
                    if(fHasGlobalGrid)
                    {
                        std::size_t gi = 0;
                        if(fGlobalGrid.FindIndex(sky_freq, gi))
                        {
                            mk4_chidx = (int)gi;
                        }
                    }
                    std::string ref_chan_id = MHO_MK4ChanId::Make(fband, mk4_chidx, sideband, pp[0]);
                    std::string rem_chan_id = MHO_MK4ChanId::Make(fband, mk4_chidx, sideband, pp[1]);

                    ch_label["mk4_channel_id"] = ref_chan_id + ":" + rem_chan_id;

                    ch_axis->at(chidx) = sky_freq; //channel axis is now sky frequency, not chidx
                    wch_axis->at(chidx) = sky_freq;
                    ch_axis->SetLabelObject(ch_label, chidx);
                    wch_axis->SetLabelObject(ch_label, chidx);
                }

                for(std::size_t ap = 0; ap < fNAPs; ap++)
                {
                    ap_axis->at(ap) = (fAPOffset + ap) * fAPLength;
                    wap_axis->at(ap) = (fAPOffset + ap) * fAPLength;
                    wsp_axis->at(0) = 0;
                    MHO_DiFXVisibilityRecord* visRec = fVisibilities[pp][freqidx][ap];
                    if(visRec == nullptr)
                    {
                        continue;
                    }
                    (*fW)(ppidx, chidx, ap, 0) = visRec->dataweight; //set the data weight for this AP
                    for(std::size_t sp = 0; sp < fNSpectralPoints; sp++)
                    {
                        sp_axis->at(sp) = sp * (bw / fNSpectralPoints); //frequency offset from edge of channel
                        std::complex< double > tmp;
                        //for lower sideband flip axis and conjugate
                        //why?...difx2mark4 does this, but then fourfit inverts it?) //TODO VERIFY IF THIS IS NEEDED
                        if(sideband == "L")
                        {
                            tmp = std::conj(visRec->visdata[fNSpectralPoints - 1 - sp]);
                        }
                        else
                        {
                            tmp = visRec->visdata[sp];
                        }
                        (*fV)(ppidx, chidx, ap, sp) = tmp;
                    }
                }
                chidx++;
            }
            ppidx++;
        }

        //finally, we need to label channels which occur in 'double-sideband' pairs
        MHO_DoubleSidebandChannelLabeler< visibility_store_type > vis_dsb_detect;
        vis_dsb_detect.SetArgs(fV);
        bool init = vis_dsb_detect.Initialize();
        if(init)
        {
            bool exe = vis_dsb_detect.Execute();
            if(!exe)
            {
                msg_error("difx_interface", "failed to execute DSB channel detection on visibilities" << eom);
            }
        }

        MHO_DoubleSidebandChannelLabeler< weight_store_type > wt_dsb_detect;
        wt_dsb_detect.SetArgs(fW);
        bool winit = wt_dsb_detect.Initialize();
        if(winit)
        {
            bool exe = wt_dsb_detect.Execute();
            if(!exe)
            {
                msg_error("difx_interface", "failed to execute DSB channel detection on weights" << eom);
            }
        }

        //the very last thing we do is to attach the pol-product set and freq-band set to the 'Tags' data
        //this is allow a program reading this file to determine this information without streaming
        //in the potentially very large visibility/weights data
        std::vector< std::string > fband_vec(fFreqBandLabelSet.begin(), fFreqBandLabelSet.end());
        std::vector< std::string > pprod_vec(fPolPairSet.begin(), fPolPairSet.end());
        fTags.SetTagValue("frequency_band_set", fband_vec);
        fTags.SetTagValue("polarization_product_set", pprod_vec);
    }
    else
    {
        if(fHaveBaselineData) //not an error if there is no data
        {
            msg_error("difx_interface",
                      "cannot channelize visibility data, as not all channels are equal length. Feature not yet supported"
                          << eom);
        }
    }

    DeleteDiFXVisRecords();
};

void MHO_DiFXBaselineProcessor::WriteVisibilityObjects(std::string output_dir)
{
    if(fV != nullptr && fW != nullptr) //only if we have data
    {
        //apply difx2mark4 style factor and Van Vleck correction before writing out
        if(fRescale)
        {
            //apply a x10000 factor to convert to "Whitney's"
            // and apply Van Vleck n-bit statistics normalization factor (only 2x2, 1x1, and 1x2 bit supported)
            (*fV) *= fScaleFactor;
        }

        //construct output file name
        std::string root_code = fRootCode;
        std::string output_file = ConstructCorFileName(output_dir, root_code, fBaselineName, fBaselineShortName);

        if(!fExportAsMark4)
        {
            //write out the data in HOPS4 format
            MHO_BinaryFileInterface inter;
            bool status = inter.OpenToWrite(output_file);
            if(status)
            {
                fTags.AddObjectUUID(fV->GetObjectUUID());
                fTags.AddObjectUUID(fW->GetObjectUUID());
                inter.Write(fTags, "tags");
                inter.Write(*fV, "vis");
                inter.Write(*fW, "weight");
                inter.Close();
            }
            else
            {
                msg_error("file", "error opening corel output file: " << output_file << eom);
            }
            inter.Close();
        }
        else
        {
            //write out the data in Mark4 legacy type_1xx format
            MHO_MK4CorelInterfaceReversed converter;
            converter.SetOutputDirectory(output_dir);
            std::string ovex_file_name = fSourceName + "." + root_code;
            converter.SetRootFileName(ovex_file_name); //have to set the ovex file name in the type_100
            converter.SetVisibilityData(fV);
            converter.SetWeightData(fW);
            converter.GenerateCorelStructure();
            converter.WriteCorelFile();
            converter.FreeAllocated();
        }
    }
    delete fV;
    fV = nullptr;
    delete fW;
    fW = nullptr;
};

void MHO_DiFXBaselineProcessor::Clear()
{
    DeleteDiFXVisRecords();
    fPolPairSet.clear();
    fFreqIndexSet.clear();
    fSpecPointSet.clear();
    fVisibilities.clear();
    fBaselineFreqs.clear();
    if(fV)
    {
        delete fV;
        fV = nullptr;
    }
    if(fW)
    {
        delete fW;
        fW = nullptr;
    }
}

void MHO_DiFXBaselineProcessor::DeleteDiFXVisRecords()
{
    //delete the visibility records
    for(std::size_t i = 0; i < fRecords.size(); i++)
    {
        delete fRecords[i];
    }
    fRecords.clear();
    malloc_trim(0); //for lots of small objects this may be helpful to flush pages back to OS
}

std::string MHO_DiFXBaselineProcessor::DetermineFreqGroup(double freq)
{
    return MHO_DiFXInputHelpers::DetermineFreqBand(fFreqBands, freq);
}

std::string MHO_DiFXBaselineProcessor::ConstructCorFileName(const std::string& output_dir, const std::string& root_code,
                                                            const std::string& baseline, const std::string& baseline_shortname)
{
    std::string output_file = output_dir + "/" + baseline_shortname + "." + baseline + "." + root_code + ".cor";
    return output_file;
}

} // namespace hops
