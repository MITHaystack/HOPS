#include "MHO_MK4CorelInterfaceReversed.hh"
#include "MHO_LegacyDateConverter.hh"

#include <algorithm>
#include <complex>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iomanip>

#ifndef HOPS3_USE_CXX
extern "C"
{
#endif
    #include "mk4_data.h"
    #include "mk4_dfio.h"
    #include "mk4_records.h"
#ifndef HOPS3_USE_CXX
}
#endif


namespace hops
{

MHO_MK4CorelInterfaceReversed::MHO_MK4CorelInterfaceReversed():
    fVisibilityData(nullptr), 
    fWeightData(nullptr), 
    fGeneratedCorel(nullptr)
{
    fNPPs = 0;
    fNAPs = 0; 
    fNChannels = 0;
    fNSpectral = 0;
    fChannelInfoList.clear();
    fPolProductToIndex.clear();
}

MHO_MK4CorelInterfaceReversed::~MHO_MK4CorelInterfaceReversed()
{
    if(fGeneratedCorel)
    {
        clear_mk4corel(fGeneratedCorel);
        free(fGeneratedCorel);
    }
}

struct mk4_corel* 
MHO_MK4CorelInterfaceReversed::GenerateCorelStructure()
{
    if(fVisibilityData && fWeightData)
    {
        fNPPs = fVisibilityData->GetDimension(POLPROD_AXIS);
        fNAPs = fVisibilityData->GetDimension(TIME_AXIS);
        fNChannels = fVisibilityData->GetDimension(CHANNEL_AXIS);
        std::size_t freq_points_per_channel = fVisibilityData->GetDimension(FREQ_AXIS);
        fNSpectral = freq_points_per_channel;
        
        msg_debug("mk4interface_reversed", "Using  data: " << fNPPs << " pol products, " 
                  << fNAPs << " APs, " << fNChannels << " channels, " << fNSpectral << " spectral points per channel" << eom);
        
        // Debug: Print actual container dimensions
        msg_debug("mk4interface_reversed", "Visibility container dimensions: " 
                  << fVisibilityData->GetDimension(POLPROD_AXIS) << "x"
                  << fVisibilityData->GetDimension(TIME_AXIS) << "x"
                  << fVisibilityData->GetDimension(CHANNEL_AXIS) << "x"
                  << fVisibilityData->GetDimension(FREQ_AXIS) << eom);
    }
    else
    {
        msg_error("mk4interface_reversed", "No visibility or weight data provided" << eom);
        return nullptr;
    }

    msg_debug("mk4interface_reversed", "Extracted " << fNChannels << " channels with " 
              << fNSpectral << " spectral points each" << eom);

    InitializeCorelStructure();
    GenerateType000();
    GenerateType100();
    GenerateType101Records();
    GenerateType120Records();

    return fGeneratedCorel;
}

void MHO_MK4CorelInterfaceReversed::InitializeCorelStructure()
{
    if(fGeneratedCorel)
    {
        clear_mk4corel(fGeneratedCorel);
        free(fGeneratedCorel);
    }

    fGeneratedCorel = (struct mk4_corel*) calloc(1, sizeof(struct mk4_corel));
    clear_mk4corel(fGeneratedCorel);
    
    // Allocate space for index records
    fGeneratedCorel->index_space = fNChannels*fNPPs;
    fGeneratedCorel->index = (struct index_tag*) calloc(fNChannels*fNPPs, sizeof(struct index_tag));
}


void MHO_MK4CorelInterfaceReversed::GenerateType000()
{
    if(!fGeneratedCorel) return;
    fGeneratedCorel->id = (struct type_000*) calloc(1, sizeof(struct type_000));
    struct type_000* id = fGeneratedCorel->id;
    init_000(id, const_cast<char*>(fOutputFile.c_str()) );
}


void MHO_MK4CorelInterfaceReversed::GenerateType100()
{
    if(!fGeneratedCorel) return;

    fGeneratedCorel->t100 = (struct type_100*) calloc(1, sizeof(struct type_100));
    struct type_100* t100 = fGeneratedCorel->t100;
    clear_100(t100);

    // Extract metadata from container tags
    std::string baseline_short = "";
    std::string start_time = "";
    std::string stop_time = "";
    std::string corr_date = "";
    std::string root_code = "";

    if(fVisibilityData->HasKey("baseline_shortname"))
    {
        fVisibilityData->Retrieve("baseline_shortname", baseline_short);
    }
    if(fVisibilityData->HasKey("start"))
    {
        fVisibilityData->Retrieve("start", start_time);
    }
    if(fVisibilityData->HasKey("stop"))
    {
        fVisibilityData->Retrieve("stop", stop_time);
    }
    if(fVisibilityData->HasKey("correlation_date"))
    {
        fVisibilityData->Retrieve("correlation_date", corr_date);
    }
    if(fVisibilityData->HasKey("root_code"))
    {
        fVisibilityData->Retrieve("root_code", root_code);
    }

    // Set baseline
    setstr(baseline_short, t100->baseline, 2);

    // Set root filename 
    std::string source_name = "dummy_src";
    if(!root_code.empty())
    {
        std::string rootname = source_name + ".." + root_code;
        setstr(rootname, t100->rootname, 34);
    }

    // Set correlation quality code (default to empty: "  ")
    setstr("  ", t100->qcode, 2);

    // Set processing percentage (default to 0%)
    t100->pct_done = 0.0;

    // // Convert date strings to mk4 date structures
    // if(!start_time.empty())
    // {
    //     t100->start = ConvertDateString(start_time);
    // }
    // if(!stop_time.empty())
    // {
    //     t100->stop = ConvertDateString(stop_time);
    // }
    // if(!corr_date.empty())
    // {
    //     t100->procdate = ConvertDateString(corr_date);
    // }

    // Set data record counts
    t100->ndrec = fNPPs * fNChannels * fNAPs;  // Total number of type_120 records
    t100->nindex = fNChannels;         // Number of type_101 records
    t100->nlags = fNSpectral;          // Number of spectral points
    t100->nblocks = 1;                 // Assuming single block per index

    msg_debug("mk4interface_reversed", "Generated type_100 record for baseline " << baseline_short << eom);
} 

void MHO_MK4CorelInterfaceReversed::GenerateType101Records()
{
    if(!fGeneratedCorel || !fGeneratedCorel->index) return;

    auto pp_ax = &(std::get<POLPROD_AXIS>(*fVisibilityData));
    auto chan_ax = &(std::get<CHANNEL_AXIS>(*fVisibilityData));

    MHO_Tokenizer tokenizer;
    tokenizer.SetDelimiter(":");
    std::vector< std::string> tokens;

    int count = 0;
    for(std::size_t pp = 0; pp < fNPPs; pp++)
    {
        std::cout<<"pp = "<<pp<<std::endl;
        for(std::size_t ch = 0; ch < fNChannels; ch++)
        {

            std::cout<<"ch = "<<ch<<std::endl;
            // Allocate type_101 record
            fGeneratedCorel->index[count].t101 = (struct type_101*) calloc(1, sizeof(struct type_101));
            struct type_101* t101 = fGeneratedCorel->index[count].t101;
            clear_101(t101);

            // Set index information
            t101->index = count;
            t101->primary = count;
            t101->nblocks = 1;

            //grab the ref/rem channel ids
            tokens.clear();
            std::string ch_id;
            bool chan_id_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "chan_id", ch_id);
            std::cout<<"ch_id = "<<ch_id<<std::endl;

            tokenizer.SetString(&ch_id);
            tokenizer.GetTokens(&tokens);
            if(tokens.size() != 2){std::cout<<"ERROR"<<std::endl; break;}

            std::string ref_chan_id = tokens[0];
            std::string rem_chan_id = tokens[1];

            //TODO FIXME!!!! pol labels are wrong
            //could replace pol-labels here 

            // Set channel IDs from extracted channel info
            setstr(ref_chan_id, t101->ref_chan_id, 8);
            setstr(rem_chan_id, t101->rem_chan_id, 8);

            // Set correlator board and slot (default values)
            t101->corr_board = 0;
            t101->corr_slot = 0;
            t101->ref_chan = ch;
            t101->rem_chan = ch;

            // Initialize post_mortem flags
            t101->post_mortem = 0;

            // Allocate space for APs in this index
            fGeneratedCorel->index[count].ap_space = fNAPs;
            fGeneratedCorel->index[count].t120 = (struct type_120**) calloc(fNAPs, sizeof(struct type_120*) );

            msg_debug("mk4interface_reversed", "Generated type_101 record " << count
                      << " for channels " << ref_chan_id << ":" << rem_chan_id << eom);

            count++;
        }
    }
}

