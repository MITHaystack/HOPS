#include "MHO_DiFXScanProcessor.hh"

namespace hops 
{

MHO_DiFXScanProcessor::MHO_DiFXScanProcessor(){};

MHO_DiFXScanProcessor::~MHO_DiFXScanProcessor(){};


void 
MHO_DiFXScanProcessor::ProcessScan(MHO_DiFXScanFileSet& fileSet)
{
    //read the input file and construct freq table
    LoadInputFile(fileSet.fInputFile);

    //read the Swinburne file (just one for now)
    std::cout<<"PROCESSING FIRST SCAN ONLY"<<std::endl;
    ReadDIFX_File(fileSet.fVisibilityFileList[0]);

    for(auto it = fAllBaselineVisibilities.begin(); it != fAllBaselineVisibilities.end(); it++)
    {
        OrganizeBaseline(it->first);
    }

    ConstructRootFileObject();
    ConstructStationFileObjects();
    ConstructVisiblityFileObjects();
    WriteScanObjects();

    //clear up an reset for next scan
    //deleteDifxInput(fDInput);
    //fDInput = nullptr;
}

void 
MHO_DiFXScanProcessor::ReadDIFX_File(std::string filename)
{
    //read the visibilities and allocate memory to store them as we go
    fAllBaselineVisibilities.clear();
    MHO_DiFXVisibilityRecord visRecord;

    //open file for binary reading
    std::fstream vFile;
    vFile.open(filename.c_str(), std::fstream::in | std::ios::binary);
    if( !vFile.is_open() || !vFile.good() )
    {
        msg_error("file", "Failed to open visibility file: "  << filename << " for reading." << eom);
    }

    std::size_t n_records = 0;
    bool keep_reading = true;
    while(keep_reading && vFile.good())
    {
        visRecord.Reset();
        vFile.read( reinterpret_cast<char*>( &(visRecord.sync) ), sizeof(int) );

        if( !(vFile.good() ) )
        {
            msg_error("difx_interface", "Could not read Swinburne file: " << filename << eom);
            break;
        }

        if (visRecord.sync == VISRECORD_SYNC_WORD_DIFX1) //old style ascii header, bad
        {
            msg_error("difx_interface", "Cannot read DiFX 1.x data." << eom );
            break;
        }

        if(visRecord.sync == VISRECORD_SYNC_WORD_DIFX2) //new style binary header, ok
        {
            vFile.read( reinterpret_cast<char*>(&(visRecord.headerversion) ), sizeof(int) );
            if(visRecord.headerversion == 1) //new style binary header
            {
                vFile.read( reinterpret_cast<char*>(&visRecord.baseline), sizeof(int) );
                vFile.read( reinterpret_cast<char*>(&visRecord.mjd), sizeof(int) );
                vFile.read( reinterpret_cast<char*>(&visRecord.seconds), sizeof(double) );
                vFile.read( reinterpret_cast<char*>( &visRecord.configindex), sizeof(int) );
                vFile.read( reinterpret_cast<char*>(&visRecord.sourceindex), sizeof(int) );
                vFile.read( reinterpret_cast<char*>(&visRecord.freqindex), sizeof(int) ); 
                vFile.read( reinterpret_cast<char*>(visRecord.polpair), 2*sizeof(char) );
                vFile.read( reinterpret_cast<char*>(&visRecord.pulsarbin), sizeof(int) ); 
                vFile.read( reinterpret_cast<char*>(&visRecord.dataweight), sizeof(double) );
                vFile.read( reinterpret_cast<char*>(visRecord.uvw), 3*sizeof(double) );

                //parcel out the visibilities one at a time (for now)
                //we may want to cache the npoint associated with each baseline+frequency in a map
                //so we can just grab them all at once someday if we think that may speed things up
                std::size_t npoints = 0;
                while(true)
                {
                    MHO_VisibilityChunk chunk;
                    vFile.read( reinterpret_cast<char*>(&chunk), sizeof(MHO_VisibilityChunk) );
                    //verify we haven't smacked into the sync word 
                    if(vFile.good())
                    {
                        if(chunk.sync_test[0] != VISRECORD_SYNC_WORD_DIFX2 )
                        {
                            npoints++;
                            visRecord.visdata.push_back( std::complex<float>(chunk.values[0], chunk.values[1] ) );
                        }
                        else
                        {
                            //"Wait! Lemme back up a minute..."
                            vFile.seekg( -1*sizeof(MHO_VisibilityChunk), std::ios_base::cur);
                            break;
                        }
                    }
                    else{ keep_reading = false; break;} //hit EOF
                }
                n_records++;
                visRecord.nchan = npoints;
                fAllBaselineVisibilities[visRecord.baseline].push_back( new MHO_DiFXVisibilityRecord(visRecord) );
                fBaselineUniquePolPairs[visRecord.baseline].insert( std::string(visRecord.polpair,2) ); //keep track of the polpairs

                if(n_records%1000 == 0){std::cout<<"read "<<n_records<<" records with "<< npoints <<" spectral points"<<std::endl;}
            }
        }
    }

    std::cout<<"read data from "<<fAllBaselineVisibilities.size()<<" baselines "<<std::endl;
    for(auto it = fAllBaselineVisibilities.begin(); it != fAllBaselineVisibilities.end(); it++)
    {
        std::cout<<" baseline: "<< it->first <<" has: "<< it->second.size()<<" records"<<std::endl;
    }

    //close the Swinburne file
    vFile.close();
}
















void 
MHO_DiFXScanProcessor::OrganizeBaseline(int baseline)
{
    //organize all of the visibility records of this baseline by time and frequency
    //fChannels.clear();
    fVisibilities.clear();
    fBaselineFreqs.clear(); 

    std::set<int> freqIndexSet;

/*
    //sort the visibility records into the appropriate channel 
    std::cout<<"sorting vis records into channels"<<std::endl;
    for(auto it = fAllBaselineVisibilities[baseline].begin(); it != fAllBaselineVisibilities[baseline].end(); it++)
    {
        int freqindex = it->freqindex;
        fChannels[freqindex].push_back(*it); //we could be more memory efficient if we just used pointers to the vis records
        auto freq = fAllFreqTable.find(freqindex);
        if(freq != fAllFreqTable.end() )
        {
            auto check = freqIndexSet.find(freqindex); //check if it is already present (due to another polpair)
            if(check == freqIndexSet.end())
            {
                fBaselineFreqs.push_back( std::make_pair(it->freqindex, freq->second ) );
                freqIndexSet.insert(it->freqindex);
            }
        }
        else
        {
            msg_error("difx_interface", "Could not locate frequency with index: " <<  it->freqindex << eom);
        }
    }
    std::cout<<"there are "<<fChannels.size()<<" channels "<<std::endl;


    std::sort(fBaselineFreqs.begin(), fBaselineFreqs.end(), fFreqPredicate);
    for(auto it = fBaselineFreqs.begin(); it != fBaselineFreqs.end(); it++)
    {
        std::cout<<"baseline frequency with index: "<< it->first<<std::endl;
        printDifxFreq(it->second);
    }


    //sort the visibility records by time (ascending order) with the timestamp predicate
    VisRecordTimeLess pred;
    for(auto it = fChannels.begin(); it != fChannels.end(); it++)
    {
        std::sort( it->second.begin(), it->second.end(), fTimePredicate);
        std::cout<<"sorting channel: "<<it->first<<" with "<< (*(it->second.begin())).visdata.size() <<" spectral points and time length: "<< it->second.size()<<std::endl;
    }
    std::cout<<"done sorting all channels"<<std::endl;


    std::cout<<"We have: "<<fUniquePolPairs.size()<<" pol pairs as follows:" <<std::endl;
    for(auto it = fUniquePolPairs.begin(); it != fUniquePolPairs.end(); it++){std::cout<<*it<<std::endl;}

*/
}





























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
    //TODO FIXME - Why does this sometimes fail when the .threads file is missing??
    DifxInput* fDInput = loadDifxInput(filename.c_str());

