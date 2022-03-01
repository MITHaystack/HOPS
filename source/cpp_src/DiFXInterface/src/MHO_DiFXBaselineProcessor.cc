#include "MHO_DiFXBaselineProcessor.hh"

namespace hops 
{

MHO_DiFXBaselineProcessor::MHO_DiFXBaselineProcessor(){};

MHO_DiFXBaselineProcessor::~MHO_DiFXBaselineProcessor()
{
    //delete the visibility records
}

void 
MHO_DiFXBaselineProcessor::AddRecord(MHO_DiFXVisibilityRecord* record)
{
    if(fRecords.size() == 0){fBaselineID = record->baseline;}
    if(fBaselineID == record->baseline)
    {
        fRecords.push_back(record);
        fUniquePolPairs.insert( std::string(visRecord.polpair,2) );
    }
}




void 
MHO_DiFXBaselineProcessor::ConstructVisibilityFileObjects()
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
                    ch_label.Insert(std::string("difx_freqindex"), freqidx); //probably ought to be more systematic about creating channel names

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






}//end namespace