#include "MHO_MK4StationInterfaceReversed.hh"
#include "MHO_LegacyDateConverter.hh"
#include "MHO_DirectoryInterface.hh"

#include "MHO_MathUtilities.hh"


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

#define MHZ_TO_HZ 1000000.0

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
}

MHO_MK4StationInterfaceReversed::~MHO_MK4StationInterfaceReversed()
{
    FreeAllocated();
    if(fGeneratedStation)
    {
        clear_mk4sdata(fGeneratedStation);
        free(fGeneratedStation);
        fGeneratedStation = nullptr;
    }
}


void
MHO_MK4StationInterfaceReversed::FreeAllocated()
{
    for(std::size_t i=0; i<fAllocated.size(); i++)
    {
        free( fAllocated[i] );
        fAllocated[i] = nullptr;
    }
    fAllocated.clear();
};

void 
MHO_MK4StationInterfaceReversed::SetOutputDirectory(const std::string& output_dir)
{
    fOutputDir = MHO_DirectoryInterface::GetDirectoryFullPath(output_dir); 
}

void
MHO_MK4StationInterfaceReversed::GenerateStationStructure()
{
    if(!fStationCoordData)
    {
        msg_error("mk4interface", "Station coordinate data not provided" << eom);
        return;
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
        return;
    }



    InitializeStationStructure();
    GenerateType000();
    GenerateType300();
    GenerateType301Records();
    //do not bother generating type_302 records...they are entirely unused in fourfit
    GenerateType303Records();

    if(fPCalData != nullptr)
    {
        // extract PCal dimensions first if provided
        fNPols = fPCalData->GetDimension(MTPCAL_POL_AXIS);
        fNAPs = fPCalData->GetDimension(MTPCAL_TIME_AXIS);
        fNTones = fPCalData->GetDimension(MTPCAL_FREQ_AXIS);
        
        msg_debug("mk4interface", "PCal dimensions: " << fNPols << " polarizations, " 
                  << fNAPs << " APs, " << fNTones << " tones" << eom);

        ExtractPCalChannelInfo(); //need the station channel setup in order to contruct type_309s
        GenerateType309Records();
    }
    else 
    {
        std::cout<<"NO PCAL DATA"<<std::endl;
    }

}

void MHO_MK4StationInterfaceReversed::InitializeStationStructure()
{
    if(fGeneratedStation)
    {
        clear_mk4sdata(fGeneratedStation);
        free(fGeneratedStation);
        fGeneratedStation = nullptr;
    }

    fGeneratedStation = (struct mk4_sdata*) calloc(1, sizeof(struct mk4_sdata));
    clear_mk4sdata(fGeneratedStation);

    // Set PCal record count
    if(fPCalData && fNAPs > 0)
    {
        fGeneratedStation->n309 = fNAPs;
        // Note: t309 is a fixed-size array in mk4_sdata, no need to allocate
    }
}

