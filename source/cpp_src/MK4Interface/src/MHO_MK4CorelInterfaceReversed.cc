#include "MHO_MK4CorelInterfaceReversed.hh"
#include "MHO_LegacyDateConverter.hh"
#include "MHO_DirectoryInterface.hh"

#include <cstdlib>
#include <cstring>
#include <sstream>

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


void FillDate(struct date* destination, struct legacy_hops_date& a_date)
{
    destination->year = a_date.year;
    destination->day = a_date.day;
    destination->hour = a_date.hour;
    destination->minute = a_date.minute;
    destination->second = a_date.second;
}

MHO_MK4CorelInterfaceReversed::MHO_MK4CorelInterfaceReversed():
    fVisibilityData(nullptr), 
    fWeightData(nullptr), 
    fGeneratedCorel(nullptr),
    fOutputDir(""),
    fOutputFile(""),
    fRootFilename(""),
    fRootFileBasename("")
{
    fNPPs = 0;
    fNAPs = 0; 
    fNChannels = 0;
    fNSpectral = 0;
}

MHO_MK4CorelInterfaceReversed::~MHO_MK4CorelInterfaceReversed()
{
    FreeAllocated();
    if(fGeneratedCorel)
    {
        clear_mk4corel(fGeneratedCorel);
        free(fGeneratedCorel);
        fGeneratedCorel = nullptr;
    }
}

void
MHO_MK4CorelInterfaceReversed::FreeAllocated()
{
    for(std::size_t i=0; i<fAllocated.size(); i++)
    {
        free( fAllocated[i] );
        fAllocated[i] = nullptr;
    }
    fAllocated.clear();
};


void MHO_MK4CorelInterfaceReversed::SetRootFileName(std::string root_filename)
{
    fRootFilename = root_filename;
    fRootFileBasename = MHO_DirectoryInterface::GetBasename(fRootFilename);
};


void 
MHO_MK4CorelInterfaceReversed::SetOutputDirectory(const std::string& output_dir)
{
    fOutputDir = MHO_DirectoryInterface::GetDirectoryFullPath(output_dir); 
}


void
MHO_MK4CorelInterfaceReversed::GenerateCorelStructure()
{
    if(fVisibilityData && fWeightData)
    {
        fNPPs = fVisibilityData->GetDimension(POLPROD_AXIS);
        fNAPs = fVisibilityData->GetDimension(TIME_AXIS);
        fNChannels = fVisibilityData->GetDimension(CHANNEL_AXIS);
        fNSpectral = fVisibilityData->GetDimension(FREQ_AXIS);

        msg_debug("mk4interface", "visibility data dimensions are: " << fNPPs << " pol products, " 
                  << fNAPs << " APs, " << fNChannels << " channels, " << fNSpectral << " spectral points per channel" << eom);

        if(fNChannels > MAXFREQ)
        {
            msg_warn("mk4interface", "the visibility data has: "<< fNChannels<<
                " channels, but the mark4 dfio library does not support more than " << MAXFREQ << 
                " channels." << eom);
        }

        // Extract metadata from container tags, so we can create the output file name
        std::string baseline_short = "";
        std::string root_code = "";
        if(fVisibilityData->HasKey("baseline_shortname"))
        {
            fVisibilityData->Retrieve("baseline_shortname", baseline_short);
        }
        if(fVisibilityData->HasKey("root_code"))
        {
            fVisibilityData->Retrieve("root_code", root_code);
        }
        if( !(baseline_short.empty() || root_code.empty() ) )
        {
            fOutputFile = baseline_short + ".." + root_code;
        }
        else 
        {
            msg_error("mk4interface", "failed to construct output file name, baseline_shortname and root_code tags missing from visibility data" << eom);
            return;
        }

    }
    else
    {
        msg_error("mk4interface", "no visibility or weight data provided" << eom);
        return;
    }

    //create the mk4 data structures
    InitializeCorelStructure();
    GenerateType000();
    GenerateType100();
    GenerateType101Records();
    GenerateType120Records();

}

void MHO_MK4CorelInterfaceReversed::InitializeCorelStructure()
{
    if(fGeneratedCorel)
    {
        clear_mk4corel(fGeneratedCorel);
        free(fGeneratedCorel);
        fGeneratedCorel = nullptr;
    }

    fGeneratedCorel = (struct mk4_corel*) calloc(1, sizeof(struct mk4_corel));
    clear_mk4corel(fGeneratedCorel);
    
    //allocate space for index records (free'd in clear_mk4corel)
    fGeneratedCorel->index_space = fNChannels*fNPPs + 1;
    msg_debug("mk4interface", "the mk4_corel index space size is: "<< fGeneratedCorel->index_space << eom);
    fGeneratedCorel->index = (struct index_tag*) calloc(fGeneratedCorel->index_space , sizeof(struct index_tag));
}


