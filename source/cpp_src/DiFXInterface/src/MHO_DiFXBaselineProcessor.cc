#include "MHO_DiFXBaselineProcessor.hh"
#include "MHO_BinaryFileInterface.hh"

#include "MHO_VisibilityPrecisionDownCaster.hh"
#include "MHO_WeightPrecisionDownCaster.hh"

#include <cctype>
#include <cmath>

#define DIFX_BASE2ANT 256
#define SCALE 10000.0

namespace hops
{

MHO_DiFXBaselineProcessor::MHO_DiFXBaselineProcessor():
    fInput(nullptr),
    fV(nullptr),
    fW(nullptr),
    fStationCodeMap(nullptr)
{
    fRootCode = "unknown";
    fRescale = true;
    fScaleFactor = 1.0;

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
    if(fBaselineID == record->baseline)
    {
        std::string pp(record->polpair,2);
        int freqidx = record->freqindex;
        int points = record->nchan;
        fPolPairSet.insert(pp);
        fFreqIndexSet.insert(freqidx);
        fSpecPointSet.insert(points);
        fRecords.push_back(record);
        fVisibilities[pp][freqidx].push_back(record);
    }
}


void
MHO_DiFXBaselineProcessor::Organize()
{
    if(fRecords.size() == 0)
    {
        msg_debug("difx_interface", "no visiblity records available for baseline: " << fBaselineID << eom);
        return;
    }

    if(fInput == nullptr)
    {
        msg_warn("difx_interface", "no difx input data set for baseline: " << fBaselineID << eom);
        return;
    }

    //first figure out the baseline name (CHECK THIS)
    /* The baseline number (256*A1 + A2, 1 indexed) */
    int ant1 = (fBaselineID / DIFX_BASE2ANT) - 1;
    int ant2 = (fBaselineID % DIFX_BASE2ANT) - 1;
    fRefStation = (*fInput)["antenna"][ant1]["name"];
    fRemStation = (*fInput)["antenna"][ant2]["name"];

    //convert the 2-char codes from all-caps to mk4-convention of 1-cap, 1-lower case
    fRefStation = std::string() + (char) std::toupper(fRefStation[0]) + (char) std::tolower(fRefStation[1]);
    fRemStation = std::string() + (char) std::toupper(fRemStation[0]) + (char) std::tolower(fRemStation[1]);

    fBaselineName = fRefStation + ":" + fRemStation;
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
            fAPSet.insert( fVisibilities[pp][freqidx].size() );
        }
    }

    //determine the number of APs
    fNAPs = 0;
    if(!fAPSet.empty()){fNAPs = *(fAPSet.rbegin());} //sets are sorted in ascending order, so grab the max from end
    if(fAPSet.size() > 1 )
    {
        msg_error("difx_interface", "channels do not have same number of APs on baseline: " << fBaselineName <<" will zero pad-out to max AP: "<< fNAPs << "."<< eom);
    }

    //TODO!! CHECK FOR MISSING APs

    //construct the table of frequencies for this baseline and sort in asscending order
    fNChannels = fFreqIndexSet.size();
    for(auto it = fFreqIndexSet.begin(); it != fFreqIndexSet.end(); it++)
    {
        int freqidx = *it;
        json freq = (*fInput)["freq"][freqidx];
        fBaselineFreqs.push_back(  std::make_pair(freqidx,freq) );
    }
    std::sort(fBaselineFreqs.begin(), fBaselineFreqs.end(), fFreqPredicate);

    msg_debug("difx_interface", "data dimension of baseline: "<<
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

    if(fCanChannelize && fInput != nullptr)
    {
        //insert the difx input data as a json string
        std::stringstream jss;
        jss << *fInput;
        fTags.SetTagValue("difx_input_json", jss.str());

    	//first construct a channelized visibility container
        if(fV){delete fV; fV = nullptr;}
        if(fW){delete fW; fW = nullptr;}

        fV = new visibility_type();
        fW = new weight_type();

        //tags for the visibilities
        fV->Resize(fNPolPairs, fNChannels, fNAPs, fNSpectralPoints);
        fV->ZeroArray();
        fV->Insert(std::string("name"), std::string("visibilities"));
        fV->Insert(std::string("difx_baseline_index"), fBaselineID);
        fV->Insert(std::string("baseline"), fBaselineName);
        fV->Insert(std::string("baseline_shortname"), fBaselineShortName);
        fV->Insert(std::string("reference_station"), fRefStation);
        fV->Insert(std::string("remote_station"), fRemStation);
        fV->Insert(std::string("reference_station_mk4id"), fRefStationMk4Id);
        fV->Insert(std::string("remote_station_mk4id"), fRemStationMk4Id);

        //tags for the weights
        fW->Resize(fNPolPairs, fNChannels, fNAPs, 1); //fNSpectralPoints -- we only have 1 weight value for each AP, so set dimension along the spectral point axis to 1
        fW->ZeroArray();
        fW->Insert(std::string("name"), std::string("weights"));
        fW->Insert(std::string("difx_baseline_index"), fBaselineID);
        fW->Insert(std::string("baseline"), fBaselineName);
        fW->Insert(std::string("baseline_shortname"), fBaselineShortName);
        fW->Insert(std::string("reference_station"), fRefStation);
        fW->Insert(std::string("remote_station"), fRemStation);
        fW->Insert(std::string("reference_station_mk4id"), fRefStationMk4Id);
        fW->Insert(std::string("remote_station_mk4id"), fRemStationMk4Id);

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
                json dfreq = fqit->second;
                double sky_freq = dfreq["freq"];
                double bw = dfreq["bw"];
                std::string sideband = dfreq["sideband"];

                if(ppidx == 0) //only one label needed for each channel
                {
                    MHO_IntervalLabel ch_label(chidx,chidx);
                    ch_label.Insert(std::string("sky_freq"), sky_freq);
                    ch_label.Insert(std::string("bandwidth"), bw);
                    ch_label.Insert(std::string("net_sideband"), sideband);
                    ch_label.Insert(std::string("difx_freqindex"), freqidx); //probably ought to be more systematic about creating channel names
                    ch_label.Insert(std::string("channel"), chidx); //channel position index
                     //TODO FIXME need to construct difx2mark4-style chan_id --
                    //or rather need to construct the chan_id which corresponds to the reference and remote station for this chunk
                    //this also needs to be able to support zoom bands
                    ch_label.Insert(std::string("chan_id"), std::string("placeholder"));

                    ch_axis->at(chidx) = sky_freq; //channel axis is now sky frequency not chidx;
                    wch_axis->at(chidx) = sky_freq; // chidx;
                    ch_axis->InsertLabel(ch_label);
                    wch_axis->InsertLabel(ch_label);
                }

                for(std::size_t ap = 0; ap<fVisibilities[pp][freqidx].size(); ap++)
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

    }
    else
    {
        msg_error("difx_interface", "cannot channelize visibility data, as not all channels are equal length. Feature not yet supported" << eom );
    }

};


