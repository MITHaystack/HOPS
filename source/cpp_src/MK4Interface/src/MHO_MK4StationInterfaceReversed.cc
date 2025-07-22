#include "MHO_MK4StationInterfaceReversed.hh"
#include "MHO_LegacyDateConverter.hh"
#include "MHO_DirectoryInterface.hh"


#include <complex>
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

#define T309_MAX_CHAN 64
#define T309_MAX_PHASOR 64

// Constants from fourfit pcal_interp.c
#define TWO31 2147483648.0
#define TWO32 4294967296.0

#define MK4_MAX_COEFF 6

namespace hops
{

void FillDateStation(struct date* destination, struct legacy_hops_date& a_date)
{
    destination->year = a_date.year;
    destination->day = a_date.day;
    destination->hour = a_date.hour;
    destination->minute = a_date.minute;
    destination->second = a_date.second;
}


MHO_MK4StationInterfaceReversed::MHO_MK4StationInterfaceReversed():
    fStationCoordData(nullptr), 
    fPCalData(nullptr), 
    fGeneratedStation(nullptr),
    fOutputDir(""),
    fOutputFile("")
{
    fNCoord = 0;
    fNIntervals = 0;
    fNCoeffs = 0;
    fNPols = 0;
    fNAPs = 0;
    fNTones = 0;
    fPCalChannelList.clear();
    fPolToChannelMap.clear();
}

MHO_MK4StationInterfaceReversed::~MHO_MK4StationInterfaceReversed()
{
    if(fGeneratedStation)
    {
        clear_mk4sdata(fGeneratedStation);
        free(fGeneratedStation);
    }
}


void 
MHO_MK4StationInterfaceReversed::SetOutputDirectory(const std::string& output_dir)
{
    fOutputDir = MHO_DirectoryInterface::GetDirectoryFullPath(output_dir); 
}


struct mk4_sdata* MHO_MK4StationInterfaceReversed::GenerateStationStructure()
{
    if(!fStationCoordData)
    {
        msg_error("mk4interface", "Station coordinate data not provided" << eom);
        return nullptr;
    }

    // Extract dimensions from station coordinate container
    fNCoord = fStationCoordData->GetDimension(COORD_AXIS);
    fNIntervals = fStationCoordData->GetDimension(INTERVAL_AXIS); 
    fNCoeffs = fStationCoordData->GetDimension(COEFF_AXIS);

    msg_debug("mk4interface", "Station coord dimensions: " << fNCoord << " coordinates, " 
              << fNIntervals << " intervals, " << fNCoeffs << " coefficients" << eom);

    // Extract metadata from container tags, so we can create the output file name
    std::string station_mk4id = "";
    std::string root_code = "";
    if(fStationCoordData->HasKey("station_mk4id"))
    {
        fStationCoordData->Retrieve("station_mk4id", station_mk4id);
    }
    if(fStationCoordData->HasKey("root_code"))
    {
        fStationCoordData->Retrieve("root_code", root_code);
    }
    if( !(station_mk4id.empty() || root_code.empty() ) )
    {
        fOutputFile = station_mk4id + ".." + root_code;
    }
    else 
    {
        msg_error("mk4interface", "failed to construct output file name, station mk4id and root_code tags missing from station coordinate data" << eom);
        return nullptr;
    }

    // Extract PCal dimensions if provided
    if(fPCalData)
    {
        fNPols = fPCalData->GetDimension(MTPCAL_POL_AXIS);
        fNAPs = fPCalData->GetDimension(MTPCAL_TIME_AXIS);
        fNTones = fPCalData->GetDimension(MTPCAL_FREQ_AXIS);
        
        msg_debug("mk4interface", "PCal dimensions: " << fNPols << " polarizations, " 
                  << fNAPs << " APs, " << fNTones << " tones" << eom);

        ExtractPCalChannelInfo();
    }


    // Initialize the station structure
    InitializeStationStructure();
    
    // Generate the various record types
    GenerateType000();
    GenerateType300();
    GenerateType301Records();
    GenerateType303Records();
    
    if(fPCalData)
    {
        GenerateType309Records();
    }

