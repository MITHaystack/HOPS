#include "MHO_DiFXVisibilityProcessor.hh"

namespace hops
{

//DiFX visibility record sync words, see difxio/parsevis.h.
static const int32_t VISRECORD_SYNC_WORD_DIFX1 = (('B' << 24) + ('A' << 16) + ('S' << 8) + 'E');
static const int32_t VISRECORD_SYNC_WORD_DIFX2 = (int32_t)0xFF00FF00;

MHO_DiFXVisibilityProcessor::MHO_DiFXVisibilityProcessor()
{
    //here so we can pass these parameters to the baseline processors upon construction (as map entries)
    fFreqBands.clear();
    fOnlyFreqGroups.clear();
    fSelectByBandwidth = false;
    fOnlyBandwidth = 0;
    fInput = nullptr;
    fZoomFreqIndices.clear();
    fApplyFlags = false;
}

void MHO_DiFXVisibilityProcessor::ReadDIFXFile(std::map< int, MHO_DiFXBaselineProcessor >& allBaselineVisibilities)
{
    //assume allBaselineVisibilities and allBaselineUniquePolPairs are empty to start
    MHO_DiFXVisibilityRecord visRecord;

    //open file for binary reading
    std::fstream vFile;
    vFile.open(fFilename.c_str(), std::fstream::in | std::ios::binary);
    if(!vFile.is_open() || !vFile.good())
    {
        msg_error("file", "failed to open visibility file: " << fFilename << " for reading." << eom);
    }

    fFlags.clear();
    if(fApplyFlags)
    {
        LoadFlags();
    }

    std::size_t n_records = 0;
    std::size_t n_flagged = 0;
    bool logged_first_record = false;
    bool keep_reading = true;
    while(keep_reading && vFile.good())
    {
        visRecord.Reset();
        vFile.read(reinterpret_cast< char* >(&(visRecord.sync)), sizeof(int));

        if(!(vFile.good()))
        {
            msg_debug("difx_interface",
                      "stopped reading Swinburne file: " << fFilename << " after " << n_records << " records." << eom);
            break;
        }

        if(visRecord.sync == VISRECORD_SYNC_WORD_DIFX1) //old style ascii header, bad
        {
            msg_error("difx_interface", "cannot read DiFX 1.x data." << eom);
            break;
        }

        if(visRecord.sync == VISRECORD_SYNC_WORD_DIFX2) //new style binary header, ok
        {
            vFile.read(reinterpret_cast< char* >(&(visRecord.headerversion)), sizeof(int));
            if(visRecord.headerversion == 1) //new style binary header
            {
                vFile.read(reinterpret_cast< char* >(&visRecord.baseline), sizeof(int));
                vFile.read(reinterpret_cast< char* >(&visRecord.mjd), sizeof(int));
                vFile.read(reinterpret_cast< char* >(&visRecord.seconds), sizeof(double));
                vFile.read(reinterpret_cast< char* >(&visRecord.configindex), sizeof(int));
                vFile.read(reinterpret_cast< char* >(&visRecord.sourceindex), sizeof(int));
                vFile.read(reinterpret_cast< char* >(&visRecord.freqindex), sizeof(int));
                vFile.read(reinterpret_cast< char* >(visRecord.polpair), 2 * sizeof(char));
                vFile.read(reinterpret_cast< char* >(&visRecord.pulsarbin), sizeof(int));
                vFile.read(reinterpret_cast< char* >(&visRecord.dataweight), sizeof(double));
                vFile.read(reinterpret_cast< char* >(visRecord.uvw), 3 * sizeof(double));

                auto nChanIt = fNChannelsMap.find(std::make_pair(visRecord.baseline, visRecord.freqindex));
                if(nChanIt != fNChannelsMap.end())
                {
                    visRecord.nchan = nChanIt->second;
                    visRecord.visdata.resize(visRecord.nchan);
                    vFile.read(reinterpret_cast< char* >(&(visRecord.visdata[0])),
                               visRecord.nchan * sizeof(MHO_VisibilityChunk));
                    if(!vFile.good())
                    {
                        keep_reading = false;
                    }
                }
                else
                {
                    //have to parcel out the visibilities one at a time
                    std::size_t npoints = 0;
                    while(true)
                    {
                        MHO_VisibilityChunk chunk;
                        vFile.read(reinterpret_cast< char* >(&chunk), sizeof(MHO_VisibilityChunk));
                        //verify we haven't smacked into the sync word
                        if(vFile.good())
                        {
                            if(chunk.sync_test[0] != VISRECORD_SYNC_WORD_DIFX2)
                            {
                                npoints++;
                                visRecord.visdata.push_back(std::complex< float >(chunk.values[0], chunk.values[1]));
                            }
                            else
                            {
                                //"Wait! Lemme back up a minute..."
                                vFile.seekg(-1 * sizeof(MHO_VisibilityChunk), std::ios_base::cur);
                                break;
                            }
                        }
                        else
                        {
                            keep_reading = false;
                            break;
                        } //hit EOF
                    }
                    n_records++;
                    visRecord.nchan = npoints;
                    //cache the n spectral points associated with each baseline+frequency in a map
                    //so we can just grab them all at once on the next encounter
                    fNChannelsMap[std::make_pair(visRecord.baseline, visRecord.freqindex)] = visRecord.nchan;
                }

                if(!logged_first_record)
                {
                    msg_debug("difx_interface", "first visibility record: baseline="
                                                    << visRecord.baseline << ", freqindex=" << visRecord.freqindex
                                                    << ", polpair=" << std::string(visRecord.polpair, 2) << ", nchan="
                                                    << visRecord.nchan << ", configindex=" << visRecord.configindex << eom);
                    logged_first_record = true;
                }

                if(IsRecordFlagged(visRecord))
                {
                    n_flagged++; //increment flagged record counter
                    continue;
                }

                //first time seeing this baseline, so pass these parameters on construction
                if(allBaselineVisibilities.find(visRecord.baseline) == allBaselineVisibilities.end())
                {
                    allBaselineVisibilities[visRecord.baseline].SetDiFXInputData(fInput);
                    if(fFreqBands.size() != 0)
                    {
                        allBaselineVisibilities[visRecord.baseline].SetFrequencyBands(fFreqBands);
                    }
                    if(fOnlyFreqGroups.size() != 0)
                    {
                        allBaselineVisibilities[visRecord.baseline].SetFreqGroups(fOnlyFreqGroups);
                    }
                    if(fSelectByBandwidth)
                    {
                        allBaselineVisibilities[visRecord.baseline].SetOnlyBandwidth(fOnlyBandwidth);
                    }
                    if(!fZoomFreqIndices.empty())
                    {
                        allBaselineVisibilities[visRecord.baseline].SetZoomFreqIndices(fZoomFreqIndices);
                    }
                }
                //add the record to the appropriate baseline
                allBaselineVisibilities[visRecord.baseline].AddRecord(new MHO_DiFXVisibilityRecord(visRecord));
            }
            else
            {
                msg_error("difx_interface", "encountered unsupported header version: "
                                                << visRecord.headerversion
                                                << ", while reading visibility record, expected version: 1" << eom);
            }
        }
    }

    msg_debug("difx_interface", "read " << n_records << " visibility records from " << allBaselineVisibilities.size()
                                        << " baselines, from: " << fFilename << eom);

    if(fApplyFlags)
    {
        msg_info("difx_interface", "excluded " << n_flagged << " visibility records maked in .flag file, from: "
                                               << fFilename << eom);
    }

    //close the Swinburne file
    vFile.close();
}

void MHO_DiFXVisibilityProcessor::LoadFlags()
{
    if(fInput == nullptr || !fInput->contains("job"))
    {
        msg_warn("difx_interface", "no job/flag data in the difx input json, flags will not be applied" << eom);
        return;
    }
    for(const auto& job : (*fInput)["job"])
    {
        if(!job.contains("flag"))
        {
            continue;
        }
        for(const auto& flag : job["flag"])
        {
            fFlags.push_back(std::make_tuple(flag["mjd1"].get< double >(), flag["mjd2"].get< double >(), flag["antennaId"].get< int >()));
        }
    }
    msg_debug("difx_interface", "loaded " << fFlags.size() << " a priori antenna flags" << eom);
}

bool MHO_DiFXVisibilityProcessor::IsRecordFlagged(const MHO_DiFXVisibilityRecord& visRecord) const
{
    if(fFlags.empty())
    {
        return false;
    }
    //baseline = 256*(a1+1) + (a2+1),
    //assuming record time is the center of the AP
    int a1 = visRecord.baseline / 256 - 1;
    int a2 = visRecord.baseline % 256 - 1;
    double t = visRecord.mjd + visRecord.seconds / 86400.0;
    for(const auto& flag : fFlags)
    {
        if(std::get< 0 >(flag) <= t && t <= std::get< 1 >(flag) && (std::get< 2 >(flag) == a1 || std::get< 2 >(flag) == a2))
        {
            return true;
        }
    }
    return false;
}

} // namespace hops
