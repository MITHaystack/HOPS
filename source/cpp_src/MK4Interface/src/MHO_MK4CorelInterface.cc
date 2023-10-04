#include "MHO_MK4CorelInterface.hh"
#include "MHO_LegacyDateConverter.hh"

#include <vector>
#include <cstdlib>
#include <cstring>
#include <complex>
#include <set>
#include <algorithm>

//mk4 IO library
#ifndef HOPS3_USE_CXX
extern "C"
{
#endif
    #include "mk4_records.h"
    #include "mk4_data.h"
    #include "mk4_dfio.h"
#ifndef HOPS3_USE_CXX
}
#endif


namespace hops
{


//the ordering operator for channel labels to sort by frequency;
class chan_label_freq_predicate
{
    public:
        chan_label_freq_predicate(){};
        virtual ~chan_label_freq_predicate(){};

    virtual bool operator()(const MHO_IntervalLabel* a, const MHO_IntervalLabel* b)
    {
        double a_frq, b_frq;
        a->Retrieve(std::string("sky_freq"), a_frq);
        b->Retrieve(std::string("sky_freq"), b_frq);
        return a_frq < b_frq;
    }
};



MHO_MK4CorelInterface::MHO_MK4CorelInterface():
    fHaveCorel(false),
    fHaveVex(false),
    fCorel(nullptr),
    fExtractedVisibilities(nullptr),
    fExtractedWeights(nullptr)
{
    fCorel = (struct mk4_corel *) calloc ( 1, sizeof(struct mk4_corel) );
    fNPPs = 0;
    fNAPs = 0;
    fNSpectral = 0;
    fNChannels = 0;
    fNChannelsPerPP = 0;
    fPolProducts.clear();
    fAllChannelMap.clear();
    fPPSortedChannelInfo.clear();
    fBaselineName = "";
    fBaselineShortName= "";
    fRefStation= "";
    fRemStation = "";
    fRefStationMk4Id = "";
    fRemStationMk4Id = "";
}

MHO_MK4CorelInterface::~MHO_MK4CorelInterface()
{
    clear_mk4corel(fCorel);
    free(fCorel);
}

void
MHO_MK4CorelInterface::ReadCorelFile()
{
    if(fHaveCorel)
    {
        msg_debug("mk4interface", "Clearing a previously exisiting corel struct."<< eom);
        clear_mk4corel(fCorel);
        fCorel = nullptr;
        fHaveCorel = false;
    }

    //have to copy fCorelFile for const_cast, as mk4 lib doesn't respect const
    std::string fname = fCorelFile;
    int retval = read_mk4corel( const_cast<char*>(fname.c_str()), fCorel );
    if(retval != 0)
    {
        fHaveCorel = false;
        msg_debug("mk4interface", "Failed to read corel file: "<< fCorelFile << ", error value: "<< retval << eom);
    }
    else
    {
        fHaveCorel = true;
        msg_debug("mk4interface", "Successfully read corel file."<< fCorelFile << eom);
    }

    fBaselineName = "";
    fBaselineShortName= "";
    fRefStation= "";
    fRemStation = "";
    fRefStationMk4Id = "";
    fRemStationMk4Id = "";
}


void
MHO_MK4CorelInterface::ReadVexFile()
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





void
MHO_MK4CorelInterface::DetermineDataDimensions()
{   
    //We need to determine 4 things:
    //(1) number of pol-products (npp)
    //(2) number of APs (nap)
    //(3) number of lags-per-channel (nlpc)
    //(4) number of channels per pol-product (ncpp)
    //final data array has dimensions npp X nap X (ncpp X nlpc)
    //figure out the max number of APs and max lags (spectral points),
    //as well as the total number of channels and their names
    fNPPs = 0;
    fNAPs = 0;
    fNSpectral = 0;
    fNChannels = 0;
    fNChannelsPerPP = 0;
    //create a map of channel-pairs to interval labels
    //to be filled in with channel information
    fAllChannelMap.clear();
    //std::set< int > valid_aps;

    std::string baseline = getstr(fCorel->t100->baseline, 2);
    fBaselineShortName = baseline;
    msg_debug("mk4interface", "Reading data for baseline: " << baseline << eom);

    // struct mk4_corel::index_tag* idx;
    struct index_tag* idx;
    for(int i=0; i<fCorel->index_space; i++)
    {
        idx = fCorel->index + i;
        for(int ap=0; ap<idx->ap_space; ap++)
        {
            struct type_120* t120 = idx->t120[ap];
            if(t120 != nullptr)
            {
                if(t120->type == SPECTRAL)
                {
                    //valid_aps.insert(ap);
                    std::size_t nlags = t120->nlags;
                    //this implicitly assumes all channels have the same number
                    //of spectral points
                    if(fNSpectral < nlags){fNSpectral = nlags;}
                    if(ap > 0){ if( fNAPs < (std::size_t)ap ){fNAPs = (std::size_t)ap; }; };
                }
                else
                {
                    msg_fatal("mk4interface", "Non-spectral type-120 not supported." << eom);
                }
            }
        }

        struct type_101* t101;
        if( (t101 = idx->t101) != NULL)
        {
            std::string ref_chan_id = getstr(t101->ref_chan_id,8);
            std::string rem_chan_id = getstr(t101->rem_chan_id,8);
            MHO_IntervalLabel tmp_label;
            tmp_label.Insert(std::string("ref_chan_id"), ref_chan_id);
            tmp_label.Insert(std::string("rem_chan_id"), rem_chan_id);
            std::string tmp_key = ref_chan_id + ":" + rem_chan_id;
            fAllChannelMap[tmp_key] = tmp_label;
        }
    }
    fNChannels = fAllChannelMap.size();
    fNAPs += 1;
    //fNAPs = valid_aps.size();




    #ifdef  HOPS_ENABLE_DEBUG_MSG
    msg_debug("mk4interface", "Total number of channel pairs = "<< fNChannels << eom );
    for(auto it = fAllChannelMap.begin(); it != fAllChannelMap.end(); it++)
    {
        msg_debug("mk4interface", " channel pair = "<<it->first<< eom);
    }
    #endif


    //assume we now have all ovex/vex in the fVex object, and that we only have a single scan
    //should only have a single 'scan' element under the schedule section, so find it
    auto sched = fVex["$SCHED"];
    if(sched.size() != 1)
    {
        msg_error("mk4interface", "OVEX file schedule section contains more than one scan."<<eom);
    }
    auto scan = sched.begin().value();

    //TODO FIXME --for complicated schedules, different stations may have different modes
    std::string mode_key = scan["mode"].get<std::string>();

    int nst = scan["station"].size(); // number of stations;

    //maps to resolve links
    // std::map< std::string, std::string > stationCodeToFreqTableName;
    std::map< std::string, std::string > mk4IDToFreqTableName;
    std::map< std::string, std::string > mk4IDToStationCode;

    auto mode = fVex["$MODE"][mode_key];
    //TODO FIXME -- this is incorrect if there are multple BBC/IFs defined
    std::string bbc_name = fVex["$MODE"][mode_key]["$BBC"][0]["keyword"].get<std::string>(); //TODO FIXME if stations have different bbcs
    std::string if_name = fVex["$MODE"][mode_key]["$IF"][0]["keyword"].get<std::string>(); //TODO FIXME if stations have different ifs

    for(int ist = 0; ist<nst; ist++)
    {
        //find the frequency table for this station 
        //first locate the mode info 
        std::string freq_key;
        for(auto it = mode["$FREQ"].begin(); it != mode["$FREQ"].end(); ++it)
        {
            std::string keyword = (*it)["keyword"].get<std::string>();
            std::size_t n_qual = (*it)["qualifiers"].size();
            for(std::size_t q=0; q<n_qual; q++)
            {
                std::string station_code = (*it)["qualifiers"][q].get<std::string>();
                // stationCodeToFreqTableName[station_code] = keyword;
                //std::string site_key = 
                std::string site_key = fVex["$STATION"][station_code]["$SITE"][0]["keyword"].get<std::string>();
                std::string mk4_id = fVex["$SITE"][site_key]["mk4_site_ID"].get<std::string>();
                mk4IDToFreqTableName[mk4_id] = keyword;
                mk4IDToStationCode[mk4_id] = station_code;
            }
        }
    }

    // //now we need to fill in the channel labels with information from the vex
    // TODO FIXME -- refactor the following section --
    //the ovex data retrieval (just to get the channel polariztaion!!!) is rather convoluted
    // we may want to add a feature to resolve vex link-words to json objects or json paths
    //which would simplfy this mess
    std::string ref_st = std::string(&(baseline[0]),1);
    std::string rem_st = std::string(&(baseline[1]),1);
    fRefStationMk4Id = ref_st;
    fRemStationMk4Id = rem_st;
    fRefStation = mk4IDToStationCode[ref_st];
    fRemStation = mk4IDToStationCode[rem_st];
    fBaselineName = fRefStation + ":" + fRemStation;
    double ref_sky_freq, ref_bw, rem_sky_freq, rem_bw;
    std::string ref_net_sb, rem_net_sb, ref_pol, rem_pol;
    bool found_ref = false;
    bool found_rem = false;
    fPolProducts.clear();

    for(auto ch = fAllChannelMap.begin(); ch != fAllChannelMap.end(); ch++)
    {
        std::string ref_chan_id, rem_chan_id;
        ch->second.Retrieve(std::string("ref_chan_id"), ref_chan_id);
        ch->second.Retrieve(std::string("rem_chan_id"), rem_chan_id);
        found_ref = false;
        found_rem = false;

        std::string ref_freq_table = mk4IDToFreqTableName[ref_st];
        std::string rem_freq_table = mk4IDToFreqTableName[rem_st];

        //get the channel information of the reference station
        for(std::size_t nch=0; nch < fVex["$FREQ"][ref_freq_table]["chan_def"].size(); nch++)
        {
            std::string chan_name = fVex["$FREQ"][ref_freq_table]["chan_def"][nch]["channel_name"].get<std::string>();
            if(chan_name == ref_chan_id)
            {
                ref_sky_freq = fVex["$FREQ"][ref_freq_table]["chan_def"][nch]["sky_frequency"]["value"].get<double>();
                ref_bw = fVex["$FREQ"][ref_freq_table]["chan_def"][nch]["bandwidth"]["value"].get<double>();
                ref_net_sb = fVex["$FREQ"][ref_freq_table]["chan_def"][nch]["net_sideband"].get<std::string>();
                std::string bbc_id = fVex["$FREQ"][ref_freq_table]["chan_def"][nch]["bbc_id"].get<std::string>();
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
                                ref_pol = fVex["$IF"][if_name]["if_def"][nif]["polarization"].get<std::string>();
                                break;
                            }
                        }
                        break;
                    }
                }
                found_ref = true;
                break;
            }
        }


        for(std::size_t nch=0; nch < fVex["$FREQ"][rem_freq_table]["chan_def"].size(); nch++)
        {
            std::string chan_name = fVex["$FREQ"][rem_freq_table]["chan_def"][nch]["channel_name"].get<std::string>();
            if(chan_name == rem_chan_id)
            {
                rem_sky_freq = fVex["$FREQ"][rem_freq_table]["chan_def"][nch]["sky_frequency"]["value"].get<double>();
                rem_bw = fVex["$FREQ"][rem_freq_table]["chan_def"][nch]["bandwidth"]["value"].get<double>();
                rem_net_sb = fVex["$FREQ"][rem_freq_table]["chan_def"][nch]["net_sideband"].get<std::string>();

                std::string bbc_id = fVex["$FREQ"][rem_freq_table]["chan_def"][nch]["bbc_id"].get<std::string>();

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
                                rem_pol = fVex["$IF"][if_name]["if_def"][nif]["polarization"].get<std::string>();
                                break;
                            }
                        }
                        break;
                    }
                }
                found_rem = true;
                break;
            }
        }

        

        if(found_ref && found_rem)
        {
            std::string pp = ref_pol + rem_pol;
            // pp.append(1,ref_pol[0]);
            // pp.append(1,rem_pol[1]);
            fPolProducts.insert(pp);

            ch->second.Insert(std::string("ref_sky_freq"), ref_sky_freq);
            ch->second.Insert(std::string("ref_bandwidth"), ref_bw);
            ch->second.Insert(std::string("ref_net_sideband"), ref_net_sb);
            ch->second.Insert(std::string("ref_polarization"), ref_pol);
            ch->second.Insert(std::string("rem_sky_freq"), rem_sky_freq);
            ch->second.Insert(std::string("rem_bandwidth"), rem_bw);
            ch->second.Insert(std::string("rem_net_sideband"), rem_net_sb);
            ch->second.Insert(std::string("rem_polarization"), rem_pol);
            ch->second.Insert(std::string("pol_product"), pp);

            //also attach some 'common across all pol-products' labels
            if(channel_info_match(ref_sky_freq, rem_sky_freq, ref_bw, rem_bw, ref_net_sb, rem_net_sb) )
            {
                ch->second.Insert(std::string("sky_freq"), ref_sky_freq);
                ch->second.Insert(std::string("bandwidth"), ref_bw);
                ch->second.Insert(std::string("net_sideband"), ref_net_sb);
            }
            else
            {
                msg_error("mk4interface", "Mis-matched channel information in pair: " << ch->first << eom);
            }

            // #ifdef HOPS_ENABLE_DEBUG_MSG
            // msg_debug("mk4interface", "Dumping channel information for: " << ch->first << eom);
            // ch->second.DumpMap<std::string>();
            // ch->second.DumpMap<double>();
            // ch->second.DumpMap<char>();
            // ch->second.DumpMap<int>();
            // #endif
        }
    }

    fNPPs = fPolProducts.size();
    //for the time being we are implicitly assuming the frequency set-up
    //is the same for each pol-product
    //but we count the number of channels per pol-product to make sure
    std::map< std::string, std::set< MHO_IntervalLabel* > > pp_chan_set_map;

    //initialize with empty set/vectors for each pol-product
    for(auto it = fPolProducts.begin(); it != fPolProducts.end(); it++)
    {
        pp_chan_set_map[*it] = std::set<MHO_IntervalLabel*>();
        fPPSortedChannelInfo[*it] = std::vector< MHO_IntervalLabel* >();
    }

    //now separate out channel labels by pol-product
    for(auto it = fAllChannelMap.begin(); it != fAllChannelMap.end(); it++)
    {
        std::string pol1, pol2;
        it->second.Retrieve("ref_polarization", pol1);
        it->second.Retrieve("rem_polarization", pol2);
        std::string ppkey;
        ppkey.append(pol1);
        ppkey.append(pol2);

        auto indicator = pp_chan_set_map[ppkey].insert( &(it->second) );
        if(indicator.second)
        {
            fPPSortedChannelInfo[ppkey].push_back( &(it->second) );
        }
    }

    //make sure all pol-products share the same number of channels
    fNChannelsPerPP = 0;
    for(auto it = pp_chan_set_map.begin(); it != pp_chan_set_map.end(); it++)
    {
        if(fNChannelsPerPP == 0){fNChannelsPerPP = it->second.size();}
        else
        {
            if(fNChannelsPerPP != it->second.size() )
            {
                msg_error("mk4interface",
                    "Not all pol-products have the same number of channels! " <<
                    fNChannelsPerPP << " !=" << it->second.size() << eom );
            }
        }
    }

    msg_debug("mk4interface", "Number of channels per pol product: "<< fNChannelsPerPP << eom);

    //now sort the channels by sky frequency
    chan_label_freq_predicate sort_pred;
    for(auto it = fPolProducts.begin(); it != fPolProducts.end(); it++)
    {
        std::sort( fPPSortedChannelInfo[*it].begin(), fPPSortedChannelInfo[*it].end(), sort_pred);
    }

}