void MHO_MK4CorelInterfaceReversed::GenerateType120Records()
{
    if(!fGeneratedCorel || !fGeneratedCorel->index) return;

    if(fVisibilityData && fWeightData)
    {
        for(std::size_t pol_idx = 0; pol_idx < fNPPs; pol_idx++)
        {
            for(std::size_t ch_idx = 0; ch_idx < fNChannels; ch_idx++)
            {
                for(std::size_t ap = 0; ap < fNAPs; ap++)
                {
                    // Allocate type_120 record with variable size for spectral data
                    size_t spec_size = fNSpectral * sizeof(struct spectral);
                    size_t total_size = sizeof(struct type_120) - sizeof(union lag_data) + spec_size;
                    struct type_120* t120 = (struct type_120*) calloc(1, total_size);
                    clear_120(t120);
                    
                    // Calculate the correct index in the mk4_corel structure
                    std::size_t record_idx = pol_idx * fNChannels + ch_idx;
                    fGeneratedCorel->index[record_idx].t120[ap] = t120;

                    // Set type to SPECTRAL
                    t120->type = SPECTRAL;
                    t120->nlags = fNSpectral;

                    // Set baseline (from type_100)
                    if(fGeneratedCorel->t100)
                    {
                        memcpy(t120->baseline, fGeneratedCorel->t100->baseline, 2);
                    }

                    // Set root code
                    std::string root_code = "";
                    if(fVisibilityData->HasKey("root_code"))
                    {
                        fVisibilityData->Retrieve("root_code", root_code);
                    }
                    setstr(root_code, t120->rootcode, 6);

                    // Set index and AP
                    t120->index = record_idx;
                    t120->ap = ap;

                    // Initialize status and delay parameters
                    t120->status = 0;
                    t120->fr_delay = 0;
                    t120->delay_rate = 0;

                    // Fill spectral data and weight from  data
                    for(std::size_t sp = 0; sp < fNSpectral; sp++)
                    {
                        std::complex<double> vis = fVisibilityData->at(pol_idx, ch_idx, ap, sp);
                        t120->ld.spec[sp].re = vis.real();
                        t120->ld.spec[sp].im = vis.imag();
                    }
                    
                    // Set weight (use first spectral point weight)
                    t120->fw.weight = fWeightData->at(pol_idx, ch_idx, ap, 0);
                }
            }
        }
    }

    msg_debug("mk4interface_reversed", "Generated " << (fNChannels * fNAPs) 
              << " type_120 records" << eom);
}