    return fGeneratedStation;
}

void MHO_MK4StationInterfaceReversed::InitializeStationStructure()
{
    if(fGeneratedStation)
    {
        clear_mk4sdata(fGeneratedStation);
        free(fGeneratedStation);
    }

    fGeneratedStation = (struct mk4_sdata*) calloc(1, sizeof(struct mk4_sdata));
    clear_mk4sdata(fGeneratedStation);

    // // Set PCal record count
    // if(fPCalData && fNAPs > 0)
    // {
    //     fGeneratedStation->n309 = fNAPs;
    //     // Note: t309 is a fixed-size array in mk4_sdata, no need to allocate
    // }
}

void MHO_MK4StationInterfaceReversed::GenerateType000()
{
    if(!fGeneratedStation) return;
    fGeneratedStation->id = (struct type_000*) calloc(1, sizeof(struct type_000));
    struct type_000* id = fGeneratedStation->id;

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


void MHO_MK4StationInterfaceReversed::GenerateType300()
{
    if(!fGeneratedStation) return;

    // Allocate type_300 record
    fGeneratedStation->t300 = (struct type_300*) calloc(1, sizeof(struct type_300));
    struct type_300* t300 = fGeneratedStation->t300;
    clear_300(t300);

    // Extract metadata from container tags
    std::string station_name = "";
    std::string station_code = "";
    std::string station_mk4id = "";
    std::string model_start = "";
    double model_interval = 1.0;  // Default 1 second

    if(fStationCoordData->HasKey("station_name"))
    {
        fStationCoordData->Retrieve("station_name", station_name);
    }
    if(fStationCoordData->HasKey("station_code"))
    {
        fStationCoordData->Retrieve("station_code", station_code);
    }
    if(fStationCoordData->HasKey("station_mk4id"))
    {
        fStationCoordData->Retrieve("station_mk4id", station_mk4id);
    }
    if(fStationCoordData->HasKey("model_start"))
    {
        fStationCoordData->Retrieve("model_start", model_start);
    }
    if(fStationCoordData->HasKey("model_interval"))
    {
        fStationCoordData->Retrieve("model_interval", model_interval);
    }

    // Set station information
    setstr(station_name, t300->name, 32);
    setstr(station_code, t300->intl_id, 2);
    if(!station_mk4id.empty())
    {
        t300->id = station_mk4id[0];
    }

    // Set model parameters
    t300->model_interval = model_interval;
    t300->nsplines = fNIntervals;

    // Convert date strings to mk4 date structures
    if(!model_start.empty())
    {
        legacy_hops_date tmp = MHO_LegacyDateConverter::ConvertFromVexFormat(model_start);
        FillDateStation( &(t300->model_start), tmp);
    }

    // Set default SU number
    t300->SU_number = 0;

    msg_debug("mk4interface", "Generated type_300 record for station " << station_name << eom);
}




void MHO_MK4StationInterfaceReversed::GenerateType301Records()
{
    if(!fGeneratedStation || !fStationCoordData) return;

    //FIXME
    std::size_t fNChannels = 32;

    for(std::size_t ch =0; ch < fNChannels; ch++)
    {
        //loop over every channel -- the information in the type_301 and type_303s is entirely redudant 
        //across every channel, only type_302 (phase spline is different, but that is only because it is delay model * channel_freq
        std::string chan_id = "dummy";

        for(std::size_t sp = 0; sp < fNIntervals; sp++)
        {
            // Allocate type_301 record
            fGeneratedStation->model[ch].t301[sp] = (struct type_301*)calloc(1, sizeof(struct type_301));
            struct type_301* t301 = fGeneratedStation->model[ch].t301[sp];
            clear_301(t301);

            // Set interval number
            t301->interval = sp;
            //set the channel id
            setstr(chan_id, t301->chan_id, 32);

            // Extract delay spline coefficients (coordinate index 0 = delay)
            for(std::size_t cf = 0; cf < std::min(fNCoeffs, (std::size_t) MK4_MAX_COEFF); cf++)
            {
                t301->delay_spline[cf] = fStationCoordData->at(0, sp, cf);  // coord=0 is delay
            }

            // Zero remaining coefficients if needed
            for(std::size_t cf = fNCoeffs; cf < MK4_MAX_COEFF; cf++)
            {
                t301->delay_spline[cf] = 0.0;
            }
        }
    }

    msg_debug("mk4interface", "Generated " << fNChannels*fNIntervals << " type_301 records" << eom);
}

void MHO_MK4StationInterfaceReversed::GenerateType303Records()
{
    if(!fGeneratedStation || !fStationCoordData) return;

    
    //FIXME
    std::size_t fNChannels = 32;

    for(std::size_t ch = 0; ch < fNChannels; ch++)
    {
        //loop over every channel -- the information in the type_301 and type_303s is entirely redudant 
        //across every channel, only type_302 (phase spline is different, but that is only because it is delay model * channel_freq
        std::string chan_id = "dummy";

        for(std::size_t sp = 0; sp < fNIntervals; sp++)
        {
            //allocate type_303 record
            fGeneratedStation->model[ch].t303[sp] = (struct type_303*)calloc(1, sizeof(struct type_303));
            struct type_303* t303 = fGeneratedStation->model[ch].t303[sp];
            clear_303(t303);

            // Set interval number
            t303->interval = sp;
            //set the channel id
            setstr(chan_id, t303->chan_id, 32);

            // Extract coordinate spline coefficients
            // Order: delay(0), azimuth(1), elevation(2), parallactic_angle(3), u(4), v(5), w(6)
            for(std::size_t cf = 0; cf < std::min(fNCoeffs, (std::size_t) MK4_MAX_COEFF); cf++)
            {
                if(fNCoord > 1) t303->azimuth[cf] = fStationCoordData->at(1, sp, cf);
                if(fNCoord > 2) t303->elevation[cf] = fStationCoordData->at(2, sp, cf);
                if(fNCoord > 3) t303->parallactic_angle[cf] = fStationCoordData->at(3, sp, cf);
                if(fNCoord > 4) t303->u[cf] = fStationCoordData->at(4, sp, cf);
                if(fNCoord > 5) t303->v[cf] = fStationCoordData->at(5, sp, cf);
                if(fNCoord > 6) t303->w[cf] = fStationCoordData->at(6, sp, cf);
            }

            // Zero remaining coefficients if needed
            for(std::size_t cf = fNCoeffs; cf < MK4_MAX_COEFF; cf++)
            {
                t303->azimuth[cf] = 0.0;
                t303->elevation[cf] = 0.0;
                t303->parallactic_angle[cf] = 0.0;
                t303->u[cf] = 0.0;
                t303->v[cf] = 0.0;
                t303->w[cf] = 0.0;
            }
        }
    }

    msg_debug("mk4interface", "Generated " << fNChannels*fNIntervals << " type_303 records" << eom);
}




void MHO_MK4StationInterfaceReversed::GenerateType309Records()
{
    if(!fGeneratedStation || !fPCalData) return;

    double default_acc_period = 1.0;  // Default 1 second accumulation period
    double default_sample_period = 1e-6;  // Default 1 microsecond sample period

    for(std::size_t ap = 0; ap < fNAPs; ap++)
    {
        // Allocate type_309 record
        struct type_309* t309 = (struct type_309*)calloc(1, sizeof(struct type_309));
        fGeneratedStation->t309[ap] = t309;

        // Set record ID and version  
        setstr("309", t309->record_id, 3);
        setstr("01", t309->version_no, 2);  // Version 1 for 64 channel support

        // Set SU number and accumulation parameters
        t309->su = 0;  // Default SU
        t309->ntones = std::min((int)fNTones, T309_MAX_PHASOR);

        // Get accumulation period from time axis if available
        auto& time_axis = std::get<MTPCAL_TIME_AXIS>(*fPCalData);
        if(ap < time_axis.GetSize())
        {
            if(ap > 0)
            {
                t309->acc_period = time_axis.at(ap) - time_axis.at(ap-1);
            }
            else
            {
                t309->acc_period = default_acc_period;
            }
        }
        else
        {
            t309->acc_period = default_acc_period;
        }

        // Set ROT (default to 0)
        t309->rot = 0.0;

        // Initialize all channels
        for(int ch = 0; ch < T309_MAX_CHAN; ch++)
        {
            // Clear channel name
            std::memset(t309->chan[ch].chan_name, 0, 8);
            t309->chan[ch].freq = 0.0;
            
            // Clear all accumulators
            for(int tone = 0; tone < T309_MAX_PHASOR; tone++)
            {
                t309->chan[ch].acc[tone][0] = 0;
                t309->chan[ch].acc[tone][1] = 0;
            }
        }

        // Fill channel data from PCal container
        for(const auto& ch_info : fPCalChannelList)
        {
            if(ch_info.channel_index >= T309_MAX_CHAN) continue;
            
            int ch_idx = ch_info.channel_index;
            
            // Set channel name
            setstr(ch_info.channel_name, t309->chan[ch_idx].chan_name, 8);
            
            // Find polarization index
            std::size_t pol_idx = 0;
            for(std::size_t p = 0; p < fNPols; p++)
            {
                std::string pol = std::get<MTPCAL_POL_AXIS>(*fPCalData).at(p);
                if(pol == ch_info.polarization)
                {
                    pol_idx = p;
                    break;
                }
            }

            // Fill tone data for this channel
            for(int tone = 0; tone < ch_info.ntones && tone < T309_MAX_PHASOR; tone++)
            {
                int tone_idx = ch_info.tone_start + tone;
                if(tone_idx >= (int)fNTones) break;
                
                std::complex<double> phasor = fPCalData->at(pol_idx, ap, tone_idx);
                
                uint32_t real_count, imag_count;
                ConvertPhasorToCounts(phasor, t309->acc_period, ch_info.sample_period, 
                                      real_count, imag_count);
                
                int acc_idx = ch_info.tone_start + tone;
                t309->chan[ch_idx].acc[acc_idx][0] = real_count;
                t309->chan[ch_idx].acc[acc_idx][1] = imag_count;
            }
        }
    }

    msg_debug("mk4interface", "Generated " << fNAPs << " type_309 records" << eom);
}



void MHO_MK4StationInterfaceReversed::ExtractPCalChannelInfo()
{
    if(!fPCalData) return;

    fPCalChannelList.clear();
    fPolToChannelMap.clear();

    // Extract channel information from frequency axis interval labels
    auto& freq_axis = std::get<MTPCAL_FREQ_AXIS>(*fPCalData);
    
    // Look for channel labels across all polarizations
    for(std::size_t pol_idx = 0; pol_idx < fNPols; pol_idx++)
    {
        std::string pol = std::get<MTPCAL_POL_AXIS>(*fPCalData).at(pol_idx);
        std::string name_key = "channel_mk4id_" + pol;
        
        // Get all interval labels that contain this polarization's channel info
        auto matching_labels = freq_axis.GetMatchingIntervalLabels(name_key);
        
        for(const auto& label : matching_labels)
        {
            std::string index_key = "channel_index";
            
            if(label.contains(name_key) && label.contains(index_key))
            {
                PCalChannelInfo ch_info;
                ch_info.channel_name = label[name_key].get<std::string>();
                ch_info.polarization = pol;
                ch_info.channel_index = label[index_key].get<int>();
                
                // Calculate number of tones in this channel from interval bounds
                int lower = label["lower_index"].get<int>();
                int upper = label["upper_index"].get<int>();
                ch_info.ntones = upper - lower;
                ch_info.tone_start = lower;
                
                // Default sample period (could be extracted from bandwidth if available)
                ch_info.sample_period = 1e-6;  // 1 microsecond default
                
                fPCalChannelList.push_back(ch_info);
                fPolToChannelMap[pol].push_back(fPCalChannelList.size() - 1);
                
                msg_debug("mk4interface", "Extracted PCal channel: " << ch_info.channel_name 
                          << " pol=" << pol << " idx=" << ch_info.channel_index 
                          << " tones=" << ch_info.ntones << eom);
            }
        }
    }
}

int MHO_MK4StationInterfaceReversed::WriteStationFile()
{
    if(!fGeneratedStation || fOutputFile.empty())
    {
        msg_error("mk4interface", "No station structure generated or output file not specified" << eom);
        return -1;
    }

    std::string outfile = fOutputDir + "/" + fOutputFile;

    int retval = write_mk4sdata(fGeneratedStation, const_cast<char*>(outfile.c_str()));
    if(retval < 0)
    {
        msg_error("mk4interface", "Failed to write station file: " << outfile 
                  << ", error code: " << retval << eom);
    }
    else
    {
        msg_debug("mk4interface", "Successfully wrote: "<<retval<<" bytes to station file: " << outfile << eom);
    }

    return retval;
}

void MHO_MK4StationInterfaceReversed::ConvertPhasorToCounts(const std::complex<double>& phasor, 
                                                            double acc_period, double sample_period,
                                                            uint32_t& real_count, uint32_t& imag_count)
{
    // Reverse the computation from ComputePhasor in the original interface
    // Original: pc_real = (u * sample_period) / (-128.0 * acc_period)
    // Reverse: u = pc_real * (-128.0 * acc_period) / sample_period
    
    double pc_real = phasor.real();
    double pc_imag = phasor.imag();
    
    double u = pc_real * (-128.0 * acc_period) / sample_period;
    double v = pc_imag * (-128.0 * acc_period) / sample_period;
    
    // Convert to uint32_t with proper wrapping
    if(u < 0)
    {
        u += TWO32;
    }
    if(v < 0)
    {
        v += TWO32;
    }
    
    real_count = (uint32_t)(u + 0.5);  // Round to nearest integer
    imag_count = (uint32_t)(v + 0.5);
}

void MHO_MK4StationInterfaceReversed::setstr(const std::string& str, char* char_array, std::size_t max_size)
{
    std::size_t copy_size = std::min(str.length(), max_size);
    std::memset(char_array, 0, max_size);
    std::memcpy(char_array, str.c_str(), copy_size);
}


std::string MHO_MK4StationInterfaceReversed::ConstructType000FileName()
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