void
MHO_DiFXBaselineProcessor::WriteVisibilityObjects(std::string output_dir)
{
    //apply difx2mark4 style factor and Van Vleck correction before writing out
    if(fRescale)
    {
        //apply a x10000 factor to convert to "Whitney's"
        // and apply Van Vleck n-bit statistics normalization factor (only 2x2, 1x1, and 1x2 bit supported)
        (*fV) *= fScaleFactor;
    }

    //now we down cast the double precision visibilities and weights from double to float
    //this is to save disk space (but can be disabled)
    MHO_VisibilityPrecisionDownCaster vdcaster;
    MHO_WeightPrecisionDownCaster wdcaster;

    visibility_store_type vis_out;
    weight_store_type weight_out;

    vdcaster.SetArgs(fV, &vis_out);
    vdcaster.Initialize();
    vdcaster.Execute();

    wdcaster.SetArgs(fW, &weight_out);
    wdcaster.Initialize();
    wdcaster.Execute();

    //construct output file name
    std::string root_code = fRootCode;
    std::string output_file = output_dir + "/" + fBaselineShortName + "." + root_code + ".cor";

    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(output_file);
    if(status)
    {
        uint32_t label = 0xFFFFFFFF; //someday make this mean something
        fTags.AddObjectUUID(vis_out.GetObjectUUID());
        fTags.AddObjectUUID(weight_out.GetObjectUUID());
        inter.Write(fTags, "tags", label);

        inter.Write(vis_out, "vis", label);
        inter.Write(weight_out, "weight", label);
        inter.Close();

        //TODO ADD AN OPTION TO EXPORT DOUBLE PRECISION DATA
        // uint32_t label = 0xFFFFFFFF; //someday make this mean something
        // fTags.AddObjectUUID(fV->GetObjectUUID());
        // fTags.AddObjectUUID(fW->GetObjectUUID());
        // inter.Write(fTags, "tags", label);
        //
        // inter.Write(*fV, "vis", label);
        // inter.Write(*fW, "weight", label);
        // inter.Close();
    }
    else
    {
        msg_error("file", "error opening corel output file: " << output_file << eom);
    }

    inter.Close();

    delete fV; fV = nullptr;
    delete fW; fW = nullptr;
};


void
MHO_DiFXBaselineProcessor::Clear()
{
    //delete the visibility records
    for(std::size_t i=0; i<fRecords.size(); i++)
    {
        delete fRecords[i];
    }
    fRecords.clear();
    fPolPairSet.clear();
    fFreqIndexSet.clear();
    fSpecPointSet.clear();
    fVisibilities.clear();
    fBaselineFreqs.clear();
    if(fV){delete fV; fV = nullptr;}
    if(fW){delete fW; fW = nullptr;}
}



}//end namespace
