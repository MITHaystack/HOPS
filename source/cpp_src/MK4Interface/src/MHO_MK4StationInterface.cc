#include "MHO_MK4StationInterface.hh"
#include "MHO_LegacyDateConverter.hh"

#include <vector>
#include <cstdlib>
#include <cstring>
#include <complex>
#include <set>
#include <algorithm>
#include <cctype>

//mk4 IO library
#ifndef HOPS3_USE_CXX
extern "C"
{
#endif
    #include "mk4_records.h"
    #include "mk4_data.h"
    #include "mk4_dfio.h"
    #include "mk4_vex.h"
#ifndef HOPS3_USE_CXX
}
#endif

#define T309_MAX_CHAN 64
#define T309_MAX_PHASOR 64

//taken from fourfit pcal_interp.c
#define TWO31 2147483648.0
#define TWO32 4294967296.0



namespace hops
{


MHO_MK4StationInterface::MHO_MK4StationInterface():
    fHaveStation(false),
    fStation(nullptr)
{
    fStation = (struct mk4_sdata *) calloc ( 1, sizeof(struct mk4_sdata) );
    fNCoeffs = 0;
    fNIntervals = 0;
    fNCoord = 0;
}

MHO_MK4StationInterface::~MHO_MK4StationInterface()
{
    clear_mk4sdata(fStation);
    free(fStation);
}

station_coord_type*
MHO_MK4StationInterface::ExtractStationFile()
{

    ReadStationFile();

    station_coord_type* st_data = nullptr;

    if(fHaveStation)
    {
        //first thing we have to do is figure out the data dimensions
        //the items stored in the mk4sdata objects are mainly:
        //(0) meta day about the spline model (type_300)
        //(1) delay spline polynomial coeff (type_301)
        //(2) parallatic angle spline coeff (type_303)
        //(3) uvw-coords spline coeff (type_303)
        //(4) phase-cal data (type_309) -- no yet supported here

        //we do not export the channel-dependent phase spline data e.g.
        //phase spline polynomial coeff (type_302)
        //as this can be constructed from the channel freq * delay spline

        fNCoord = NCOORD; //delay, az, el, par-angle, u, v, w (no phase spline)
        fNCoeffs = NCOEFF; //hard-coded value in the mk4 type_301, 303, etc. for max spline coeff
        fNIntervals = fStation->t300->nsplines; //the number of spline intervals

        st_data = new station_coord_type();
        st_data->Resize(fNCoord, fNIntervals, fNCoeffs);
        st_data->ZeroArray();

        std::get<COORD_AXIS>(*st_data)[0] = "delay";
        std::get<COORD_AXIS>(*st_data)[1] = "azimuth";
        std::get<COORD_AXIS>(*st_data)[2] = "elevation";
        std::get<COORD_AXIS>(*st_data)[3] = "parallactic_angle";
        std::get<COORD_AXIS>(*st_data)[4] = "u";
        std::get<COORD_AXIS>(*st_data)[5] = "v";
        std::get<COORD_AXIS>(*st_data)[6] = "w";

        //extract some basics from the type_300 
        type_300* t300 = fStation->t300;
        double spline_interval = t300->model_interval;

        //convert the legacy date struct to a cannonical date/time-stamp string
        struct date model_start = t300->model_start;
        legacy_hops_date ldate;
        ldate.year = model_start.year;
        ldate.day = model_start.day;
        ldate.hour = model_start.hour;
        ldate.minute = model_start.minute;
        ldate.second = model_start.second;
        std::string model_start_date = MHO_LegacyDateConverter::ConvertToVexFormat(ldate);

        //retrieve the station name/id
        std::string mk4_id = getstr(&(t300->id), 1);
        std::string station_id = getstr(t300->intl_id, 2);
        std::string station_name = getstr(t300->name, 32);

        //tag the station data structure with all the meta data from the type_300
        st_data->Insert(std::string("name"), std::string("station_data"));
        st_data->Insert(std::string("station_name"), station_name);
        st_data->Insert(std::string("station_mk4id"), mk4_id);
        st_data->Insert(std::string("station_code"), station_id);
        st_data->Insert(std::string("model_start"), model_start_date);
        st_data->Insert(std::string("nsplines"), fStation->t300->nsplines);
        st_data->Insert(std::string("model_interval"), spline_interval);

        //with the exception of the type_302s, the spline data is the same from each channel, so just use ch=0
        std::size_t ch = 0;
        for(std::size_t sp=0; sp<fNIntervals; sp++)
        {
            std::get<INTERVAL_AXIS>(*st_data)[sp] = sp*spline_interval; //seconds since start
            type_301* t301 = fStation->model[ch].t301[sp]; //delay
            type_303* t303 = fStation->model[ch].t303[sp]; //az,el,par,u,v,w
            if( t301 != nullptr && t303 != nullptr)
            {
                if( t301->interval != sp){msg_error("mk4interface", "spline interval mis-match." << eom);};
                if( t303->interval != sp){msg_error("mk4interface", "spline interval mis-match." << eom);};

                for(std::size_t cf=0; cf<fNCoeffs; cf++)
                {
                    std::get<COEFF_AXIS>(*st_data)[cf] = cf; //polynomial power of this term
                    st_data->at(0, sp, cf) = t301->delay_spline[cf];
                    st_data->at(1, sp, cf) = t303->azimuth[cf];
                    st_data->at(2, sp, cf) = t303->elevation[cf];
                    st_data->at(3, sp, cf) = t303->parallactic_angle[cf];
                    st_data->at(4, sp, cf) = t303->u[cf];
                    st_data->at(5, sp, cf) = t303->v[cf];
                    st_data->at(6, sp, cf) = t303->w[cf];
                }
            }
        }

        //now deal with the type_309 pcal data 
        ExtractPCal(fStation->n309, fStation->t309);

    }

    return st_data;

}


void MHO_MK4StationInterface::ReadStationFile()
{
    if(fHaveStation)
    {
        msg_debug("mk4interface", "Clearing a previously exisiting station data struct."<< eom);
        clear_mk4sdata(fStation);
        fStation = nullptr;
        fHaveStation = false;
    }

    //have to copy fStationFile for const_cast, as mk4 lib doesn't respect const
    std::string fname = fStationFile;
    int retval = read_mk4sdata( const_cast<char*>(fname.c_str()), fStation );
    if(retval == 0)
    {
        fHaveStation = true;
        msg_debug("mk4interface", "Successfully read station data file."<< fStationFile << eom);
    }
    else
    {
        fHaveStation = false;
        msg_debug("mk4interface", "Failed to read station data file: "<< fStationFile << ", error value: "<< retval << eom);
    }
}




std::vector< std::string > 
MHO_MK4StationInterface::GetFreqGroups(int n309, type_309** t309)
{
    //assume data is the same for all APs
    //first determine the set of polarizations and freq groups which are present
    int ap = 0;
    std::set< std::string > fgroup_set;
    std::string fgroup, sb, pol;
    int idx;
    for(int ch=0; ch < T309_MAX_CHAN; ch++)
    {
        std::string ch_name = getstr(&(t309[ap]->chan[ch].chan_name[0]), 8);
        if( ExtractChannelInfo(ch_name, fgroup, sb, pol, idx ) ){fgroup_set.insert(fgroup);}
    }
    std::vector< std::string > fgroups;
    for(auto it = fgroup_set.begin(); it != fgroup_set.end(); it++){fgroups.push_back(*it);}
    return fgroups;
}

std::vector< std::pair< std::string, int>  > 
MHO_MK4StationInterface::GetFreqGroupPolInfo(int n309, type_309** t309, const std::string& fg, bool& same_size)
{
    same_size = false;
    //just use first AP
    int ap = 0;
    //first determine the set of polarizations and how many tones 
    std::set< std::string > pol_set;
    std::map< std::string, int> pol_count;
    std::string fgroup, sb, pol;
    int idx;
    for(int ch=0; ch < T309_MAX_CHAN; ch++)
    {
        std::string ch_name = getstr(&(t309[ap]->chan[ch].chan_name[0]), 8);
        if( ExtractChannelInfo(ch_name, fgroup, sb, pol, idx ) && fgroup == fg)
        {
            pol_set.insert(pol);
            int ntones = t309[0]->ntones;
            for(int ti=0; ti < ntones; ti++)
            {
                if(t309[0]->chan[ch].acc[ti][0] != 0 || t309[0]->chan[ch].acc[ti][1] != 0)
                {
                    pol_count[pol] += 1;
                }
            }
        }
    }

    std::vector< std::pair<std::string, int> > pol_info;
    if(pol_set.size() == 0){return pol_info;}

    same_size = true;
    int ntones = pol_count.begin()->second;
    for(auto it = pol_set.begin(); it != pol_set.end(); it++)
    {
        int count =  pol_count[*it];
        if(count != ntones){same_size = false;}
        pol_info.push_back( std::make_pair( *it, count ) );
    }
    return pol_info;
}

// 
// std::vector< std::string > 
// MHO_MK4StationInterface::GetFreqGroupPolChannels(int n309, type_309** t309, std::string fg, std::string p)
// {
//     //first determine the set of polarizations and freq groups which are present
//     std::set< std::string > chan_set;
//     for(int i=0; i < n309; i++)
//     {
//         for(int ch=0; ch < T309_MAX_CHAN; ch++)
//         {
//             std::string ch_name = getstr(&(t309[i]->chan[ch].chan_name[0]), 8);
//             if(ch_name != "")
//             {
//                 std::string fgroup = FreqGroupFromMK4ChannelID(ch_name);
//                 std::string pol = PolFromMK4ChannelID(ch_name);
//                 if(fgroup = fg && pol = p)
//                 {
//                     std::string sb = SidebandFromMK4ChannelId(ch_name);
//                     int idx = IndexFromMK4ChannelId(ch_name);
//                     if(ChannelInfoOK(fgroup, sb, pol, idx)){chan_set.insert(ch_name);}
//                 }
//             }
//         }
//     }
// 
//     std::vector< std::string > chans;
//     for(auto it = chan_set.begin(); it != chan_set.end(); it++)
//     {
//         chans.push_back(*it);
//     }
//     return chans;
// }




void 
MHO_MK4StationInterface::ExtractPCal(int n309, type_309** t309)
{
    if(n309 == 0){return;}

    std::size_t naps = n309; //number of APs
    auto fgroups = GetFreqGroups(n309, t309); //determine the frequency groups
    std::size_t nfg = fgroups.size();
    if(nfg == 0){return;}

    //pcal data for each frequency group
    std::vector< multitone_pcal_type > fgroup_pcal; //multitone_pcal_type dims = pol x time x freq
    fgroup_pcal.resize(nfg);

    //for each freq group
    for(std::size_t n=0; n<nfg; n++)
    {
        //grab the pols and number of tones for each pol
        bool same_size = false;
        auto pol_info = GetFreqGroupPolInfo(n309, t309, fgroups[n], same_size);
        std::size_t npols = pol_info.size();
        if(npols != 0 && same_size)
        {
            std::size_t total_ntones = pol_info.begin()->second;
            //resize the pcal data array 
            fgroup_pcal[n].Resize(npols, naps, total_ntones);
            fgroup_pcal[n].ZeroArray();

            std::cout<<"PCAL DATA ARRAY SIZE = ("<<npols<<", "<<naps<<", "<<total_ntones<<")"<<std::endl;

            //fill the pcal data with the tone phasors
            for(std::size_t p=0; p<pol_info.size(); p++)
            {
                std::string pol = pol_info[p].first;
                FillPCalArray(fgroups[n], pol, &(fgroup_pcal[n]), n309, t309);
            }
        }
        else 
        {
            msg_error("mk4interface", "differing number of pcal tones for each polaization in frequency group: "<< fgroups[n] << eom);
        }
    }
}

void 
MHO_MK4StationInterface::FillPCalArray(const std::string& fgroup, const std::string& pol, multitone_pcal_type* pc, int n309, type_309** t309)
{




}







// 
// 
// 
// 
// 
// 
//     std::set< std::string > chanids;
//     std::map< std::string, int> chan2idx; //map mk4 channel id to 'order' index
//     std::map< std::string, std::string> chan2pol; //map mk4 channel id to pol string
// 
//     //needed to determine the number of tones
//     std::map< std::string, int> chan2ntones; //count tones per each channel
//     std::map< std::string, int> chan2start; //the start index of the each channel
//     std::map< std::string, int > pol2ntones; //count tones per each pol
// 
//     //loop over all records and extract info from the mk4 names
//     int n_channels = 0;
//     int max_tones_per_channel = 0;
// 
//     for(int i=0; i < n309; i++)
//     {
//         for(int ch=0; ch < T309_MAX_CHAN; ch++)
//         {
//             std::string ch_name = getstr(&(t309[i]->chan[ch].chan_name[0]), 8);
//             if(ch_name != "")
//             {
//                 chanids.insert(ch_name);
//                 std::string fgroup = FreqGroupFromMK4ChannelID(ch_name);
//                 std::string sb = SidebandFromMK4ChannelId(ch_name);
//                 std::string pol = PolFromMK4ChannelID(ch_name);
//                 int idx = IndexFromMK4ChannelId(ch_name);
// 
//                 if(ChannelInfoOK(fgroup, sb, pol, idx))
//                 {
//                     chan2pol[ch_name] = pol;
//                     auto ib_pair = pol_set.insert(pol);
//                     if(ib_pair.second == true){pol2ntones[pol] = 0;} //insertion was successful, so init count to zero
//                     int count = 0; 
//                     int ntones = t309[i]->ntones;
//                     bool first = true;
//                     for(int ti=0; ti < ntones; ti++)
//                     {
//                         if(t309[i]->chan[ch].acc[ti][0] != 0 || t309[i]->chan[ch].acc[ti][1] != 0)
//                         {
//                             if(first){ chan2start[ch_name] = ti; first = false;}
//                             count++;
//                         }
//                     }
//                     chan2ntones[ch_name] = count;
//                     pol2ntones[pol] += count; //this is ok because mapped int value has already been initialized to zero
//                     if(count > max_tones_per_channel){max_tones_per_channel = count;}
//                 }
//             }
//         }
//     }
// 
//     n_channels = chanids.size(); //total count of channels *(includes both polarizations)
//     std::size_t nap = n309;
//     std::size_t npol = pol_set.size();
//     std::size_t ntotal_tones = 0;
//     std::set<int> tmp;
//     for(auto it = pol2ntones.begin(); it != pol2ntones.end(); it++)
//     {
//         tmp.insert(it->second);
//     }
// 
//     //error out if number of tones for each pol do not match! 
//     //will not create a pcal object for this station
//     if(tmp.size() == 1)
//     {
//         ntotal_tones = *(tmp.begin())/nap; 
//     }
//     else{msg_error("mk4interface", "cannot create pcal object, number of tones per polarization do no match.");}
// 
// 
//     std::cout<<"NAP = "<<nap<<std::endl;
//     std::cout<<"NPOL = "<<npol<<std::endl;
//     std::cout<<"NCHAN = "<<n_channels<<std::endl;
//     std::cout<<"MAXTONES PER CHAN = "<<max_tones_per_channel<<std::endl;
//     std::cout<<"N TOTAL TONES = "<<ntotal_tones<<std::endl;
// 
//     for(auto it = pol_set.begin(); it != pol_set.end(); it++)
//     {
//         std::cout<<"pcal pol present = "<<*it<<std::endl;
//     }
// 
//     //resize the pcal array 
//     pcal.Resize(npol,nap,ntotal_tones);
//     pcal.ZeroArray();
// 
//     for(int i=0; i < n309; i++)
//     {
//         std::size_t ap = (std::size_t) i;
//         int su = t309[i]->su;
//         int ntones = t309[i]->ntones;
//         double rot = t309[i]->rot;
//         double acc_period = t309[i]->acc_period;
// 
//         for(int ch=0; ch < T309_MAX_CHAN; ch++)
//         {
//             std::string ch_name = getstr(&(t309[i]->chan[ch].chan_name[0]), 8);
//             std::string pol = PolFromMK4ChannelID(ch_name);  
//             int channel_idx = chan2idx[ch_name];
//             int start_idx = chan2start[ch_name];
//             int n_chan_tones = chan2ntones[ch_name];
//             for(int ti=0; ti < n_chan_tones ; ti++)
//             {
//                 int idx = ti+start_idx;
//                 std::complex<double> ph = ComputePhasor(t309[i]->chan[ch].acc[idx][0], t309[i]->chan[ch].acc[idx][1], 1.0, 1.0);
//             }
// 
//         }
//     }
// 
// }

// std::set< std::string > 
// MHO_MK4StationInterface::DeterminePCalPols(int n309, type_309** t309) const
// {
//     std::set< std::string > pset;
//     for(int i=0; i < n309; i++)
//     {
//         for(int ch=0; ch < T309_MAX_CHAN; ch++)
//         {
//             std::string ch_name( &(t309[i]->chan[ch].chan_name[0]), 8);
//             if(ch_name != "")
//             {
//                 std::string pol = PolFromMK4ChannelID(ch_name);
//                 if(pol != ""){pset.insert(pol);}
//             }
//         }
//     }
//     return pset;
// }

std::string MHO_MK4StationInterface::FreqGroupFromMK4ChannelID(std::string id) const
{
    //MK4 channel ID format looks like "freq group" + "index" + "sideband" + "pol"
    //e.g X22LY
    std::string fgroup = "";
    if(id.size() < 5){return fgroup;}
    return id.substr(0,1);
}

std::string 
MHO_MK4StationInterface::PolFromMK4ChannelID(std::string id) const
{
    std::string pol = "";
    if(id.size() < 5){return pol;}

    //MK4 channel ID format looks like "freq group" + "index" + "sideband" + "pol"
    //e.g X22LY
    std::string p = id.substr(4,1);

    //only allow the following values for polarization
    if(p == "R" || p == "L" || p == "X" || p == "Y" || p == "H" || p == "V"){pol = p;}
    else 
    {
        msg_warn("mk4interface", "error parsing mk4 channel id: "<<id<<" for polarization. "<< eom);
    }

    return pol;
}

std::string 
MHO_MK4StationInterface::SidebandFromMK4ChannelId(std::string id) const
{
    std::string sb = "";
    if(id.size() < 5){return sb;}

    //MK4 channel ID format looks like "freq group" + "index" + "sideband" + "pol"
    //e.g X22LY
    std::string side = id.substr(3,1);
    //only allow the following values for sideband
    if(side == "L" || side == "U" || side == "D"){sb = side;}
    else 
    {
        msg_warn("mk4interface", "error parsing mk4 channel id: "<<id<<" for sideband. "<< eom);
    }

    return sb;
}

int 
MHO_MK4StationInterface::IndexFromMK4ChannelId(std::string id) const
{
    int idx = -1;
    if(id.size() < 5){return idx;}

    //MK4 channel ID format looks like "freq group" + "index" + "sideband" + "pol"
    //e.g X22LY

    std::string index = id.substr(1,2);
    if(std::isdigit(index[0]) && std::isdigit(index[1]) )
    {
        idx = std::atoi( index.c_str() );
    }
    else 
    {
        msg_warn("mk4interface", "error parsing mk4 channel id: "<<id<<" for index. "<< eom);
    }

    return idx;

}

bool 
MHO_MK4StationInterface::ExtractChannelInfo(const std::string& ch_name, std::string& fgroup, std::string& sb, std::string& pol, int& index)
{
    fgroup = FreqGroupFromMK4ChannelID(ch_name);
    if(fgroup == ""){return false;}
    sb = SidebandFromMK4ChannelId(ch_name);
    if(sb == ""){return false;}
    pol = PolFromMK4ChannelID(ch_name);
    if(pol == ""){return false;}
    index = IndexFromMK4ChannelId(ch_name);
    if(index < 0){return false;}
    return true;
}



std::complex< double > 
MHO_MK4StationInterface::ComputePhasor(uint32_t real, uint32_t imag, double acc_period, double sample_period)
{
    double u, v;
    if( u < TWO31){u = real;}
    else{u = real - TWO32;}

    if( v < TWO31){v = imag;}
    else{v = imag - TWO32;}

    //scale such that 1000 = 100% correlation
    //and match SU phase by shifting 180 degrees
    //what is the origin of the hard-coded value 128?
    double pc_real = ( u*sample_period )/(-128.0 * acc_period);
    double pc_imag = ( v*sample_period )/(-128.0 * acc_period);
    auto cp = std::complex<double>(pc_real, pc_imag);
    return cp;
}



}//end of namespace
