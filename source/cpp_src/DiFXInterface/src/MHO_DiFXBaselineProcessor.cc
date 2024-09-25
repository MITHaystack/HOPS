#include "MHO_DiFXBaselineProcessor.hh"
#include "MHO_BinaryFileInterface.hh"

#include "MHO_ElementTypeCaster.hh"
#include "MHO_DoubleSidebandChannelLabeler.hh"


#include <cctype>
#include <cmath>

#include <malloc.h>

#define DIFX_BASE2ANT 256
#define SCALE 10000.0

//tolerance to use when selecting by bandwidth
//and frequency band
#define FREQ_EPS 0.001

namespace hops
{

MHO_DiFXBaselineProcessor::MHO_DiFXBaselineProcessor():
    fInput(nullptr),
    fV(nullptr),
    fW(nullptr),
    fStationCodeMap(nullptr)
{
    fRootCode = "unknown";
    fCorrDate = "";
    fRescale = true;
    fScaleFactor = 1.0;
    fBaselineDelim = "-";

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
};

MHO_DiFXBaselineProcessor::~MHO_DiFXBaselineProcessor()
{
    Clear();
}

void
MHO_DiFXBaselineProcessor::AddRecord(MHO_DiFXVisibilityRecord* record)
{
    //keep track of the baseline id on first insertion
    if(fRecords.size() == 0){fBaselineID = record->baseline;}
    bool keep_record = true;

    //discard if this record does not match our baseline (how did it get here?)
    if(fBaselineID != record->baseline){keep_record = false;}

    //figure out this records channel info (freq,bandwidth)
    int freqidx = record->freqindex;
    int points = record->nchan;
    double freq = 0;
    double bandwidth = 0;
    std::string sideband = "";

    //collect channel info
    if(fInput && freqidx < (*fInput)["freq"].size() )
    {
        freq = (*fInput)["freq"][freqidx]["freq"].get<double>();
        bandwidth = (*fInput)["freq"][freqidx]["bw"].get<double>();
    }

    if(fSelectByBandwidth)
    {
        if( std::fabs(fOnlyBandwidth - bandwidth) > FREQ_EPS)
        {
            keep_record = false; //discard, does not match our bandwidth selection
        }
    }

    std::string fgroup = DetermineFreqGroup(freq);
    bool in_fgroups = true;
    if(fOnlyFreqGroups.size() != 0)
    {
        in_fgroups = false;
        for(std::size_t i=0; i<fOnlyFreqGroups.size(); i++)
        {
            if(fgroup == fOnlyFreqGroups[i]){in_fgroups = true; break;}
        }
    }

    //discard if not in the list of frequency groups
    if(!in_fgroups){keep_record = false;}

    if(keep_record)
    {
        std::string pp(record->polpair,2);
        int freqidx = record->freqindex;
        int points = record->nchan;
        double bandwidth = 0.;
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


void
MHO_DiFXBaselineProcessor::Organize()
{
    fHaveBaselineData = true;

    if(fRecords.size() == 0)
    {
        msg_warn("difx_interface", "no visiblity records available for baseline: " << fBaselineID << eom);
        fHaveBaselineData = false;
        return;
    }

    if(fInput == nullptr)
    {
        msg_warn("difx_interface", "no difx input data set for baseline: " << fBaselineID << eom);
        fHaveBaselineData = false;
        return;
    }

    //first figure out the baseline name (CHECK THIS)
    /* The baseline number (256*A1 + A2, 1 indexed) */
    int ant1 = (fBaselineID / DIFX_BASE2ANT) - 1;
    int ant2 = (fBaselineID % DIFX_BASE2ANT) - 1;
    std::string difx_ref_station = (*fInput)["antenna"][ant1]["name"];
    std::string difx_rem_station = (*fInput)["antenna"][ant2]["name"];

    //convert the 2-char codes from all-caps to mk4-convention of 1-cap, 1-lower case
    if(fDiFX2VexStationCodes.find(difx_ref_station) != fDiFX2VexStationCodes.end()){fRefStation = fDiFX2VexStationCodes[difx_ref_station];}
    else
    {
        fRefStation = std::string() + (char) std::toupper(difx_ref_station[0]) + (char) std::tolower(difx_ref_station[1]);
        msg_warn("difx_interface", "could not locate vex 2-char ID associated with difx station: "<< difx_ref_station << ", assigning mk4 convention: "<< fRefStation <<eom);
    }
    
    if(fDiFX2VexStationCodes.find(difx_rem_station) != fDiFX2VexStationCodes.end()){fRemStation = fDiFX2VexStationCodes[difx_rem_station];}
    else
    {
        fRemStation = std::string() + (char) std::toupper(difx_rem_station[0]) + (char) std::tolower(difx_rem_station[1]);
        msg_warn("difx_interface", "could not locate vex 2-char ID associated with difx station: "<< difx_rem_station << ", assigning mk4 convention: "<< fRefStation <<eom);
    }
    
    
    //figure out the station names
    if(fDiFX2VexStationNames.find(difx_ref_station) != fDiFX2VexStationNames.end()){fRefStationName = fDiFX2VexStationNames[difx_ref_station];}
    else
    {
        fRefStationName = difx_ref_station;
        msg_warn("difx_interface", "could not locate vex station name associated with difx station: "<< difx_ref_station << ", using: "<< fRefStationName <<eom);
    }
    
    if(fDiFX2VexStationNames.find(difx_rem_station) != fDiFX2VexStationNames.end()){fRemStationName = fDiFX2VexStationNames[difx_rem_station];}
    else
    {
        fRemStationName = difx_rem_station;
        msg_warn("difx_interface", "could not locate vex station name associated with difx station: "<< difx_rem_station << ", using: "<< fRemStationName <<eom);
    }

    fBaselineName = fRefStation + fBaselineDelim + fRemStation;
    fRefStationMk4Id = fStationCodeMap->GetMk4IdFromStationCode(fRefStation);
    fRemStationMk4Id = fStationCodeMap->GetMk4IdFromStationCode(fRemStation);
    fBaselineShortName = fRefStationMk4Id + fRemStationMk4Id;

    //get the AP length (which config should we use if there is more than one?)
    fAPLength = (*fInput)["config"][0]["tInt"];

    //check number of polpairs
    fNPolPairs = fPolPairSet.size();
    if(fNPolPairs > 4){msg_warn("difx_interface", "more than 4 pol-products, detected (" << fNPolPairs <<") on baseline: " << fBaselineName << eom) ;}
    fNChannels = fFreqIndexSet.size();

    //check if the number of spectral points is the same for every channel correlated
    fNSpectralPoints = 0;
    if(!fSpecPointSet.empty()){fNSpectralPoints = *(fSpecPointSet.rbegin());}//grab the max # of spectral points
    //can only use channelized visibilities if every channel has the same number of spectral points
    fCanChannelize = true;
    if(fSpecPointSet.size() > 1 )
    {
        fCanChannelize = false;
        msg_error("difx_interface", "channels do not have same number of spectral points on baseline: " << fBaselineName << eom);
        msg_fatal("difx_interface", "un-channelized data not yet supported" << eom );
        std::exit(1);
    }

    //sort the individual visiblity records in time order
    fAPSet.clear();
    for(auto ppit = fPolPairSet.begin(); ppit != fPolPairSet.end(); ppit++)
    {
        std::string pp = *ppit;
        for(auto frqit = fFreqIndexSet.begin(); frqit != fFreqIndexSet.end(); frqit++)
        {
            int freqidx = *frqit;
            std::sort( fVisibilities[pp][freqidx].begin(), fVisibilities[pp][freqidx].end(), fTimePredicate);
            //msg_debug("difx_interface", "Number of APs on "<<fBaselineName<<" with ID: "<<fBaselineID<<" for polprod: "<<pp<<" and freqidx: "<<freqidx<< " is "<< fVisibilities[pp][freqidx].size() << eom);
            fAPSet.insert( fVisibilities[pp][freqidx].size() );
        }
    }

    //TODO -- what happens when we have missing/extra APs (should we truncate as below, or pad?)
    //determine the number of APs
    fNAPs = 0;
    if(!fAPSet.empty()){fNAPs = *(fAPSet.begin());} //sets are sorted in ascending order, so grab the max from end
    if(fAPSet.size() > 1 )
    {
        msg_error("difx_interface", "channels do not have same number of APs on baseline: " <<
            fBaselineName <<", ID: "<<fBaselineID<<" truncating to lowest common number of APs: "<< fNAPs << "."<< eom);
        // msg_error("difx_interface", "channels do not have same number of APs on baseline: " << fBaselineName <<" will zero pad-out to max AP: "<< fNAPs << "."<< eom);
        // for(auto apit = fAPSet.begin(); apit != fAPSet.end(); apit++)
        // {
        //     std::cout<<"ap: "<<*apit<<std::endl;
        // }
    }



    //construct the table of frequencies for this baseline and sort in asscending order
    fNChannels = fFreqIndexSet.size();
    for(auto it = fFreqIndexSet.begin(); it != fFreqIndexSet.end(); it++)
    {
        int freqidx = *it;
        mho_json freq = (*fInput)["freq"][freqidx];
        fBaselineFreqs.push_back(  std::make_pair(freqidx,freq) );
    }
    std::sort(fBaselineFreqs.begin(), fBaselineFreqs.end(), fFreqPredicate);

    msg_debug("difx_interface", "data dimensions of baseline: "<<
              fBaselineID << " - " << fBaselineName <<" are (" << fNPolPairs << ", " << fNChannels
              << ", " << fNAPs << ", " << fNSpectralPoints <<")" << eom);


    //loop over the datastreams, find which ones correspond to the stations on
    //this baseline and figure out the #number of quantizations bits for
    //the reference (ant1) and remote station (ant2)
    //this implicity assumes all datastreams from one antenna are sampled with
    //the same number of bits
    fRemStationBits = 0;
    fRefStationBits = 0;
    std::size_t nDatastreams = (*fInput)["datastream"].size();

    for(std::size_t d = 0; d<nDatastreams; d++)
    {
        int ant_id = (*fInput)["datastream"][d]["antennaId"];
        if(ant_id == ant1) //reference station data stream
        {
            fRefStationBits =  (*fInput)["datastream"][d]["quantBits"];
        }
        if(ant_id == ant2) //remote station data stream
        {
            fRemStationBits = (*fInput)["datastream"][d]["quantBits"];
        }
        if(fRefStationBits != 0 && fRemStationBits != 0 ){break;}
    }

    if(ant1 != ant2) // cross-correlation
    {
        fScaleFactor = SCALE * fNBitsToFactor[fRefStationBits] * fNBitsToFactor[fRemStationBits];
    }
    else // auto-correlation
    {
        fScaleFactor = SCALE;
    }
}


void
MHO_DiFXBaselineProcessor::SetStationCodes(MHO_StationCodeMap* code_map)
{
    fStationCodeMap = code_map;
}



void
MHO_DiFXBaselineProcessor::ConstructVisibilityFileObjects()
{
    //fBaselineFreqs contains the ordered (ascending) list of channel frequencies
    //fVisibilities contains the pol-pair and time sorted visibilities
    Organize();

    //first construct a channelized visibility container
    if(fV){delete fV; fV = nullptr;}
    if(fW){delete fW; fW = nullptr;}

    if(fHaveBaselineData && fCanChannelize && fInput != nullptr)
    {
        //insert the difx input data as a json object
        fTags.SetTagValue("difx_input_json", *fInput);
        fTags.SetTagValue("root_code", fRootCode);
        //add a bug of tags from the visib/weight objects for ease of retrieval
        fTags.SetTagValue("difx_baseline_index", fBaselineID);
        fTags.SetTagValue("baseline", fBaselineName);
        fTags.SetTagValue("baseline_shortname", fBaselineShortName);
        fTags.SetTagValue("reference_station", fRefStation);
        fTags.SetTagValue("remote_station", fRemStation);
        fTags.SetTagValue("reference_station_name", fRefStationName);
        fTags.SetTagValue("remote_station_name", fRemStationName);
        fTags.SetTagValue("reference_station_mk4id", fRefStationMk4Id);
        fTags.SetTagValue("remote_station_mk4id", fRemStationMk4Id);
        fTags.SetTagValue("correlation_date", fCorrDate);
        fTags.SetTagValue("root_code", fRootCode);
        fTags.SetTagValue("origin", "difx");

        fV = new visibility_store_type();
        fW = new weight_store_type();

        //tags for the visibilities
        fV->Resize(fNPolPairs, fNChannels, fNAPs, fNSpectralPoints);
        fV->ZeroArray();
        fV->Insert(std::string("name"), std::string("visibilities"));
        fV->Insert(std::string("difx_baseline_index"), fBaselineID);
        fV->Insert(std::string("baseline"), fBaselineName);
        fV->Insert(std::string("baseline_shortname"), fBaselineShortName);
        fV->Insert(std::string("reference_station"), fRefStation);
        fV->Insert(std::string("remote_station"), fRemStation);
        fV->Insert(std::string("reference_station_name"), fRefStationName);
        fV->Insert(std::string("remote_station_name"), fRemStationName);
        fV->Insert(std::string("reference_station_mk4id"), fRefStationMk4Id);
        fV->Insert(std::string("remote_station_mk4id"), fRemStationMk4Id);
        fV->Insert(std::string("correlation_date"), fCorrDate);
        fV->Insert(std::string("root_code"), fRootCode);
        fV->Insert(std::string("origin"), "difx");

        //tags for the weights
        fW->Resize(fNPolPairs, fNChannels, fNAPs, 1); //fNSpectralPoints -- we only have 1 weight value for each AP, so set dimension along the spectral point axis to 1
        fW->ZeroArray();
        fW->Insert(std::string("name"), std::string("weights"));
        fW->Insert(std::string("difx_baseline_index"), fBaselineID);
        fW->Insert(std::string("baseline"), fBaselineName);
        fW->Insert(std::string("baseline_shortname"), fBaselineShortName);
        fW->Insert(std::string("reference_station"), fRefStation);
        fW->Insert(std::string("remote_station"), fRemStation);
        fW->Insert(std::string("reference_station_name"), fRefStationName);
        fW->Insert(std::string("remote_station_name"), fRemStationName);
        fW->Insert(std::string("reference_station_mk4id"), fRefStationMk4Id);
        fW->Insert(std::string("remote_station_mk4id"), fRemStationMk4Id);
        fW->Insert(std::string("correlation_date"), fCorrDate);
        fW->Insert(std::string("root_code"), fRootCode);
        fW->Insert(std::string("origin"), "difx");

        //polarization product axis
        auto* polprod_axis = &(std::get<POLPROD_AXIS>(*fV));
        auto* wpolprod_axis = &(std::get<POLPROD_AXIS>(*fW));
        polprod_axis->Insert(std::string("name"), std::string("polarization_product") );
        wpolprod_axis->Insert(std::string("name"), std::string("polarization_product") );

        //channel axis
        auto* ch_axis = &(std::get<CHANNEL_AXIS>(*fV));
        auto* wch_axis = &(std::get<CHANNEL_AXIS>(*fW));
        ch_axis->Insert(std::string("name"), std::string("channel") );
        wch_axis->Insert(std::string("name"), std::string("channel") );
        ch_axis->Insert(std::string("units"), std::string("MHz") );
        wch_axis->Insert(std::string("units"), std::string("MHz") );

        //AP axis
        auto* ap_axis = &(std::get<TIME_AXIS>(*fV));
        auto* wap_axis = &(std::get<TIME_AXIS>(*fW));
        ap_axis->Insert(std::string("name"), std::string("time") );
        wap_axis->Insert(std::string("name"), std::string("time") );
        ap_axis->Insert(std::string("units"), std::string("s") );
        wap_axis->Insert(std::string("units"), std::string("s") );

        //(sub-channel) frequency axis
        auto* sp_axis = &(std::get<FREQ_AXIS>(*fV));
        auto* wsp_axis = &(std::get<FREQ_AXIS>(*fW));
        sp_axis->Insert(std::string("name"), std::string("frequency") );
        wsp_axis->Insert(std::string("name"), std::string("frequency") );
        sp_axis->Insert(std::string("units"), std::string("MHz") );
        wsp_axis->Insert(std::string("units"), std::string("MHz") );


        int ppidx = 0;
        fFreqIndexSet.clear();
        for(auto ppit = fPolPairSet.begin(); ppit != fPolPairSet.end(); ppit++)
        {
            std::string pp = *ppit;
            polprod_axis->at(ppidx) = pp;
            wpolprod_axis->at(ppidx) = pp;

            //loop through baseline freqs in order, grabbing the associated channel
            //and populating the visibility container
            int chidx = 0;
            for(auto fqit = fBaselineFreqs.begin(); fqit != fBaselineFreqs.end(); fqit++) //loop though in freq (low -> high) order
            {
                int freqidx = fqit->first;
                mho_json dfreq = fqit->second;
                double sky_freq = dfreq["freq"];
                double bw = dfreq["bw"];
                std::string sideband = dfreq["sideband"];

                if(ppidx == 0) //only one label needed for each channel
                {
                    mho_json ch_label;
                    ch_label["sky_freq"] =  sky_freq;
                    ch_label["bandwidth"] = bw;
                    ch_label["net_sideband"] = sideband;
                    ch_label["difx_freqindex"] = freqidx; //probably ought to be more systematic about creating channel names
                    ch_label["channel"] = chidx; //channel position index
                    std::string fband = DetermineFreqGroup(sky_freq);
                    fFreqBandLabelSet.insert(fband);//keep track of all the freq-band labels we've inserted
                    ch_label["frequency_band"] = fband; //will be empty string if assignment can't be made

                     //TODO FIXME need to construct difx2mark4-style chan_id --
                    //or rather need to construct the chan_id which corresponds to the reference and remote station for this chunk
                    //this also needs to be able to support zoom bands

                    std::string ref_chan_id = ConstructMK4ChannelID(fband, chidx, sideband, pp[0]);
                    std::string rem_chan_id = ConstructMK4ChannelID(fband, chidx, sideband, pp[1]);

                    ch_label["mk4_channel_id"] = ref_chan_id + ":" + rem_chan_id;

                    ch_axis->at(chidx) = sky_freq; //channel axis is now sky frequency not chidx;
                    wch_axis->at(chidx) = sky_freq;
                    ch_axis->SetLabelObject(ch_label,chidx);
                    wch_axis->SetLabelObject(ch_label,chidx);

                    // MHO_IntervalLabel ch_label(chidx,chidx);
                    // ch_label.Insert(std::string("sky_freq"), sky_freq);
                    // ch_label.Insert(std::string("bandwidth"), bw);
                    // ch_label.Insert(std::string("net_sideband"), sideband);
                    // ch_label.Insert(std::string("difx_freqindex"), freqidx); //probably ought to be more systematic about creating channel names
                    // ch_label.Insert(std::string("channel"), chidx); //channel position index
                    //  //TODO FIXME need to construct difx2mark4-style chan_id --
                    // //or rather need to construct the chan_id which corresponds to the reference and remote station for this chunk
                    // //this also needs to be able to support zoom bands
                    // ch_label.Insert(std::string("chan_id"), std::string("placeholder"));
                    //
                    // ch_axis->at(chidx) = sky_freq; //channel axis is now sky frequency not chidx;
                    // wch_axis->at(chidx) = sky_freq; // chidx;
                    // ch_axis->InsertLabel(ch_label);
                    // wch_axis->InsertLabel(ch_label);
                }

                for(std::size_t ap = 0; ap<fNAPs; ap++)
                {
                    ap_axis->at(ap) = ap*fAPLength;
                    wap_axis->at(ap) = ap*fAPLength;
                    MHO_DiFXVisibilityRecord* visRec = fVisibilities[pp][freqidx][ap];
                    (*fW)(ppidx,chidx,ap,0) = visRec->dataweight; //set the data weight for this AP
                    wsp_axis->at(0) = 0;
                    for(std::size_t sp = 0; sp<fNSpectralPoints; sp++)
                    {
                        sp_axis->at(sp) = sp*(bw/fNSpectralPoints); //frequency offset from edge of channel
                        std::complex<double> tmp;
                        //for lower sideband flip axis and conjugate
                        //why?...difx2mark4 does this, but then fourfit inverts it?) //TODO VERIFY IF THIS IS NEEDED
                        if(sideband == "L"){tmp = std::conj( visRec->visdata[fNSpectralPoints-1-sp] );}
                        else{ tmp = visRec->visdata[sp]; }
                        (*fV)(ppidx,chidx,ap,sp) = tmp;
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
            if(!exe){msg_error("difx_interface", "failed to execute DSB channel detection on visibilities" << eom);}
        }
        
        MHO_DoubleSidebandChannelLabeler< weight_store_type > wt_dsb_detect;
        wt_dsb_detect.SetArgs(fW);
        bool winit = wt_dsb_detect.Initialize();
        if(winit)
        {
            bool exe = wt_dsb_detect.Execute();
            if(!exe){msg_error("difx_interface", "failed to execute DSB channel detection on weights" << eom);}
        }

        //the very last thing we do is to attach the pol-product set and freq-band set to the 'Tags' data
        //this is allow a program reading this file to determine this information without streaming
        //in the potentially very large visibility/weights data
        std::vector< std::string > fband_vec(fFreqBandLabelSet.begin(), fFreqBandLabelSet.end());
        std::vector< std::string > pprod_vec(fPolPairSet.begin(), fPolPairSet.end() );
        fTags.SetTagValue("frequency_band_set", fband_vec);
        fTags.SetTagValue("polarization_product_set", pprod_vec);
    }
    else
    {
        if(fHaveBaselineData) //not an error if there is no data
        {
            msg_error("difx_interface", "cannot channelize visibility data, as not all channels are equal length. Feature not yet supported" << eom );
        }
    }

    DeleteDiFXVisRecords();
};


void
MHO_DiFXBaselineProcessor::WriteVisibilityObjects(std::string output_dir)
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
    delete fV; fV = nullptr;
    delete fW; fW = nullptr;
};


void
MHO_DiFXBaselineProcessor::Clear()
{
    DeleteDiFXVisRecords();
    fPolPairSet.clear();
    fFreqIndexSet.clear();
    fSpecPointSet.clear();
    fVisibilities.clear();
    fBaselineFreqs.clear();
    if(fV){delete fV; fV = nullptr;}
    if(fW){delete fW; fW = nullptr;}
}

void
MHO_DiFXBaselineProcessor::DeleteDiFXVisRecords()
{
    //delete the visibility records
    for(std::size_t i=0; i<fRecords.size(); i++)
    {
        delete fRecords[i];
    }
    fRecords.clear();
    malloc_trim(0); //for lots of small objects this may be helpful to flush pages back to OS
}


std::string
MHO_DiFXBaselineProcessor::ConstructMK4ChannelID(std::string fgroup, int index, std::string sideband, char pol)
{
    std::stringstream ss;
    ss << fgroup;
    if(index < 10 ){ss << "0";} //pad with leading zero if less than 10
    ss << index;
    ss << sideband;
    ss << pol;
    return ss.str();
}

std::string
MHO_DiFXBaselineProcessor::DetermineFreqGroup(const double& freq)
{
    //decide based on sky-freq of channel (we ignore bandwidth/sideband for now)
    //so may not be correct for extra-wide channels...but if that is the case,
    //probably some customize fband settings should be passed by user to specify the freq band labeling
    std::string fband = "";
    double fband_width = 1e30;

    for(auto it = fFreqBands.begin(); it != fFreqBands.end(); it++)
    {
        std::string b = std::get<0>(*it);
        double low = std::get<1>(*it);
        double high = std::get<2>(*it);
        if(high < low){double tmp = low; low = high; high = tmp;} //swap if mis-ordered
        double width = high - low;
        if(freq < high && low < freq && width < fband_width)
        {
            fband = b;
            fband_width = width;
            //do not break here, so that narrower freq-bands will override larger ones
        }
    }
    return fband; //returns "" if no matching band assignment found
}

std::string 
MHO_DiFXBaselineProcessor::ConstructCorFileName(const std::string& output_dir, 
                                                const std::string& root_code, 
                                                const std::string& baseline,
                                                const std::string& baseline_shortname)
{
    std::string output_file = output_dir + "/" + baseline_shortname + "." + baseline + "." + root_code + ".cor";
    return output_file;
}

}//end namespace