    //lets build the freq table 
    fAllFreqTable.clear();
    for(int i=0; i<fDInput->nFreq; i++){fAllFreqTable[i] = &(fDInput->freq[i]);}

    // printDifxInput(fDInput); //debug
    // 
    // std::cout<<"-----------------------------------"<<std::endl;
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
MHO_DiFXScanProcessor::ConstructVisiblityFileObjects()
{
	//fBaselineFreqs contains the ordered (ascending) list of channel frequencies 
	//fChannels contains the time sorted visibilities 

	//loop through baseline freqs in order, grabbing the associated channel
	//and populating the visibility container 

	//TODO verify that the channels all have the same number of spectral points!
	//if not then we need to use the flag visibility container 


	//first construct a channelized visibility container 
/*
	ch_baseline_data_type* vis = new ch_baseline_data_type();
        vis.Resize(fUniquePolPairs.size(), NCHANS, NAPS, NSPECPTS);	


	//now loop and fill it up
	for(auto it = fBaselineFreqs.begin(); it != fBaselineFreqs.end(); it++)
	{
		int index = it->first;
		DifxFreq* freq = it->second;
		


	}
    std::map<int, std::vector<MHO_DiFXVisibilityRecord> > fChannels;
        //maps freqindex to the difx frequency description
        std::map<int, DifxFreq*> fAllFreqTable;
        //list of channel frequencies for this baseline, sorted in ascending order (freq)
        std::vector< std::pair<int, DifxFreq*> > fBaselineFreqs;
*/

};

void 
MHO_DiFXScanProcessor::WriteScanObjects()
{

};

}