void
MHO_MK4CorelInterface::ExtractCorelFile()
{
    ReadCorelFile();
    ReadVexFile();

    uch_visibility_store_type* bl_data = nullptr;
    uch_weight_store_type* bl_wdata = nullptr;

    if(fHaveCorel && fHaveVex)
    {

        DetermineDataDimensions();

        double ap_time_length = 1.0; //defaults to 1 sec
        if(fVex.contains("$EVEX") && fVex["$EVEX"].size() == 1)
        {
            ap_time_length = (fVex["$EVEX"].begin().value())["AP_length"]["value"].get<double>();
        }
        else 
        {
            msg_warn("mk4interface", "warning, could not find AP_length information root (ovex) file."<<eom);
        }

        //now we can go ahead an create containers for all the visibilities and data-weights
        //the data weights container has the same layout as the visibilities (so axis + label data is stored twice)
        msg_debug("mk4interface", "Number of pol-products = " << fNPPs << eom );
        msg_debug("mk4interface", "Number of APs = " << fNAPs << eom );
        msg_debug("mk4interface", "Number of channels per pol-product = " << fNChannelsPerPP << eom);
        msg_debug("mk4interface", "Number of spectral points = " << fNSpectral << eom);

        std::size_t bl_dim[VIS_NDIM] = {fNPPs, fNAPs, (fNChannelsPerPP*fNSpectral)};
        bl_data = new uch_visibility_store_type(bl_dim);
        bl_wdata = new uch_weight_store_type(bl_dim);

        //first label the pol-product axis
        std::size_t pp_count = 0;
        std::map<std::string, std::size_t> pp_index_lookup;
        for(auto it = fPolProducts.begin(); it != fPolProducts.end(); it++)
        {
            pp_index_lookup[*it] = pp_count;
            std::get<UCH_POLPROD_AXIS>(*bl_data)[pp_count] = *it;
            std::get<UCH_POLPROD_AXIS>(*bl_wdata)[pp_count] = *it;
            pp_count++;
        }

        //now assign values to the time (0-th dim) axis
        for(size_t ap=0; ap<fNAPs; ap++)
        {
            std::get<UCH_TIME_AXIS>(*bl_data)[ap] = ap_time_length*ap;
            std::get<UCH_TIME_AXIS>(*bl_wdata)[ap] = ap_time_length*ap;
        }

        //finally we need to label the frequency axis
        //this is trickier because the frequency axis is not continuous...
        //for the time being we assume the all pol-products have the same freq-axis
        //so if true, the outer loop is actually unecessary
        std::set< int > inserted_channel_labels;
        for(auto ppit = fPolProducts.begin(); ppit != fPolProducts.end(); ppit++ )
        {
            std::size_t freq_count = 0;
            int ch_count = 0;
            double sky_freq, bw;
            std::string net_sb;
            for(auto it = fPPSortedChannelInfo[*ppit].begin();
                it != fPPSortedChannelInfo[*ppit].end();
                it++)
            {
                (*it)->Retrieve(std::string("sky_freq"), sky_freq);
                (*it)->Retrieve(std::string("bandwidth"), bw);
                (*it)->Retrieve(std::string("net_sideband"), net_sb);
                //add the freq-axis bounds info for this channel
                (*it)->SetBounds(freq_count, freq_count + fNSpectral);
                //add a common channel ID, for now this is just an integer
                //but eventually it could be anything
                (*it)->Insert(std::string("channel"), ch_count);

                //ought to add mk4 style channel ids, e.g. X08LX:X08LY?
                (*it)->Insert(std::string("chan_id"), std::string("placeholder"));//placeholder for now

                //if not present, insert a clean channel label on this axis
                auto indicator = inserted_channel_labels.insert(ch_count);
                if(indicator.second)
                {
                    MHO_IntervalLabel ch_label;
                    ch_label.Insert(std::string("sky_freq"), sky_freq);
                    ch_label.Insert(std::string("bandwidth"), bw);
                    ch_label.Insert(std::string("net_sideband"), net_sb);
                    ch_label.Insert(std::string("channel"), ch_count);
                    ch_label.SetBounds(freq_count, freq_count + fNSpectral);
                    std::get<UCH_FREQ_AXIS>(*bl_data).InsertLabel(ch_label);
                    std::get<UCH_FREQ_AXIS>(*bl_wdata).InsertLabel(ch_label);
                }

                //set up this portion of the frequency axis
                for(std::size_t sp=0; sp<fNSpectral; sp++)
                {
                    int findex = 0;
                    if(net_sb == "U"){findex = sp;};
                    if(net_sb == "L"){findex = fNSpectral-sp-1;}
                    double freq = calc_freq_bin(sky_freq, bw, net_sb, fNSpectral, findex);
                    std::get<UCH_FREQ_AXIS>(*bl_data).at(freq_count) = freq;
                    std::get<UCH_FREQ_AXIS>(*bl_wdata).at(freq_count) = freq;
                    freq_count++;
                }
                ch_count++;
            }
        }

        #ifdef HOPS_ENABLE_DEBUG_MSG
        //lets print out the pol, time and freq axes now:
        for(std::size_t i=0; i< std::get<UCH_POLPROD_AXIS>(*bl_data).GetSize(); i++)
        {
            msg_debug("mk4interface", "pol_axis: "<<i<<" = "<<std::get<UCH_POLPROD_AXIS>(*bl_data).at(i)<< eom);
        }
        
        for(std::size_t i=0; i< std::get<UCH_TIME_AXIS>(*bl_data).GetSize(); i++)
        {
            msg_debug("mk4interface", "time_axis: "<<i<<" = "<<std::get<UCH_TIME_AXIS>(*bl_data).at(i)<<eom);
        }
        
        // for(std::size_t i=0; i< std::get<UCH_FREQ_AXIS>(*bl_data).GetSize(); i++)
        // {
        //     msg_debug("mk4interface", "freq_axis: "<<i<<" = "<<std::get<UCH_FREQ_AXIS>(*bl_data).at(i)<<eom);
        // }
        #endif

        //now fill in the actual visibility data
        struct type_101* t101 = nullptr;
        //struct mk4_corel::index_tag* idx = nullptr;
        struct index_tag* idx = nullptr;
        for(int i=0; i<fCorel->index_space; i++)
        {
            idx = fCorel->index + i;
            if( (t101 = idx->t101) != nullptr)
            {
                //extract all of the type101 index records
                std::string ref_chan_id = getstr(t101->ref_chan_id,8);
                std::string rem_chan_id = getstr(t101->rem_chan_id,8);
                std::string key = ref_chan_id + ":" + rem_chan_id;
                char net_sb;
                auto ch = fAllChannelMap.find(key);
                if( ch != fAllChannelMap.end() )
                {
                    auto ch_label = ch->second;
                    ch_label.Retrieve(std::string("net_sideband"), net_sb);
                    //now we want to extract the data in the type_120's
                    for(int ap=0; ap<idx->ap_space; ap++)
                    {
                        struct type_120* t120 = idx->t120[ap];
                        if(t120 != nullptr)
                        {
                            if(t120->type == SPECTRAL)
                            {
                                std::string ppkey;
                                ch_label.Retrieve(std::string("pol_product"), ppkey);
                                std::size_t pol_index = pp_index_lookup[ppkey];
                                int nlags = t120->nlags;
                                // msg_debug("mk4interface",
                                //           "Adding freq data for ap: "<<ap
                                //           <<" channel: "<< key << eom);


                                for(int j=0; j<nlags; j++)
                                {
                                    int findex = 0;
                                    int low = ch_label.GetLowerBound();
                                    int up = ch_label.GetUpperBound();
                                    findex = low+j;
                                    //TODO FIXME!!
                                    //Do we need reverse the order of the freq axis for lower-sideband data??!
                                    //If so do we need to conjugate the data as well?
                                    // if(net_sb == 'U'){findex = low+j;};
                                    // if(net_sb == 'L'){findex = up-j-1;}
                                    VFP_TYPE re = t120->ld.spec[j].re;
                                    VFP_TYPE im = t120->ld.spec[j].im;
                                    WFP_TYPE w = t120->fw.weight;
                                    std::complex<double> val(re,im);
                                    bl_data->at(pol_index, ap, findex) = val;

                                    //the last dimension for the weights isn't particulary necessary
                                    //(as all values of findex (spectral points) have the same value)
                                    bl_wdata->at(pol_index, ap, findex) = w;
                                }
                            }
                        }
                    }
                }
            }
        }//end of index loop
    }
    else
    {
        msg_error("mk4interface", "Failed to read both corel and vex file." << eom);
    }


    //grab the meta data from the type_100 and tag this data with it
    type_100* t100 = fCorel->t100;
    legacy_hops_date ldate;
    date* adate;

    // //convert the legacy date structs to a cannonical date/time-stamp string
    adate = &(t100->start);
    ldate.year = adate->year;
    ldate.day = adate->day;
    ldate.hour = adate->hour;
    ldate.minute = adate->minute;
    ldate.second = adate->second;
    std::string start_string = MHO_LegacyDateConverter::ConvertToISO8601Format(ldate);

    adate = &(t100->stop);
    ldate.year = adate->year;
    ldate.day = adate->day;
    ldate.hour = adate->hour;
    ldate.minute = adate->minute;
    ldate.second = adate->second;
    std::string stop_string = MHO_LegacyDateConverter::ConvertToISO8601Format(ldate);

    bl_data->Insert(std::string("name"), std::string("visibilities"));
    bl_data->Insert(std::string("baseline"), fBaselineName);
    bl_data->Insert(std::string("baseline_shortname"), fBaselineShortName);
    bl_data->Insert(std::string("reference_station"), fRefStation);
    bl_data->Insert(std::string("remote_station"), fRemStation);
    bl_data->Insert(std::string("reference_station_mk4id"), fRefStationMk4Id);
    bl_data->Insert(std::string("remote_station_mk4id"), fRemStationMk4Id);
    //bl_data->Insert(std::string("procdate"), procdate_string); //processing data no long used
    bl_data->Insert(std::string("start"), start_string);
    bl_data->Insert(std::string("stop"), stop_string);

    bl_wdata->Insert(std::string("name"), std::string("weights"));
    bl_wdata->Insert(std::string("baseline"), fBaselineName);
    bl_wdata->Insert(std::string("baseline_shortname"), fBaselineShortName);
    bl_wdata->Insert(std::string("reference_station"), fRefStation);
    bl_wdata->Insert(std::string("remote_station"), fRemStation);
    bl_wdata->Insert(std::string("reference_station_mk4id"), fRefStationMk4Id);
    bl_wdata->Insert(std::string("remote_station_mk4id"), fRemStationMk4Id);
    //bl_wdata->Insert(std::string("procdate"), procdate_string);
    bl_wdata->Insert(std::string("start"), start_string);
    bl_wdata->Insert(std::string("stop"), stop_string);


    fExtractedVisibilities = bl_data;
    fExtractedWeights = bl_wdata;
}




std::string
MHO_MK4CorelInterface::getstr(const char* char_array, std::size_t max_size)
{
    return std::string( char_array, std::min( strlen(char_array), max_size) );
}


bool
MHO_MK4CorelInterface::channel_info_match(double ref_sky_freq, double rem_sky_freq,
                        double ref_bw, double rem_bw,
                        std::string ref_net_sb, std::string rem_net_sb)
{
    //perhaps we ought to consider some floating point tolerance?
    if(ref_sky_freq != rem_sky_freq){return false;}
    if(ref_bw != rem_bw){return false;}
    if(ref_net_sb != rem_net_sb){return false;}
    return true;
}


double
MHO_MK4CorelInterface::calc_freq_bin(double sky_freq, double bw, std::string net_sb, int nlags, int bin_index)
{
    double step_sign = 1.0;
    if(net_sb == "U"){step_sign = 1.0;}
    if(net_sb == "L"){step_sign = -1.0;}
    double freq = sky_freq + bin_index*step_sign*(bw/nlags);
    return freq;
}

}//end of namespace
