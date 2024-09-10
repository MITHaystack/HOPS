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
    fHaveVex(false),
    fStation(nullptr)
{
    fStationFile = "";
    fVexFile = "";
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

void
MHO_MK4StationInterface::ReadVexFile()
{
    fHaveVex = false;
    MHO_MK4VexInterface vinter;
    vinter.OpenVexFile(fVexFile);
    fVex = vinter.GetVex();
    if( fVex.contains("$OVEX_REV") ){fHaveVex = true;}
    else
    {
        msg_debug("mk4interface", "Failed to read root (ovex) file: " << fVexFile << eom);
    }
}



station_coord_type*
MHO_MK4StationInterface::ExtractStationFile()
{
    ReadVexFile();
    ReadStationFile();

    station_coord_type* st_data = nullptr;

    if(fHaveStation && fHaveVex)
    {
        //first thing we have to do is figure out the data dimensions
        //the items stored in the mk4sdata objects are mainly:
        //(0) meta day about the spline model (type_300)
        //(1) delay spline polynomial coeff (type_301)
        //(2) parallatic angle spline coeff (type_303)
        //(3) uvw-coords spline coeff (type_303)
        //(4) phase-cal data (type_309)

        //determine the root code;
        std::size_t last_dot = fStationFile.find_last_of('.');
        fRootCode = "";
        if(last_dot != std::string::npos && last_dot < fStationFile.length() - 1)
        {
            fRootCode = fStationFile.substr(last_dot + 1);
            //trim to 6 chars if too long (this shouldn't happen)
            if(fRootCode.size() > 6){fRootCode.resize(6);}
        }

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
        fStationName = getstr(t300->name, 32); 
        fStationCode = getstr(t300->intl_id, 2);
        fStationMK4ID = getstr(&(t300->id), 1);

        //tag the station data structure with all the meta data from the type_300
        st_data->Insert(std::string("name"), std::string("station_data"));
        st_data->Insert(std::string("station_name"), fStationName);
        st_data->Insert(std::string("station_mk4id"), fStationMK4ID);
        st_data->Insert(std::string("station_code"), fStationCode);
        st_data->Insert(std::string("model_start"), model_start_date);
        st_data->Insert(std::string("nsplines"), fStation->t300->nsplines);
        st_data->Insert(std::string("model_interval"), spline_interval);
        st_data->Insert(std::string("origin"), std::string("mark4")); //add tag to indicate this was converted from mark4 data
        st_data->Insert(std::string("root_code"), fRootCode);

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
    else 
    {
        msg_error("mk4interface", "failed to convert station data for file: "<<fStationFile << eom);
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
            //int ntones = t309[0]->ntones;
            for(int ti=0; ti < T309_MAX_PHASOR; ti++)
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


void
MHO_MK4StationInterface::ExtractPCal(int n309, type_309** t309)
{
    if(n309 == 0){return;}

    std::size_t naps = n309; //number of APs
    auto fgroups = GetFreqGroups(n309, t309); //determine the frequency groups
    std::size_t nfg = fgroups.size();
    if(nfg == 0){return;}

    //pcal data for each frequency group
    fFreqGroupPCal.resize(nfg);

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
            fFreqGroupPCal[n].Resize(npols, naps, total_ntones);
            fFreqGroupPCal[n].ZeroArray();

            msg_debug("mk4interface", "constructing a pcal data from type_309s with dimensions ("<<npols<<", "<<naps<<", "<<total_ntones<<")"<< eom );

            //fill the pcal data with the tone phasors
            for(std::size_t p=0; p<pol_info.size(); p++)
            {
                std::string pol = pol_info[p].first;
                std::get<MTPCAL_POL_AXIS>(fFreqGroupPCal[n]).at(p) = pol; //label pol axis
                FillPCalArray(fgroups[n], pol, p, &(fFreqGroupPCal[n]), n309, t309);
            }
        }
        else
        {
            msg_error("mk4interface", "differing number of pcal tones for each polaization in frequency group: "<< fgroups[n] << eom);
        }

        std::get<MTPCAL_POL_AXIS>(fFreqGroupPCal[n]).Insert(std::string("name"), std::string("polarization"));
        std::get<MTPCAL_TIME_AXIS>(fFreqGroupPCal[n]).Insert(std::string("name"), std::string("time"));
        std::get<MTPCAL_TIME_AXIS>(fFreqGroupPCal[n]).Insert(std::string("units"), std::string("s"));
        //indicate this is not frequency! no units
        std::get<MTPCAL_FREQ_AXIS>(fFreqGroupPCal[n]).Insert(std::string("name"), std::string("tone_index"));

        //repair the tone frequency axis info
        RepairMK4PCData(fgroups[n], fFreqGroupPCal[n]);

        //tag this pcal data
        fFreqGroupPCal[n].Insert(std::string("name"), std::string("pcal"));
        fFreqGroupPCal[n].Insert(std::string("frequency_group"), fgroups[n]);
        fFreqGroupPCal[n].Insert(std::string("station_name"), fStationName);
        fFreqGroupPCal[n].Insert(std::string("station_mk4id"), fStationMK4ID);
        fFreqGroupPCal[n].Insert(std::string("station_code"), fStationCode);
        fFreqGroupPCal[n].Insert(std::string("origin"), std::string("mark4")); //add tag to indicate this was converted from mark4 data
        fFreqGroupPCal[n].Insert(std::string("sample_period_used"), 1.0); //indicate the sample_period needs rescaling
        fFreqGroupPCal[n].Insert(std::string("root_code"), fRootCode);
    }
}

void
MHO_MK4StationInterface::FillPCalArray(const std::string& fgroup, const std::string& pol, int pol_idx, multitone_pcal_type* pc, int n309, type_309** t309)
{
    //loop over all channels that match this freq group and pol
    //count the number of tones they each have and then order them by index

    //channel index -> <channel location, ntones>
    std::map< int, std::pair< int, int > > chan2ntones;
    std::map< int, std::pair< int, int > > chan2start;
    std::map< int, std::string > chan2name;
    std::map< int, std::string > chan2sideband;
    std::set< int > channel_idx_set;
    //just use first AP
    int ap = 0;
    std::string fg, sb, p;
    int idx;
    for(int ch=0; ch < T309_MAX_CHAN; ch++)
    {
        std::string ch_name = getstr(&(t309[ap]->chan[ch].chan_name[0]), 8);
        if( ExtractChannelInfo(ch_name, fg, sb, p, idx ) && fgroup == fg && pol == p )
        {
            channel_idx_set.insert(idx);
            int ntones = t309[ap]->ntones;
            int count = 0;
            int start = 0;
            bool first = true;
            for(int ti=0; ti < T309_MAX_PHASOR; ti++)
            {
                if(t309[ap]->chan[ch].acc[ti][0] != 0 || t309[ap]->chan[ch].acc[ti][1] != 0)
                {
                    if(first){start = ti; first = false;}
                    count++;
                }
            }
            chan2ntones[idx] = std::make_pair(ch, count);
            chan2start[idx] = std::make_pair(ch, start);
            chan2name[idx] = ch_name;
            chan2sideband[idx] = sb;
        }
    }

    //sort the channels by mk4 index
    std::vector<int> channel_idx;
    for(auto it = channel_idx_set.begin(); it != channel_idx_set.end(); it++){channel_idx.push_back(*it);}
    std::sort(channel_idx.begin(), channel_idx.end());

    //now loop over channels and AP's collecting tone phasors
    int naps = pc->GetDimension(MTPCAL_TIME_AXIS);
    int tone_idx = 0;
    for(std::size_t i=0; i<channel_idx.size(); i++)
    {
        int ch = channel_idx[i];
        int ch_loc = chan2start[ch].first;
        int start = chan2start[ch].second;
        int stop = start + chan2ntones[ch].second;
        int channel_start = tone_idx;
        std::string sb = chan2sideband[ch];

        int nt = stop - start;
        for(int ti=0; ti < nt; ti++)
        {
            TODO_FIXME_MSG("TODO FIXME -- check 309 tone order and conjugation for USB data.")
            int acc_idx = start + ti;  //USB should be: start + ti?
            if(sb == "L"){acc_idx = (stop-1) - ti;} //tone order for LSB channels
            for(ap=0; ap<naps; ap++)
            {
                double acc_period = t309[ap]->acc_period;
                uint32_t rc = t309[ap]->chan[ch_loc].acc[ acc_idx ][0];
                uint32_t ic = t309[ap]->chan[ch_loc].acc[ acc_idx ][1];
                //use 1.0 as sample_period since this data is not stored in the station data files
                //will have to re-scale this later (when we have access to channel bandwidth in multitone pcal application)
                // auto ph = ComputePhasor(rc, ic, acc_period, 1.0/(2.0*32.0*1e6) );
                auto ph = ComputePhasor(rc, ic, acc_period, 1.0 );

                //LSB tone's are flipped and conjugated (we ignore 2012 sign flip)
                if(sb == "L"){ph = -1.0*std::conj(ph);}
                pc->at(pol_idx, ap, tone_idx) = ph;
                std::get<MTPCAL_TIME_AXIS>(*pc).at(ap) = ap*acc_period;

            }
            //values are meaningless because the type309s do not carry tone frequency information;
            std::get<MTPCAL_FREQ_AXIS>(*pc).at(tone_idx) = 0;
            tone_idx++;
        }
        int channel_stop = tone_idx;
        //std::cout<<"filled tones for pol: "<<pol<<" channel: "<<chan2name[ch]<<" start/stop: "<<channel_start<<"/"<<channel_stop<<std::endl;
        std::string name_key = "channel_mk4id_";
        std::string index_key = "channel_index";
        name_key += pol;
        std::get<MTPCAL_FREQ_AXIS>(*pc).InsertIntervalLabelKeyValue(channel_start, channel_stop, name_key, chan2name[ch]);
        std::get<MTPCAL_FREQ_AXIS>(*pc).InsertIntervalLabelKeyValue(channel_start, channel_stop, index_key, ch);

    }
}


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
    double u = real;
    double v = imag;
    if(u > TWO31){u -= TWO32;}
    if(v > TWO31){v -= TWO32;}

    //scale such that 1000 = 100% correlation
    //and match SU phase by shifting 180 degrees
    //what is the origin of the hard-coded value 128?
    double pc_real = ( u*sample_period )/(-128.0 * acc_period);
    double pc_imag = ( v*sample_period )/(-128.0 * acc_period);
    auto cp = std::complex<double>(pc_real, pc_imag);
    return cp;
}

void 
MHO_MK4StationInterface::RepairMK4PCData(std::string freqGroup, multitone_pcal_type& pc_data)
{
    //we don't have visibility data at this point, so we are going to take the ovex 
    //and construct an equivalent channel axis, then we will use that to work out 
    //the tone frequencies -- obviously, we are trusting that the ovex is correct
    auto per_pol_chan_ax = ConstructPerPolChannelAxis(freqGroup);
    if(per_pol_chan_ax.size() == 0)
    {
        msg_error("mk4interface", "cannot determine channel information for pcal tone repair" << eom);
        return;
    }
    auto chan_ax = per_pol_chan_ax.begin()->second; //TODO FIXME , just using one (first) pol
    if(chan_ax.GetSize() == 0)
    {
        msg_error("mk4interface", "cannot determine channel information for pcal tone repair" << eom);
        return;
    }

    TODO_FIXME_MSG("TODO FIXME -- fix hardcoded pcal spacing!!")
    double pcal_spacing = 5.0; //TODO FIXME HARDCODED PCAL SPACING

    //only perform this operation if the pcal data originated from mark4 type309s
    std::string data_origin;
    pc_data.Retrieve("origin", data_origin);

    //first loop over the pcal freq axis and extract the channel indexes and ranges
    auto pc_tone_ax = &(std::get<MTPCAL_FREQ_AXIS>(pc_data));

    std::map< std::size_t, std::pair<std::size_t, std::size_t> > chanidx2range;

    auto interval_objs = pc_tone_ax->GetMatchingIntervalLabels("channel_index");
    for(std::size_t i=0; i<interval_objs.size(); i++)
    {
        std::size_t channel_idx = interval_objs[i]["channel_index"].get<int>();
        std::size_t low = interval_objs[i]["lower_index"].get<int>();
        std::size_t high = interval_objs[i]["upper_index"].get<int>();
        chanidx2range[channel_idx] = std::make_pair(low, high);
    }

    double sky_freq = 0;
    double bandwidth = 0;
    std::string net_sideband;

    //fix the tone frequency axis (deduce this from the channel boundaries and pcal tone spacing)
    for(auto it = chanidx2range.begin(); it != chanidx2range.end(); it++)
    {
        std::size_t ch = it->first; //channel index
        std::size_t start = it->second.first; //tone start index
        std::size_t stop = it->second.second; //tone stop index

        //make sure to grab the channel that matches the channel index
        //we need to do this in case the 'freqs' command has been applied to sub-select channels
        std::size_t ch_idx = 0;
        for(ch_idx=0; ch_idx<chan_ax.GetSize(); ch_idx++)
        {
            int chid = -1;
            chan_ax.RetrieveIndexLabelKeyValue(ch_idx, "channel", chid);
            if(chid == ch)
            {
                // std::string channel_label;
                // chan_ax.RetrieveIndexLabelKeyValue(ch_idx, "channel_label", channel_label);
                break;
            }
        }

        if( ch_idx < chan_ax.GetSize() ) //will be equal to chan_ax size if channel not found
        {
            //get the channel frequency info and range
            sky_freq = chan_ax.at(ch_idx); //get the sky frequency of this channel
            bandwidth = 0;
            net_sideband;
            bool key_present = chan_ax.RetrieveIndexLabelKeyValue(ch_idx, "net_sideband", net_sideband);
            if(!key_present){msg_error("calibration", "missing net_sideband label for channel "<< ch_idx << " with sky_freq: "<<sky_freq << eom); }
            key_present = chan_ax.RetrieveIndexLabelKeyValue(ch_idx, "bandwidth", bandwidth);
            if(!key_present){msg_error("calibration", "missing bandwidth label for channel "<< ch_idx << " with sky_freq: "<<sky_freq << eom);}

            //figure out the upper/lower frequency limits for this channel
            double lower_freq, upper_freq;
            std::size_t start_idx, ntones;
            DetermineChannelFrequencyLimits(sky_freq, bandwidth, net_sideband, lower_freq, upper_freq);

            //figure out the number of tones in this channel (better match stop-start)
            int c = std::floor(lower_freq/pcal_spacing);
            if( (lower_freq - c*pcal_spacing) > 0 ){c += 1;} //first tone in channel is c*pcal_spacing
            int d = std::floor(upper_freq/pcal_spacing);
            if( (upper_freq - d*pcal_spacing) > 0 ){d += 1;} //d is first tone just beyond the channel

            if( (d - c) == (stop - start) ) //number of tones matches
            {
                std::size_t ntones = stop - start;
                for(std::size_t ti=0; ti < ntones; ti++)
                {
                    //loop over the tone indexes and figure out the tone frequencies
                    //which are multiples of the pcal spacing within the channel
                    pc_tone_ax->at(start+ti) = (c+ti)*pcal_spacing;
                }
            }
        }
    }

    //rescale all the phasors by the sample_period
    double sample_period = 1.0/(2.0*bandwidth*1e6);
    (pc_data) *= sample_period;

}

std::map< std::string, channel_axis_type >
MHO_MK4StationInterface::ConstructPerPolChannelAxis(std::string freqGroup)
{
    std::map< std::string, channel_axis_type> per_pol_chan_ax;
    
    std::cout<<"Station info: "<<fStationName<<", "<<fStationCode<<", "<<fStationMK4ID<<std::endl;
    
    //assume we now have all ovex/vex in the fVex object, and that we only have a single scan
    //should only have a single 'scan' element under the schedule section, so find it
    auto sched = fVex["$SCHED"];
    mho_json scan;
    if(sched.size() != 1){msg_error("mk4interface", "could not read scan from $SCHED section of ovex/root file" << eom);}
    else
    {
        scan = sched.begin().value();
    }

    //TODO FIXME --for complicated schedules and/or zoom bands,
    //different stations may have different modes
    std::string mode_key = scan["mode"].get<std::string>();
    int nst = scan["station"].size(); // number of stations;

    //maps to resolve links
    std::map< std::string, std::string > stationCodeToSiteID;
    std::map< std::string, std::string > stationCodeToMk4ID;
    std::map< std::string, std::string > stationCodeToFreqTableName;
    std::map< std::string, std::string > mk4IDToFreqTableName;

    auto mode = fVex["$MODE"][mode_key];
    // //TODO FIXME -- this is incorrect if there are multple BBC/IFs defined
    std::string bbc_name = fVex["$MODE"][mode_key]["$BBC"][0]["keyword"].get<std::string>(); //TODO FIXME if stations have different bbcs
    std::string if_name = fVex["$MODE"][mode_key]["$IF"][0]["keyword"].get<std::string>(); //TODO FIXME if stations have different ifs

    //find the frequency table for this station
    //first locate the mode info
    std::string freq_key;
    std::map< std::string, std::vector< mho_json> > channel_info;
    for(auto it = mode["$FREQ"].begin(); it != mode["$FREQ"].end(); ++it)
    {
        std::string keyword = (*it)["keyword"].get<std::string>();
        std::size_t n_qual = (*it)["qualifiers"].size();
        for(std::size_t q=0; q < n_qual; q++)
        {
            std::string station_code = (*it)["qualifiers"][q].get<std::string>();
            if(station_code == fStationCode)
            {
                std::cout<<"found the station code:"<<fStationCode<<std::endl;
                std::cout<<"assuming mk4id :"<<fStationMK4ID<<std::endl;
                
                std::string freq_table = keyword;
                
                //get the channel information for this station
                for(std::size_t nch=0; nch < fVex["$FREQ"][freq_table]["chan_def"].size(); nch++)
                {
                    std::string chan_name = fVex["$FREQ"][freq_table]["chan_def"][nch]["channel_name"].get<std::string>(); //ovex specialty
                    double sky_freq = fVex["$FREQ"][freq_table]["chan_def"][nch]["sky_frequency"]["value"].get<double>();
                    double bw = fVex["$FREQ"][freq_table]["chan_def"][nch]["bandwidth"]["value"].get<double>();
                    std::string net_sb = fVex["$FREQ"][freq_table]["chan_def"][nch]["net_sideband"].get<std::string>();
                    std::string bbc_id = fVex["$FREQ"][freq_table]["chan_def"][nch]["bbc_id"].get<std::string>();
                    std::string pol = "-";
                    for(std::size_t nbbc=0; nbbc< fVex["$BBC"][bbc_name]["BBC_assign"].size(); nbbc++)
                    {
                        if( fVex["$BBC"][bbc_name]["BBC_assign"][nbbc]["logical_bbc_id"].get<std::string>() == bbc_id )
                        {
                            std::string if_id = fVex["$BBC"][bbc_name]["BBC_assign"][nbbc]["logical_if"].get<std::string>();
                            //finally retrieve the polarization
                            for(std::size_t nif = 0; nif < fVex["$IF"][if_name]["if_def"].size(); nif++)
                            {
                                if(fVex["$IF"][if_name]["if_def"][nif]["if_id"].get<std::string>() == if_id)
                                {
                                    pol = fVex["$IF"][if_name]["if_def"][nif]["polarization"].get<std::string>();
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    mho_json ch_label;
                    ch_label["sky_freq"] =  sky_freq;
                    ch_label["bandwidth"] = bw;
                    ch_label["net_sideband"] = net_sb;
                    ch_label["channel_name"] = chan_name;
                    ch_label["pol"] = pol;
                    
                    std::string fg = FreqGroupFromMK4ChannelID(chan_name);
                    //only insert channel info which is part of this frequency group
                    if(fg == freqGroup){ channel_info[pol].push_back(ch_label); }
                    
                    std::cout<<"channel info = "<<chan_name<<", "<<sky_freq<<", "<<bw<<", "<<net_sb<<", "<<bbc_id<<", "<<pol<<std::endl;
                }
            }
        }
    }

    //sort by sky frequency 
    for(auto it = channel_info.begin(); it != channel_info.end(); it++)
    {
        std::sort(it->second.begin(), it->second.end(), fChannelPredicate);
    }

    //construct the channel axis for each pol
    for(auto it = channel_info.begin(); it != channel_info.end(); it++)
    {
        std::size_t nchan = it->second.size();
        channel_axis_type ax(nchan);
        for(std::size_t i=0; i<nchan; i++)
        {
            mho_json ch_label = it->second.at(i);
            ch_label["channel"] = i;
            double sky_freq = ch_label["sky_freq"].get<double>();
            ax.at(i) = sky_freq;
            ax.SetLabelObject(ch_label, i);
        }
        std::string pol = it->first;
        per_pol_chan_ax[pol] = ax;
    }

    return per_pol_chan_ax;
}


void
MHO_MK4StationInterface::DetermineChannelFrequencyLimits(double sky_freq, double bandwidth, std::string net_sideband, double& lower_freq, double& upper_freq)
{
    lower_freq = sky_freq;
    upper_freq = sky_freq;    
    
    if(net_sideband == "U")
    {
        lower_freq = sky_freq;
        upper_freq = sky_freq + bandwidth;
    }
    
    if(net_sideband == "L")
    {
        upper_freq = sky_freq;
        lower_freq = sky_freq - bandwidth;
    }

}

}//end of namespace