int MHO_MK4CorelInterfaceReversed::WriteCorelFile()
{
    if(!fGeneratedCorel || fOutputFile.empty())
    {
        msg_error("mk4interface_reversed", "No corel structure generated or output file not specified" << eom);
        return -1;
    }

    int retval = write_mk4corel(fGeneratedCorel, const_cast<char*>(fOutputFile.c_str()));
    if(retval != 0)
    {
        msg_error("mk4interface_reversed", "Failed to write corel file: " << fOutputFile 
                  << ", error code: " << retval << eom);
    }
    else
    {
        msg_debug("mk4interface_reversed", "Successfully wrote corel file: " << fOutputFile << eom);
    }

    return retval;
}

void MHO_MK4CorelInterfaceReversed::setstr(const std::string& str, char* char_array, std::size_t max_size)
{
    std::size_t copy_size = std::min(str.length(), max_size);
    std::memset(char_array, 0, max_size);
    std::memcpy(char_array, str.c_str(), copy_size);
}

// struct date MHO_MK4CorelInterfaceReversed::ConvertDateString(const std::string& date_str)
// {
//     struct date mk4_date;
//     std::memset(&mk4_date, 0, sizeof(struct date));
// 
//     //TODO FIXME!
// 
//     return mk4_date;
// }


} // namespace hops