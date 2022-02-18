#include "MHO_DiFXScanProcessor.hh"

namespace hops 
{

MHO_DiFXScanProcessor::MHO_DiFXScanProcessor():
    fV(nullptr),
    fW(nullptr)
{};

MHO_DiFXScanProcessor::~MHO_DiFXScanProcessor()
{
    if(fV){delete fV;}
    if(fW){delete fW;}
    if(fDInput){deleteDifxInput(fDInput);}
};


void 
MHO_DiFXScanProcessor::ProcessScan(MHO_DiFXScanFileSet& fileSet)
{
    fFileSet = &fileSet;

    LoadInputFile(fileSet.fInputFile); //read .input file and build freq table

    //load the visibilities
    ClearVisibilityRecords();
    MHO_DiFXVisibilityProcessor visProcessor;
    visProcessor.SetFilename(fileSet.fVisibilityFileList[0]);
    visProcessor.ReadDIFX_File(fAllBaselineVisibilities, fAllBaselineUniquePolPairs);

    ConstructRootFileObject();

    for(auto it = fAllBaselineVisibilities.begin(); it != fAllBaselineVisibilities.end(); it++)
    {
        OrganizeBaseline(it->first);
        fCurrentBaselineIndex = it->first;
        ConstructVisibilityFileObjects();
        WriteVisibilityObjects();
    }

    ConstructStationFileObjects();


    //clear up and reset for next scan
    deleteDifxInput(fDInput);
    fDInput = nullptr;
}


void 
MHO_DiFXScanProcessor::OrganizeBaseline(int baseline)
{
    if(fAllBaselineVisibilities[baseline].size() == 0)
    {
        msg_warn("difx_interface", "no visiblity records available for baseline: " << baseline << eom);
        return;
    }

    fVisibilities.clear();
    fBaselineFreqs.clear(); 
    fPolPairSet.clear();
    fFreqIndexSet.clear();
    fAPSet.clear();
    fSpecPointSet.clear();

    fNPolPairs = 0;
    fNChannels = 0;
    fNAPs = 0;
    fNSpectralPoints = 0;

    //only have to pad APs if they are unequal across channels
    fPadAPs = false;
    //can only use channelized visibilities if every channel has the same number of spectral points
    fCanChannelize = true;

    //sort the visibility records of this baseline by pols and freqs (channels)
    //also keeping track of the set of frequencies available on this baseline
    for(auto it = fAllBaselineVisibilities[baseline].begin(); it != fAllBaselineVisibilities[baseline].end(); it++)
    {
        std::string pp( (*it)->polpair, 2);
        int freqidx = (*it)->freqindex;
        int points = (*it)->nchan;
        fFreqIndexSet.insert(freqidx);
        fPolPairSet.insert(pp);
        fSpecPointSet.insert(points);
        fVisibilities[pp][freqidx].push_back(*it);
    }

    fNPolPairs = fPolPairSet.size();
    if(fNPolPairs > 4){msg_warn("difx_interface", "More than 4 pol-products detected (" << fNPolPairs <<") on baseline: " << baseline << eom) ;}
    fNChannels = fFreqIndexSet.size();

    //construct the table of frequencies for this baseline and sort in asscending order
    for(auto it = fFreqIndexSet.begin(); it != fFreqIndexSet.end(); it++)
    {
        auto freq = fAllFreqTable.find(*it);
        if(freq != fAllFreqTable.end() )
        {
            fBaselineFreqs.push_back( std::make_pair(*it, freq->second ) );
        }
    }
    std::sort(fBaselineFreqs.begin(), fBaselineFreqs.end(), fFreqPredicate);

    //sort the individual visiblity records in time order 
    for(auto ppit = fPolPairSet.begin(); ppit != fPolPairSet.end(); ppit++)
    {
        std::string pp = *ppit;
        for(auto fqit = fBaselineFreqs.begin(); fqit != fBaselineFreqs.end(); fqit++)
        {
            int freqidx = fqit->first;
            std::sort( fVisibilities[pp][freqidx].begin(), fVisibilities[pp][freqidx].end(), fTimePredicate);
            fAPSet.insert( fVisibilities[pp][freqidx].size() );
        }
    }

    //determine the number of APs
    if(!fAPSet.empty())
    {
        //sets are sorted in ascending order, so grab the max from end
        fNAPs = *(fAPSet.rbegin());
    }

    if(fAPSet.size() > 1 )
    {
        fPadAPs = true;
        msg_error("difx_interface", "Channels do not have same number of APs on baseline: " << baseline << eom);
    }

    //determine the number of spectral points per channel
    if(!fSpecPointSet.empty())
    {
        fNSpectralPoints = *(fSpecPointSet.rbegin());//grab the max
    }

    if(fSpecPointSet.size() > 1 )
    {
        fCanChannelize = false;
        msg_error("difx_interface", "Channels do not have same number of spectral points on baseline: " << baseline << eom);
    }

    msg_debug("difx_interface", "data dimension of baseline: "<< 
              baseline <<" are (" << fNPolPairs << ", " << fNChannels 
              << ", " << fNAPs << ", " << fNSpectralPoints <<")" << eom);
}


void 
MHO_DiFXScanProcessor::ConstructVisibilityFileObjects()
{
	//fBaselineFreqs contains the ordered (ascending) list of channel frequencies 
	//fChannels contains the time sorted visibilities 

	//loop through baseline freqs in order, grabbing the associated channel
	//and populating the visibility container 

	//TODO verify that the channels all have the same number of spectral points!
	//if not then we need to use the flag visibility container 

    if(fCanChannelize)
    {
    	//first construct a channelized visibility container
        if(fV){delete fV; fV = nullptr;}
        if(fW){delete fW; fW = nullptr;}

        fV = new ch_baseline_data_type(); 
        fW = new ch_baseline_weight_type();

        fV->Resize(fNPolPairs, fNChannels, fNAPs, fNSpectralPoints);
        fV->ZeroArray();
        fV->Insert(std::string("name"), std::string("visibilities"));
        fV->Insert(std::string("difx_baseline_index"), fCurrentBaselineIndex);

        fW->Resize(fNPolPairs, fNChannels, fNAPs, fNSpectralPoints);
        fW->ZeroArray();
        fV->Insert(std::string("name"), std::string("weights"));
        fV->Insert(std::string("difx_baseline_index"), fCurrentBaselineIndex);

        // /* Straight from DiFX frequency table */
        // typedef struct
        // {
        // 	double freq;		/* (MHz) */
        // 	double bw;		/* (MHz) */
        // 	char sideband;		/* U or L -- net sideband */
        // 	int nChan;
        // 	int specAvg;		/* This is averaging within mpifxcorr  */
        // 	int overSamp;
        // 	int decimation;
        // 	int nTone;		/* Number of pulse cal tones */
        // 	int *tone;		/* Array of tone indices */
        // 	char rxName[DIFXIO_RX_NAME_LENGTH];
        // } DifxFreq;

        auto* polprod_axis = &(std::get<CH_POLPROD_AXIS>(*fV));
        auto* wpolprod_axis = &(std::get<CH_POLPROD_AXIS>(*fW));
        polprod_axis->Insert(std::string("name"), std::string("polarization_product") );
        wpolprod_axis->Insert(std::string("name"), std::string("polarization_product") );

        auto* ch_axis = &(std::get<CH_CHANNEL_AXIS>(*fV));
        auto* wch_axis = &(std::get<CH_CHANNEL_AXIS>(*fW));
        ch_axis->Insert(std::string("name"), std::string("channel") );
        wch_axis->Insert(std::string("name"), std::string("channel") );

        auto* ap_axis = &(std::get<CH_TIME_AXIS>(*fV));
        auto* wap_axis = &(std::get<CH_TIME_AXIS>(*fW));
        ap_axis->Insert(std::string("name"), std::string("time") );
        wap_axis->Insert(std::string("name"), std::string("time") );
        ap_axis->Insert(std::string("units"), std::string("s") );
        wap_axis->Insert(std::string("units"), std::string("s") );

        auto* sp_axis = &(std::get<CH_FREQ_AXIS>(*fV));
        auto* wsp_axis = &(std::get<CH_FREQ_AXIS>(*fW));
        sp_axis->Insert(std::string("name"), std::string("frequency") );
        wsp_axis->Insert(std::string("name"), std::string("frequency") );
        sp_axis->Insert(std::string("units"), std::string("MHz") );
        wsp_axis->Insert(std::string("units"), std::string("MHz") );


        int ppidx = 0;
        for(auto ppit = fPolPairSet.begin(); ppit != fPolPairSet.end(); ppit++)
        {
            std::string pp = *ppit;
            //std::cout<<"pol = "<<pp<<std::endl;

            polprod_axis->at(ppidx) = pp;
            wpolprod_axis->at(ppidx) = pp;
            int chidx = 0;
            for(auto fqit = fBaselineFreqs.begin(); fqit != fBaselineFreqs.end(); fqit++)
            {
                int freqidx = fqit->first;
                DifxFreq* dfreq = fqit->second;
                double sky_freq = dfreq->freq;
                double bw = dfreq->bw; 
                char sideband = dfreq->sideband;

                if(ppidx == 0) //only one label needed for each channel
                {
                    MHO_IntervalLabel ch_label(chidx,chidx);
                    ch_label.Insert(std::string("sky_freq"), sky_freq);
                    ch_label.Insert(std::string("bandwidth"), bw);
                    ch_label.Insert(std::string("net_sideband"), std::string(&sideband,1) );
                    ch_label.Insert(std::string("channel"), freqidx); //probably ought to be more systematic about creating channel names

                    ch_axis->at(chidx) = chidx;
                    wch_axis->at(chidx) = chidx;
                    ch_axis->InsertLabel(ch_label);
                    wch_axis->InsertLabel(ch_label);
                }

                for(std::size_t ap = 0; ap<fVisibilities[pp][freqidx].size(); ap++)
                {
                    ap_axis->at(ap) = 0.0; //TODO FIXME -- compute ap*ap_length
                    wap_axis->at(ap) = 0.0; 
                    MHO_DiFXVisibilityRecord* visRec = fVisibilities[pp][freqidx][ap];
                    for(std::size_t sp = 0; sp<fNSpectralPoints; sp++)
                    {
                        sp_axis->at(sp) = sp*(bw/fNSpectralPoints); //frequency offset from edge of channel
                        wsp_axis->at(sp) = sp*(bw/fNSpectralPoints);
                        (*fW)(ppidx,chidx,ap,sp) = visRec->dataweight; //data weights don't need spectral point weighting?
                        if(sideband == 'L')
                        {
                            //flip axis and conjugate (why?...difx2mark4 does this, but then fourfit inverts it?) //TODO VERIFY
                            std::complex<double> tmp =  visRec->visdata[fNSpectralPoints-1-sp];
                            (*fV)(ppidx,chidx,ap,sp) = std::conj(tmp);
                        }
                        else
                        {
                            //treat as upper sideband
                            (*fV)(ppidx,chidx,ap,sp) = visRec->visdata[sp];
                            std::cout<<(*fV)(ppidx,chidx,ap,sp)<<std::endl;
                        }

                    }
                }
                chidx++;
            }
            ppidx++;
        }
    }
    else 
    {
        msg_error("difx_interface", "cannot channelize visibility data, as not all channels are equal lenght. Feature not yet supported" << eom );
    }

};


























// void 
// MHO_DiFXScanProcessor::ReadPCAL_File(std::string filename)
// {
//     //TODO 
// }
// 
// void 
// MHO_DiFXScanProcessor::ReadIM_File(std::string filename)
// {
//     //TODO
// }


void 
MHO_DiFXScanProcessor::LoadInputFile(std::string filename)
{
    //TODO FIXME - Why does this sometimes fail for DiFX versions <2.6 
    //when the .threads file is missing??
    DifxInput* fDInput = loadDifxInput(filename.c_str());

    //lets build the freq table 
    fAllFreqTable.clear();
    for(int i=0; i<fDInput->nFreq; i++){fAllFreqTable[i] = &(fDInput->freq[i]);}
}


void 
MHO_DiFXScanProcessor::ConstructRootFileObject()
{

};

void 
MHO_DiFXScanProcessor::ConstructStationFileObjects()
{

};

void 
MHO_DiFXScanProcessor::WriteVisibilityObjects()
{
    //construct output file name (eventually figure out how to construct the baseline name)
    std::stringstream ss;
    ss << fCurrentBaselineIndex;
    std::string root_code = "dummy"; //TODO replace with actual 'root' code
    std::string output_file = fFileSet->fOutputBaseDirectory + "/" + ss.str() + "." + root_code + ".cor";

    MHO_BinaryFileInterface inter;
    bool status = inter.OpenToWrite(output_file);
    if(status)
    {
        uint32_t label = 0xFFFFFFFF; //someday make this mean something
        inter.Write(*fV, "vis", label);
        inter.Write(*fW, "weight", label);
        inter.Close();
    }
    else
    {
        msg_error("file", "Error opening corel output file: " << output_file << eom);
    }

    inter.Close();

    delete fV; fV = nullptr;
    delete fW; fW = nullptr;
};


void 
MHO_DiFXScanProcessor::ClearVisibilityRecords()
{
    for(auto it = fAllBaselineVisibilities.begin(); it != fAllBaselineVisibilities.end(); it++)
    {
        for(std::size_t i = 0; i < it->second.size(); i++)
        {
            MHO_DiFXVisibilityRecord* ptr = it->second[i];
            delete ptr;
        }
    }
    fAllBaselineVisibilities.clear();
    fAllBaselineUniquePolPairs.clear();
}


}
