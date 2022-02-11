#include "MHO_DiFXVisibilityProcessor.hh"

namespace hops
{

void 
MHO_DiFXVisibilityProcessor::ReadDIFX_File(std::map< int, std::vector< MHO_DiFXVisibilityRecord* > >& allBaselineVisibilities,
                                           std::map< int, std::set< std::string > >& allBaselineUniquePolPairs)
{
    //assume allBaselineVis and allBaselineUniquePolPairs are empty to start
    MHO_DiFXVisibilityRecord visRecord;

    //open file for binary reading
    std::fstream vFile;
    vFile.open(fFilename.c_str(), std::fstream::in | std::ios::binary);
    if( !vFile.is_open() || !vFile.good() )
    {
        msg_error("file", "Failed to open visibility file: "  << fFilename << " for reading." << eom);
    }

    std::size_t n_records = 0;
    bool keep_reading = true;
    while(keep_reading && vFile.good())
    {
        visRecord.Reset();
        vFile.read( reinterpret_cast<char*>( &(visRecord.sync) ), sizeof(int) );

        if( !(vFile.good() ) )
        {
            msg_error("difx_interface", "Could not read Swinburne file: " << fFilename << eom);
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

                auto nChanIt = fNChannelsMap.find( std::make_pair(visRecord.baseline, visRecord.freqindex) );
                if(nChanIt != fNChannelsMap.end() )
                {
                    visRecord.nchan = nChanIt->second;
                    visRecord.visdata.resize(visRecord.nchan);
                    vFile.read( reinterpret_cast<char*>(&(visRecord.visdata[0])), visRecord.nchan*sizeof(MHO_VisibilityChunk));
                    if(!vFile.good()){keep_reading = false;}
                }
                else 
                {
                    //have to parcel out the visibilities one at a time
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
                    //cache the n spectral points associated with each baseline+frequency in a map
                    //so we can just grab them all at once on the next encounter
                    fNChannelsMap[ std::make_pair(visRecord.baseline, visRecord.freqindex) ] = visRecord.nchan; 
                }

                allBaselineVisibilities[visRecord.baseline].push_back( new MHO_DiFXVisibilityRecord(visRecord) );
                allBaselineUniquePolPairs[visRecord.baseline].insert( std::string(visRecord.polpair,2) ); //keep track of the polpairs
            }
        }
    }

    msg_debug("difx_interface", "read " << n_records << " visibility records from " <<allBaselineVisibilities.size()<<" baselines, from: " << fFilename << eom);

    //close the Swinburne file
    vFile.close();
}



}