void MHO_MK4CorelInterfaceReversed::GenerateType000()
{
    if(!fGeneratedCorel) return;
    fGeneratedCorel->id = (struct type_000*) calloc(1, sizeof(struct type_000));
    struct type_000* id = fGeneratedCorel->id;
    fAllocated.push_back( reinterpret_cast<void*>( fGeneratedCorel->id ) );

    //type_000 expects the file name in the format:
    // '/exp_number/scan/filename'
    std::string output_name = ConstructType000FileName();
    msg_debug("mk4interface", "output file name for type_000 is: " << output_name << eom);

    int retval = init_000(id, const_cast<char*>(output_name.c_str()) );
    if(retval != 0)
    {
        msg_error("mk4interface", "init_000 returned non-zero value: "<< retval << eom);
    }
}


void MHO_MK4CorelInterfaceReversed::GenerateType100()
{
    if(!fGeneratedCorel) return;

    fGeneratedCorel->t100 = (struct type_100*) calloc(1, sizeof(struct type_100));
    struct type_100* t100 = fGeneratedCorel->t100;
    clear_100(t100);
    fAllocated.push_back( reinterpret_cast<void*>(t100) );

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
    setstr(fRootFileBasename, t100->rootname, 34);

    // Set correlation quality code (default to empty: "  ")
    setstr("  ", t100->qcode, 2);

    // Set processing percentage (default to 0%)
    t100->pct_done = 0.0;

    // Convert date strings to mk4 date structures
    if(!start_time.empty())
    {
        legacy_hops_date tmp = MHO_LegacyDateConverter::ConvertFromVexFormat(start_time);
        FillDate( &(t100->start), tmp);
    }
    if(!start_time.empty())
    {
        //TODO FIXME --- calculate the correct stop time
        legacy_hops_date tmp = MHO_LegacyDateConverter::ConvertFromVexFormat(start_time);
        FillDate( &(t100->stop), tmp);
    }
    if(!corr_date.empty())
    {
        legacy_hops_date tmp = MHO_LegacyDateConverter::ConvertFromVexFormat(corr_date);
        FillDate( &(t100->procdate), tmp);
    }

    // Set data record counts
    t100->ndrec = fNPPs * fNChannels * fNAPs;  // Total number of type_120 records
    t100->nindex = fNChannels;         // Number of type_101 records
    t100->nlags = fNSpectral;          // Number of spectral points
    t100->nblocks = 1;                 // Assuming single block per index

    msg_debug("mk4interface", "Generated type_100 record for baseline " << baseline_short << eom);
} 

void MHO_MK4CorelInterfaceReversed::GenerateType101Records()
{
    if(!fGeneratedCorel || !fGeneratedCorel->index) return;
    auto pp_ax = &(std::get<POLPROD_AXIS>(*fVisibilityData));

    int count = 0;
    for(std::size_t pp = 0; pp < fNPPs; pp++)
    {
        std::string pprod = pp_ax->at(pp);
        char ref_pol;
        char rem_pol; 
        if(pprod.size() == 2)
        {
            ref_pol = pprod[0];
            rem_pol = pprod[1];
            //Allocate unused space in the 0-th index -- TERRIBLE HACK
            //We cannot use the data in the fGeneratedCorel->index[0] location. This is because the type_120.index value
            //which is used to refer to the related type_101 record (channel record), also does double-duty as a condition by
            //which to "flag" the presence of the visibility data (see apply_filter.c line 102).
            //The related filtering appears to have been done under the idiotic assumption that if type_120.index == 0,
            //then there is no data there and it should be discarded 
            //so to deal with this, we create an dummy (extra) copy of the very first element of the array at index 0 here
            //and increment the count count:
            if(count == 0){ CreateType101Record(0, 0, ref_pol, rem_pol); count++;}
            for(std::size_t ch = 0; ch < fNChannels; ch++)
            {
                CreateType101Record(count, ch, ref_pol, rem_pol);
                count++;
            }
        }
        else 
        {
            msg_error("mk4interface", "mis-labeled pol-product is not a 2-character code: "<< pprod << eom);
        }
    }
}