void MHO_MK4StationInterfaceReversed::GenerateType000()
{
    if(!fGeneratedStation) return;
    fGeneratedStation->id = (struct type_000*) calloc(1, sizeof(struct type_000));
    struct type_000* id = fGeneratedStation->id;
    fAllocated.push_back( reinterpret_cast<void*>(id) );

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
    fAllocated.push_back( reinterpret_cast<void*>(t300) );

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
    //keep a local variable with this info around
    fStationCode = station_code;

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

    //we do not loop over every channel (index of fGeneratedStation->model[] array)
    //because the fourfit c-code only uses the information in the very first element
    //the information in the rest of type_301 and type_303s (as populated by difx2mark4) is entirely redudant for every channel

    std::string chan_id = "dummy";
    std::size_t ch = 0;
    for(std::size_t sp = 0; sp < fNIntervals; sp++)
    {
        // Allocate type_301 record
        fGeneratedStation->model[ch].t301[sp] = (struct type_301*)calloc(1, sizeof(struct type_301));
        struct type_301* t301 = fGeneratedStation->model[ch].t301[sp];
        clear_301(t301);
        fAllocated.push_back( reinterpret_cast<void*>(t301) );

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

    msg_debug("mk4interface", "Generated " << fNIntervals << " type_301 records" << eom);
}

void MHO_MK4StationInterfaceReversed::GenerateType303Records()
{
    if(!fGeneratedStation || !fStationCoordData) return;


    //we do not loop over every channel (index of fGeneratedStation->model[] array)
    //because the fourfit c-code only uses the information in the very first element
    std::size_t ch = 0;
    std::string chan_id = "dummy";
    for(std::size_t sp = 0; sp < fNIntervals; sp++)
    {
        //allocate type_303 record
        fGeneratedStation->model[ch].t303[sp] = (struct type_303*)calloc(1, sizeof(struct type_303));
        struct type_303* t303 = fGeneratedStation->model[ch].t303[sp];
        clear_303(t303);
        fAllocated.push_back( reinterpret_cast<void*>(t303) );

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

    msg_debug("mk4interface", "Generated " << fNIntervals << " type_303 records" << eom);
}




void MHO_MK4StationInterfaceReversed::GenerateType309Records()
{
    if(!fGeneratedStation || !fPCalData) return;

    double default_acc_period = 1.0;  // Default 1 second accumulation period
    double default_sample_period = 1.0/32e6;

    // Get accumulation period from time axis if available -- assume same AP across entire array
    auto& time_axis = std::get<MTPCAL_TIME_AXIS>(*fPCalData);
    auto& freq_axis = std::get<MTPCAL_FREQ_AXIS>(*fPCalData);

    if(time_axis.GetSize() >= 1)
    {
        default_acc_period = time_axis.at(1) - time_axis.at(0);
    }

    //figure out the pcal start time info -- this information may not exist 
    //if the hops file was generated originally from mark4-type data
    //in that case we will fall back to using the 'model_start' time in the type_300
    std::string start_time = "";
    double start_time_mjd = 0;
    if(fPCalData->HasKey("start"))
    {
        fPCalData->Retrieve("start", start_time);
    }
    if(fPCalData->HasKey("start_time_mjd"))
    {
        fPCalData->Retrieve("start_time_mjd", start_time_mjd);
    }

    std::size_t count309items = 0;
    for(std::size_t ap = 0; ap < fNAPs; ap++)
    {
        // Allocate type_309 record
        struct type_309* t309 = (struct type_309*)calloc(1, sizeof(struct type_309));
        fGeneratedStation->t309[ap] = t309;
        clear_309(t309);
        fAllocated.push_back( reinterpret_cast<void*>(t309) );
        double ap_start = time_axis.at(ap); //grap the ap start time
        t309->rot = ComputeType309Rot(ap_start, start_time, start_time_mjd); //t309 proxy time parameter
        count309items++;

        // Set SU number and accumulation parameters
        t309->su = 0;  // Default SU
        t309->ntones = std::min((int)fNTones, T309_MAX_PHASOR);
        t309->acc_period = default_acc_period;

        // initialize all tone phasor space
        for(int ch = 0; ch < T309_MAX_CHAN; ch++)
        {
            std::memset(t309->chan[ch].chan_name, 0, 8);
            t309->chan[ch].freq = 0.0;
            for(int tone = 0; tone < T309_MAX_PHASOR; tone++)
            {
                t309->chan[ch].acc[tone][0] = 0;
                t309->chan[ch].acc[tone][1] = 0;
            }
        }

        // Fill channel data from PCal container
        for(std::size_t ch=0; ch < fPCalChannelList.size(); ch++)
        {
            auto ch_info = fPCalChannelList[ch];
            int ch_idx = ch;
            if(ch_idx >= T309_MAX_CHAN){ break;}

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
                //need to know the offset into the accumulation array for this channel/tone-set, assume USB
                int acc_idx = ch_info.accumulator_start_index + tone;
                //tone order for LSB channels is backwards....but possibly also for USB channels that were created from LSB channels (zoom-bands)?
                //how can we detect the zoom-bands issue?
                if(ch_info.net_sideband == "L")
                {
                    acc_idx = (ch_info.accumulator_start_index + ch_info.ntones - 1) - tone;
                }
                
                std::complex<double> phasor = fPCalData->at(pol_idx, ap, tone_idx);
                double tone_freq = freq_axis.at(tone_idx);

                //LSB tone's are flipped and conjugated (we ignore 2012 sign flip)
                if(ch_info.net_sideband == "L")
                {
                    phasor = -1.0 * std::conj(phasor);
                }

                //calculate offset from the channel edge
                double ch_freq = ch_info.sky_freq;
                double freq_delta = tone_freq - ch_freq;  //TODO FIXME LSB & USB
                freq_delta *= MHZ_TO_HZ; //stored as Hz 

                //set this freq offset value in the type_309 array
                //the tone frequency offsets are stored in chan[ch].freq)
                //this is because the tone accumulator index and channel index are 
                //inexplicably mixed up for no particular reason
                t309->chan[acc_idx].freq = freq_delta;
                
                uint32_t real_count, imag_count;
                ConvertPhasorToCounts(phasor, t309->acc_period, ch_info.sample_period, 
                                      real_count, imag_count);

                std::cout<<"adding phasor = "<<real_count<<", "<<imag_count<<std::endl;
                t309->chan[ch_idx].acc[acc_idx][0] = real_count;
                t309->chan[ch_idx].acc[acc_idx][1] = imag_count;
            }
        }
    }

    msg_debug("mk4interface", "Generated " << count309items << " type_309 records" << eom);
}



void MHO_MK4StationInterfaceReversed::ExtractPCalChannelInfo()
{
    //this method is used is we are converting data back to mark4 format, that originally came from mark4 
    //in that case, the station channel set-up is already attached to the pcal data object
    if(!fPCalData) return;
    fPCalChannelList.clear();

    // Extract channel information from frequency axis interval labels
    auto& freq_axis = std::get<MTPCAL_FREQ_AXIS>(*fPCalData);
    
    // Look for channel labels across all polarizations
    for(std::size_t pol_idx = 0; pol_idx < fNPols; pol_idx++)
    {
        std::string pol = std::get<MTPCAL_POL_AXIS>(*fPCalData).at(pol_idx);
        std::string name_key = "channel_mk4id_" + pol;
        
        // Get all interval labels that contain this polarization's channel info
        auto matching_labels = freq_axis.GetMatchingIntervalLabels(name_key);

        std::cout<<"labels matching: "<<name_key<<std::endl;
        
        for(const auto& label : matching_labels)
        {
            //std::cout<<"dump: "<<label.dump(2)<<std::endl;
            std::string index_key = "channel_index";
            
            if(label.contains(name_key) )
            {
                PCalChannelInfo ch_info;
                ch_info.channel_name = label[name_key].get<std::string>();
                ch_info.net_sideband = label["net_sideband"].get<std::string>();
                ch_info.sky_freq = label["sky_freq"].get<double>();
                ch_info.bandwidth = label["bandwidth"].get<double>();
                ch_info.accumulator_start_index = label["accumulator_start_index"].get<int>();

                ch_info.polarization = pol;
                //ch_info.channel_index = label[index_key].get<int>();
                
                // Calculate number of tones in this channel from interval bounds
                int lower = label["lower_index"].get<int>();
                int upper = label["upper_index"].get<int>();
                ch_info.ntones = upper - lower;
                ch_info.tone_start = lower;
                
                // Default sample period (could be extracted from bandwidth if available)
                ch_info.sample_period = label["sample_period"].get<double>();
                
                fPCalChannelList.push_back(ch_info);
                
                msg_debug("mk4interface", "Extracted PCal channel: " << ch_info.channel_name 
                          << " pol=" << ch_info.polarization
                          << " tones=" << ch_info.ntones << eom);
            }
        }
    }

    //pcal data did not have any channel info attached (so extract it from the VEX file)
    if( fPCalChannelList.size() == 0)
    {
        msg_debug("mk4interface", "there is no station channel data attached to the pcal data object, falling back to vex info" << eom );
        ExtractPCalChannelInfoFromVex();
    }

    std::cout<<"channel info size ="<<fPCalChannelList.size()<<std::endl;
}

std::string MHO_MK4StationInterfaceReversed::GetStationMode()
{
    std::string mode = "";
    for(auto it = fVexData["$SCHED"].begin(); it != fVexData["$SCHED"].end(); ++it)
    {
        mode = (*it)["mode"].get<std::string>();
        for(auto it = fVexData["$MODE"][mode]["$FREQ"].begin(); it != fVexData["$MODE"][mode]["$FREQ"].end(); it++)
        {
            for( auto qit = (*it)["qualifiers"].begin(); qit != (*it)["qualifiers"].end(); qit++)
            {
                std::string station_id = (*qit).get<std::string>();
                if(station_id == fStationCode) 
                {
                    //this mode's freq config matches this station
                    mode = (*it)["keyword"].get<std::string>();
                    return mode;
                }
            }
        }
    }
    mode == "";
    return mode;
}


void MHO_MK4StationInterfaceReversed::ExtractPCalChannelInfoFromVex()
{
    //this method is used if we are trying to do a conversion directly from DiFX 
    //this is because the pcal-data object will not already have station channel 
    //meta-data attached if we are coming directly from DiFX

    // std::cout<< fVexData["$FREQ"].dump(2) << std::endl;
    // std::cout<< fVexData["$SCHED"].dump(2) << std::endl;

    std::string mode = GetStationMode();
    std::cout<<"MODE = "<<mode<<std::endl;
    if(mode == ""){msg_error("mk4interface", "could not located mode information in vex file $SCHED block"<<eom);}

    // SCHED -> [0] -> mode
    // 
    // MODE[mode] -> FREQ [?]  where station code (2-char) is present
    // 
    // SITE[?] -> site_ID (2-char code)
    // 
    // FREQ[ mode ] where mode matches the station

    double pcal_spacing = 5.0;// TODO FIXME
    auto tone_freq_ax = &(std::get< MTPCAL_FREQ_AXIS >(*fPCalData));
    std::vector< double > tone_freq_offsets;

    if( fVexData["$FREQ"].contains(mode) )
    {
        //loop over all channels and collect the information
        for(auto it = fVexData["$FREQ"][mode]["chan_def"].begin(); it != fVexData["$FREQ"][mode]["chan_def"].end(); ++it)
        {
            //std::cout<< (*it).dump(2) << std::endl;

            double factor = 1.0;
            PCalChannelInfo ch_info;
            ch_info.channel_name = (*it)["channel_name"].get<std::string>();
            ch_info.net_sideband = (*it)["net_sideband"].get<std::string>();

            ch_info.sky_freq = (*it)["sky_frequency"]["value"].get<double>();
            std::string fr_units = (*it)["sky_frequency"]["units"].get<std::string>();
            factor = FactorConvertToMHz(fr_units);
            ch_info.sky_freq *= factor; //convert to MHz

            ch_info.bandwidth = (*it)["bandwidth"]["value"].get<double>();
            std::string bw_units = (*it)["bandwidth"]["units"].get<std::string>();
            factor = FactorConvertToMHz(bw_units);
            ch_info.bandwidth *= factor; //convert to MHz

            //figure out the upper/lower frequency limits for this channel
            double lower_freq, upper_freq;
            MHO_MathUtilities::DetermineChannelFrequencyLimits(ch_info.sky_freq, ch_info.bandwidth, 
                                                               ch_info.net_sideband, lower_freq, upper_freq);
            std::cout<<"low freq, upper freq = "<<lower_freq<<", "<<upper_freq<<std::endl;

            std::size_t start_idx, ntones;
            DetermineChannelToneIndexes(lower_freq, upper_freq, start_idx, ntones);
            // Calculate number of tones in this channel from interval bounds
            ch_info.tone_start = start_idx;
            ch_info.ntones = ntones;
            std::cout<<"start idx, ntones = "<<start_idx<<", "<<ntones<<std::endl;

            //figure out the accumulator index (need to compute the tone offset frequencies for each tone in this channel 
            for(std::size_t j = 0; j < ntones; j++)
            {
                std::size_t idx = start_idx + j;
                double tone_freq = tone_freq_ax->at(idx);
                //std::cout<<"tone freq = "<<tone_freq<<std::endl;
                //std::cout<<"sky freq = "<< ch_info.sky_freq <<std::endl;
                double freq_delta = tone_freq - ch_info.sky_freq;  //TODO FIXME LSB & USB
                //std::cout<<"freq delta = "<<freq_delta<<std::endl;
                freq_delta *= MHZ_TO_HZ; //offsets are calculated/stored as Hz

                //dumb brute force search 
                double tol = 1e-3;
                bool found = false;
                for(std::size_t i = 0; i < tone_freq_offsets.size(); i++)
                {
                    if( std::fabs(freq_delta - tone_freq_offsets[i]) < tol)
                    {
                        found = true;
                        break;
                    }
                }
                if(!found)
                {
                    //store the index of this offset if we are on the first tone
                    if(j == 0){ch_info.accumulator_start_index = tone_freq_offsets.size();}
                    std::cout<<"adding new freq off = "<<freq_delta<<std::endl;
                    tone_freq_offsets.push_back(freq_delta);
                }
            }

            //set to zero because don't yet know
            ch_info.polarization = "?";
            ch_info.channel_index = 0;


            // Default sample period (could be extracted from bandwidth if available)
            ch_info.sample_period = 1.0/(2.0*ch_info.bandwidth*MHZ_TO_HZ); //assuming bandwidth is in MHz
            
            fPCalChannelList.push_back(ch_info);
            
            msg_debug("mk4interface", "Extracted PCal channel: " << ch_info.channel_name << " acc idx: "<< ch_info.accumulator_start_index
                      << " pol=" << ch_info.polarization << " idx=" << ch_info.channel_index 
                      << " tones=" << ch_info.ntones << eom);
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


double 
MHO_MK4StationInterfaceReversed::ComputeType309Rot(double ap_offset, std::string start_time, double start_time_mjd) //, const std::string start_time_vex, double start_time_mjd)
{
    double magic_number = 3.2e7; //also known as SYSCLK in CorAsc2.c
    double seconds_per_day = 86400.0;

    //figure out the fractional part of the day since the start of this scan
    double ref_day = std::floor(start_time_mjd);
    double frac_day = start_time_mjd - ref_day;
    frac_day += ap_offset/seconds_per_day; //add the time offset into the scan

    //start_time is in vex format, so figure out the integer day-of-year 
    size_t y_pos = start_time.find('y');
    size_t d_pos = start_time.find('d');
    int doy = 0;
    if (y_pos != std::string::npos && d_pos != std::string::npos && d_pos > y_pos && start_time_mjd != 0.) 
    {
        std::string doy_str = start_time.substr(y_pos + 1, d_pos - y_pos - 1);
        doy = std::stoi(doy_str);
    }
    else 
    {
        //fall back to type_300 model_start to figure out the scan start time
        //if we dont have the MJD start or vex time
        return ComputeType309RotFallback(ap_offset);
    }

    //finally, add the integer days since the start of the year (this is 1 indexed)
    double t = doy + frac_day;
    double rot = magic_number * seconds_per_day * (t - 1.0); //subtract off 1 because day-of-year is 1-indexed
    return rot;
}


double 
MHO_MK4StationInterfaceReversed::ComputeType309RotFallback(double ap_offset)
{
    double magic_number = 3.2e7; //also known as SYSCLK in CorAsc2.c
    double seconds_per_day = 86400.0;
    int doy = fGeneratedStation->t300->model_start.day;
    int hour = fGeneratedStation->t300->model_start.hour;
    int minute = fGeneratedStation->t300->model_start.minute;
    float second = fGeneratedStation->t300->model_start.second;

    double frac_day = ( second +  60*( minute + 60*hour) )/seconds_per_day;
    frac_day += ap_offset/seconds_per_day;

    //finally, add the integer days since the start of the year (this is 1 indexed)
    double t = doy + frac_day;
    double rot = magic_number * seconds_per_day * (t - 1.0); //subtract off 1 because day-of-year is 1-indexed
    return rot;
}


double MHO_MK4StationInterfaceReversed::FactorConvertToMHz(std::string units)
{
    //currently only support Ms/sec --> we need to implement units support throughout for proper support
    double factor = 1.0;
    if(units == "MHz"){factor = 1.0;}
    if(units == "GHz"){factor = 1000.0;}
    if(units == "KHz" || units == "kHz"){factor = 1e-3;}
    if(units == "Hz"){factor = 1e-6;}
    return factor;
}

void MHO_MK4StationInterfaceReversed::DetermineChannelToneIndexes(double lower_freq, 
                                                               double upper_freq, 
                                                               std::size_t& start_idx,
                                                               std::size_t& ntones)
{
    auto tone_freq_ax = &(std::get< MTPCAL_FREQ_AXIS >(*fPCalData));

    double start_tone_frequency = 0;
    start_idx = 0;
    ntones = 0;

    for(std::size_t j = 0; j < tone_freq_ax->GetSize(); j++)
    {
        if(lower_freq <= tone_freq_ax->at(j) && tone_freq_ax->at(j) < upper_freq)
        {
            if(ntones == 0)
            {
                start_tone_frequency = tone_freq_ax->at(j);
                start_idx = j;
            }
            ntones++;
        }
    }
}



} // namespace hops