void 
MHO_MK4CorelInterfaceReversed::CreateType101Record(int count, std::size_t ch, char ref_pol, char rem_pol)
{
    auto chan_ax = &(std::get<CHANNEL_AXIS>(*fVisibilityData));

    // Allocate type_101 record
    fGeneratedCorel->index[count].t101 = (struct type_101*) calloc(1, sizeof(struct type_101));
    struct type_101* t101 = fGeneratedCorel->index[count].t101;
    clear_101(t101);
    fAllocated.push_back( reinterpret_cast<void*>(t101) );

    // Set index information
    t101->index = count;
    t101->primary = 0;
    t101->nblocks = 1;

    //construct the reference/remote station channel names
    std::string fgroup;
    std::string sideband;
    bool fgroup_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "frequency_band", fgroup);
    bool sb_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", sideband);
    std::string ref_chan_id = ConstructMK4ChannelID(fgroup, ch, sideband, ref_pol);
    std::string rem_chan_id = ConstructMK4ChannelID(fgroup, ch, sideband, rem_pol);

    // Set channel IDs from extracted channel info
    setstr(ref_chan_id, t101->ref_chan_id, 8);
    setstr(rem_chan_id, t101->rem_chan_id, 8);

    // Set correlator board and slot (default values)
    t101->corr_board = 0;
    t101->corr_slot = 0;
    //logically we should set these to 'ch', but difx2mark4 just sets them to zero
    t101->ref_chan = 0;
    t101->rem_chan = 0;
    //not used
    t101->post_mortem = 0;

    // Allocate space for APs in this index (these are deleted in clear_mk4corel)
    fGeneratedCorel->index[count].ap_space = fNAPs;
    fGeneratedCorel->index[count].t120 = (struct type_120**) calloc(fNAPs, sizeof(struct type_120*) );

    msg_debug("mk4interface", "Generated type_101 record " << count
              << " for channels " << ref_chan_id << ":" << rem_chan_id << eom);
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
                //index into record, note the offset of '1' to account for the dummy entry at index 0
                std::size_t record_idx = pol_idx * fNChannels + ch_idx + 1; 
                for(std::size_t ap = 0; ap < fNAPs; ap++)
                {
                    // Allocate type_120 record with variable size for spectral data
                    size_t spec_size = fNSpectral * sizeof(struct spectral);
                    size_t total_size = sizeof(struct type_120) - sizeof(union lag_data) + spec_size;
                    struct type_120* t120 = (struct type_120*) calloc(1, total_size);
                    clear_120(t120);
                    fAllocated.push_back( reinterpret_cast<void*>(t120) );
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
                    
                    // Set weight
                    t120->fw.weight = fWeightData->at(pol_idx, ch_idx, ap, 0);
                }
            }
        }
    }
}


int MHO_MK4CorelInterfaceReversed::WriteCorelFile()
{
    if(!fGeneratedCorel || fOutputFile.empty())
    {
        msg_error("mk4interface", "no corel structure generated or output file not specified" << eom);
        return -1;
    }
    
    std::string outfile = fOutputDir + "/" + fOutputFile;

    int retval = write_mk4corel(fGeneratedCorel, const_cast<char*>(outfile.c_str()));
    if(retval <= 0)
    {
        msg_error("mk4interface", "failed to write corel file: " << outfile 
                  << ", error code: " << retval << eom);
    }
    else
    {
        msg_debug("mk4interface", "successfully wrote: "<<retval<<" bytes, to corel file: " << fOutputFile << eom);
    }

    return retval;
}

void MHO_MK4CorelInterfaceReversed::setstr(const std::string& str, char* char_array, std::size_t max_size)
{
    std::size_t copy_size = std::min(str.length(), max_size);
    std::memset(char_array, 0, max_size);
    std::memcpy(char_array, str.c_str(), copy_size);
}


std::string MHO_MK4CorelInterfaceReversed::ConstructMK4ChannelID(std::string fgroup, int index, std::string sideband, char pol)
{
    std::stringstream ss;
    ss << fgroup;
    if(index < 10)
    {
        ss << "0";
    } //pad with leading zero if less than 10
    ss << index;
    ss << sideband;
    ss << pol;
    return ss.str();
}


std::string MHO_MK4CorelInterfaceReversed::ConstructType000FileName()
{
    //type_000 expects the file name in the format:
    // '/exp_number/scan_name/filename'

    std::vector< std::string > tokens;
    MHO_Tokenizer tokenizer;
    tokenizer.SetDelimiter("/");
    tokenizer.SetString(&fOutputDir);
    tokenizer.GetTokens(&tokens);

    std::string scan_name;
    std::string exp_num;
    std::size_t count = 0;

    //starting from the end of the tokens, we expect to  collect the scan_name, then experiement number 
    for(auto it = tokens.rbegin(); it != tokens.rend(); it++)
    {
        if(count == 0){scan_name = *it;}
        if(count == 1){exp_num = *it;}
        count++;
        if(count > 1){break;}
    }

    //warn the user if exp_num is not actually a 4-digit number
    if(exp_num.size() != 4)
    {
        msg_warn("mk4interface", "the experiment directory is: "<< exp_num << 
            ", this is not a mark4 style 4-digit code, and may break downstream processing." << eom);
    }

    std::string output_name = "/" + exp_num + "/" + scan_name + "/" + fOutputFile;
    return output_name;
}


} // namespace hops