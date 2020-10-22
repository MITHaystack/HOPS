#include "MHOMK4CorelInterface.hh"

#include <vector>
#include <cstdlib>
#include <cstring>
#include <complex>
#include <set>
#include <algorithm>

//mk4 IO library
extern "C"
{
    #include "mk4_records.h"
    #include "mk4_data.h"
    #include "mk4_dfio.h"
    #include "mk4_vex.h"
}


namespace hops
{

//the ordering operator for channel labels;
class chan_label_freq_predicate
{
    public:
        chan_label_freq_predicate(){};
        virtual ~chan_label_freq_predicate(){};

    virtual bool operator()(const MHOIntervalLabel* a, const MHOIntervalLabel* b)
    {
        double a_frq, b_frq;
        a->Retrieve(std::string("ref_sky_freq"), a_frq);
        b->Retrieve(std::string("ref_sky_freq"), b_frq);
        return a_frq < b_frq;
    }
};


double
MHOMK4CorelInterface::calc_freq_bin(double ref_sky_freq, double rem_sky_freq, double ref_bw, double rem_bw, char ref_net_sb, char rem_net_sb, int nlags, int bin_index)
{
    double step_sign = 1.0;
    if(ref_net_sb == 'U' && rem_net_sb == 'U')
    {
        step_sign = 1.0;
    }
    else if(ref_net_sb == 'L' && rem_net_sb == 'L')
    {
        step_sign = -1.0;
    }
    else
    {
        std::cout<<"mixed side bands?!"<<std::endl;
    }

    //mixed U/L?

    double freq = 0;
    if( ref_sky_freq == rem_sky_freq && ref_bw == rem_bw)
    {
        freq = ref_sky_freq + bin_index*step_sign*(ref_bw/nlags);
    }
    else
    {
        std::cout<<"ref-rem channel infor mis-match?!"<<std::endl;
    }

    return freq;

}




// template< typename XType, std::size_t N>
// std::array<XType, N> create_and_fill_array(XType values[N])
// {
//     std::array<XType, N> arr;
//     for(size_t i=0; i<N; i++)
//     {
//         arr[i] = values[i];
//     }
//     return arr;
// }


MHOMK4CorelInterface::MHOMK4CorelInterface():
    fHaveCorel(false),
    fHaveVex(false),
    fCorel(nullptr),
    fVex(nullptr)
{
    fVex = (struct vex *) calloc ( 1, sizeof(struct vex) );
    fCorel = (struct mk4_corel *) calloc ( 1, sizeof(struct mk4_corel) );
}

MHOMK4CorelInterface::~MHOMK4CorelInterface()
{
    clear_mk4corel(fCorel);
    free(fCorel);
    free(fVex);
}

void
MHOMK4CorelInterface::ReadCorelFile()
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
    if(retval == 0)
    {
        fHaveCorel = true;
        msg_debug("mk4interface", "Failed to read corel file: "<< fCorelFile << ", error value: "<< retval << eom);
    }
    else
    {
        fHaveCorel = false;
        msg_debug("mk4interface", "Successfully read corel file."<< fCorelFile << eom);
    }
}


void
MHOMK4CorelInterface::ReadVexFile()
{
    if(fHaveVex)
    {
        msg_debug("mk4interface", "Clearing a previously exisiting vex struct."<< eom);
        free(fVex);
        fVex = (struct vex *) calloc ( 1, sizeof(struct vex) );
        fHaveVex = false;
    }

    std::string tmp_key(""); //use empty key for now
    std::string fname = fVexFile;
    int retval = get_vex( const_cast<char*>(fname.c_str() ),  OVEX | EVEX | IVEX | LVEX , const_cast<char*>(tmp_key.c_str() ), fVex);

    if(retval !=0 )
    {
        fHaveVex = false;
        msg_debug("mk4interface", "Failed to read vex file: " << fVexFile << ", error value: "<< retval << eom);
    }
    else
    {
        //do something with the vex object
        fHaveVex = true;
        msg_debug("mk4interface", "Successfully read vex file."<< fVexFile << eom);
    }

}





void
MHOMK4CorelInterface::DetermineDataDimensions()
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
    std::set< int > valid_aps;

    std::string baseline = getstr(fCorel->t100->baseline, 2);
    msg_debug("mk4interface", "Reading data for baseline: " << baseline << eom);

    struct mk4_corel::index_tag* idx;
    for(int i=0; i<fCorel->index_space; i++)
    {
        idx = fCorel->index + i;
        for(int ap=0; ap<idx->ap_space; ap++)
        {
            struct type_120* t120 = idx->t120[ap];
            if(t120 != NULL)
            {
                if(t120->type == SPECTRAL)
                {
                    valid_aps.insert(ap);
                    std::size_t nlags = t120->nlags;
                    if(fNSpectral < nlags){fNSpectral = nlags;}
                }
            }
        }

        struct type_101* t101;
        if( (t101 = idx->t101) != NULL)
        {
            std::string ref_chan_id = getstr(t101->ref_chan_id,8);
            std::string rem_chan_id = getstr(t101->rem_chan_id,8);
            MHOIntervalLabel tmp_label;
            tmp_label.Insert(std::string("ref_chan_id"), ref_chan_id);
            tmp_label.Insert(std::string("rem_chan_id"), rem_chan_id);
            std::string tmp_key = ref_chan_id + ":" + rem_chan_id;
            fAllChannelMap[tmp_key] = tmp_label;
        }
    }
    fNChannels = fAllChannelMap.size();
    fNAPs = valid_aps.size();

    #ifdef  HOPS_ENABLE_DEBUG_MSG
    msg_debug("mk4interface", "Total number of channels = "<< fNChannels << eom );
    for(auto it = fAllChannelMap.begin(); it != fAllChannelMap.end(); it++)
    {
        msg_debug("mk4interface", " channel pair = "<<it->first<<", "<<it->second<< eom);
    }
    #endif

    //now we need to fill in the channel labels with information from the vex
    int nst = fVex->ovex->nst;
    char ref_st = baseline[0];
    char rem_st = baseline[1];
    double ref_sky_freq, ref_bw, rem_sky_freq, rem_bw;
    char ref_net_sb, rem_net_sb, ref_pol, rem_pol;
    bool found_ref = false;
    bool found_rem = false;
    fPolProducts.clear(); //for counting pol-products

    for(auto ch = fAllChannelMap.begin(); ch != fAllChannelMap.end(); ch++)
    {
        std::string ref_chan_id, rem_chan_id;
        ch->second->Retrieve(std::string("ref_chan_id"), ref_chan_id);
        ch->second->Retrieve(std::string("rem_chan_id"), rem_chan_id);
        found_ref = false;
        found_rem = false;
        for(int ist = 0; ist<nst; ist++)
        {
            if(ref_st == fVex->ovex->st[ist].mk4_site_id && !found_ref)
            {
                //get the channel information of the reference station
                for(size_t nch=0; nch<MAX_CHAN; nch++)
                {
                    std::string chan_name = getstr( fVex->ovex->st[ist].channels[nch].chan_name,32);
                    if(chan_name == ref_chan_id)
                    {
                        ref_sky_freq = fVex->ovex->st[ist].channels[nch].sky_frequency;
                        ref_bw = fVex->ovex->st[ist].channels[nch].bandwidth;
                        ref_net_sb = fVex->ovex->st[ist].channels[nch].net_sideband;
                        ref_pol = fVex->ovex->st[ist].channels[nch].polarization;
                        found_ref = true;
                    }
                }
            }

            if(rem_st == fVex->ovex->st[ist].mk4_site_id && !found_rem)
            {
                for(size_t nch=0; nch<MAX_CHAN; nch++)
                {
                    std::string chan_name = getstr( fVex->ovex->st[ist].channels[nch].chan_name,32);
                    if(chan_name == rem_chan_id)
                    {
                        rem_sky_freq = fVex->ovex->st[ist].channels[nch].sky_frequency;
                        rem_bw = fVex->ovex->st[ist].channels[nch].bandwidth;
                        rem_net_sb = fVex->ovex->st[ist].channels[nch].net_sideband;
                        rem_pol = fVex->ovex->st[ist].channels[nch].polarization;
                        found_rem = true;
                    }
                }
            }
        }

        if(found_ref && found_rem)
        {
            std::string pp;
            pp.append(1,ref_pol);
            pp.append(1,rem_pol);
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

            #ifdef HOPS_ENABLE_DEBUG_MSG
            msg_debug("mk4interface", "Dumping channel information for: " << tmp_key << eom);
            ch->second.DumpMap<std::string>();
            ch->second.DumpMap<double>();
            ch->second.DumpMap<char>();
            ch->second.DumpMap<int>();
            #endif
        }
    }

    fNPPs = fPolProducts.size();
    //for the time being we are implicitly assuming the frequency set-up
    //but we count the number of channels per pol-product to make sure
    std::map< std::string, std::set< MHOIntervalLabel* > > pp_chan_set_map;

    //initialize with empty set/vectors for each pol-product
    for(auto it = fPolProducts.begin(); it != fPolProducts.end(); it++)
    {
        pp_chan_set_map[*it] = std::set<MHOIntervalLabel*>();
        fAllChannelInfo[*it] = std::vector< MHOIntervalLabel >();
    }
    //now sort channel labels by pol-product
    for(auto it = fAllChannelMap.begin(); it != fAllChannelMap.end(); it++)
    {
        char pol1, pol2;
        it->second.Retrieve("ref_polarization", pol1);
        it->second.Retrieve("rem_polarization", pol2);
        std::string ppkey;
        ppkey.append(1,pol1);
        ppkey.append(1,pol2);
        auto indicator = pp_chan_set_map[ppkey].insert( &(it->second) );
        if(indicator.second)
        {
            fAllChannelInfo[ppkey].push_back( it->second );
        }
    }

    //make sure all pol-products share the same number of channels
    std::size_t fNChannelsPerPP = 0;
    for(auto it = pp_chan_set_map.begin(); it != pp_chan_set_map.end(); it++)
    {
        if(fNChannelsPerPP == 0){fNChannelsPerPP = it->second.size();}
        else
        {
            if(fNChannelsPerPP != it->second.size() )
            {
                msg_warning("mk4interface",
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
        std::sort( fAllChannelInfo[*it].begin(), fAllChannelInfo[*it].end(), sort_pred);
    }

}




baseline_data_type*
MHOMK4CorelInterface::ExtractCorelFile()
{
    ReadCorelFile();
    ReadVexFile();

    if(fHaveCorel && fHaveVex)
    {
        DetermineDataDimensions();
        double ap_time_length = fVex->evex->ap_length;

        //now we can go ahead an create a container for all the visibilities
        std::size_t bl_dim[NDIM] = {fNPPs, fNAPs, (fNChannelsPerPP*fNSpectral)};
        baseline_data_type* bl_data = new baseline_data_type(bl_dim);

        //first label the pol-product axis
        std::size_t pp_count = 0;
        std::map<std::string, std::size_t> pp_index_lookup;
        for(auto it = fPolProducts.begin(); it != fPolProducts.end(); it++)
        {
            pp_index_lookup[*it] = pp_count;
            std::get<POLPROD_AXIS>(*bl_data)[pp_count] = *it;
            pp_count++;
        }

        //now assign values to the time (0-th dim) axis
        for(size_t ap=0; ap<fNAPs; ap++)
        {
            std::get<TIME_AXIS>(*bl_data)[ap] = ap_time_length*ap;
        }

        //finally we need to label the frequency axis
        //this is trickier because the frequency axis is not continuous...
        //for the time being we assume the all pol-products have the same freq-axis
        std::size_t freq_count = 0;
        std::size_t ch_count = 0;
        double ref_sky_freq, ref_bw, rem_sky_freq, rem_bw;
        char ref_net_sb, rem_net_sb, ref_pol, rem_pol;
        for(auto it = fAllChannelInfo[*(fPolProducts.begin())].begin(); it != fAllChannelInfo[*(fPolProducts.begin())].end(); it++)
        {
            // double ref_sky_freq, ref_bw, rem_sky_freq, rem_bw;
            // char ref_net_sb, rem_net_sb, ref_pol, rem_pol;
            (*it)->Retrieve(std::string("ref_sky_freq"), ref_sky_freq);
            (*it)->Retrieve(std::string("ref_bandwidth"), ref_bw);
            (*it)->Retrieve(std::string("ref_net_sideband"), ref_net_sb);
            (*it)->Retrieve(std::string("ref_polarization"), ref_pol);
            (*it)->Retrieve(std::string("rem_sky_freq"), rem_sky_freq);
            (*it)->Retrieve(std::string("rem_bandwidth"), rem_bw);
            (*it)->Retrieve(std::string("rem_net_sideband"), rem_net_sb);
            (*it)->Retrieve(std::string("rem_polarization"), rem_pol);
            (*it)->SetBounds(freq_count, freq_count + fNSpectral); //add the freq-axis bounds for this channel for later lookup

            //insert an appropriate label for this chunk of frequency data
            //set the bounds on the channel label, and add some info
            MHOIntervalLabel ch_label;
            ch_label.Insert(std::string("ch_sky_freq"), ref_sky_freq);
            ch_label.Insert(std::string("ch_bandwidth"), ref_bw);
            //add channel ID, for now this is just a number, but it could be anything
            std::stringstream ss;
            ss << ch_count;
            ch_label.Insert(std::string("channel"), ss.str());
            ch_label.SetBounds(freq_count, freq_count + fNSpectral);
            std::get<FREQ_AXIS>(*bl_data).InsertLabel(ch_label);

            //set up the frequency axis
            for(std::size_t sp=0; sp<fNSpectral; sp++)
            {
                double freq = calc_freq_bin(ref_sky_freq, rem_sky_freq, ref_bw, rem_bw, ref_net_sb, rem_net_sb, fNSpectral, sp);
                std::get<FREQ_AXIS>(*bl_data)(freq_count) = freq;
                freq_count++;
            }

            ch_count++;
        }


        //lets print out the pol, time and freq axes now:
        for(std::size_t i=0; i< std::get<POLPROD_AXIS>(*bl_data).GetSize(); i++)
        {
            std::cout<<"pol_axis: "<<i<<" = "<<std::get<POLPROD_AXIS>(*bl_data).at(i)<<std::endl;
        }

        for(std::size_t i=0; i< std::get<TIME_AXIS>(*bl_data).GetSize(); i++)
        {
            std::cout<<"time_axis: "<<i<<" = "<<std::get<TIME_AXIS>(*bl_data).at(i)<<std::endl;
        }

        for(std::size_t i=0; i< std::get<FREQ_AXIS>(*bl_data).GetSize(); i++)
        {
            std::cout<<"freq_axis: "<<i<<" = "<<std::get<FREQ_AXIS>(*bl_data).at(i)<<std::endl;
        }


        //now fill in the actual visibility data
        struct type_101* t101 = nullptr;
        for(int i=0; i<fCorel->index_space; i++)
        {
            idx = fCorel->index + i;
            if( (t101 = idx->t101) != NULL)
            {
                //extract all of the type101 index records
                std::string ref_chan_id = getstr(t101->ref_chan_id,8);
                std::string rem_chan_id = getstr(t101->rem_chan_id,8);
                std::string key = ref_chan_id + ":" + rem_chan_id;
                auto ch = fAllChannelMap.find(key);
                if( ch != fAllChannelMap.end() )
                {
                    //now we want to extract the data in the type_120's
                    for(int ap=0; ap<idx->ap_space; ap++)
                    {
                        struct type_120* t120 = idx->t120[ap];
                        if(t120 != NULL)
                        {
                            if(t120->type == SPECTRAL)
                            {
                                std::string ppkey;
                                ch->second.Retrieve(std::string("pol_product"), pp);
                                std::size_t pol_index = pp_index_lookup[ppkey];

                                auto ch_it = fAllChannelMap.find(key);

                                int nlags = t120->nlags;
                                std::cout<<"adding freq data for ap: "<<ap<<" channels: "<<ref_chan_id<<" "<<rem_chan_id<<std::endl;
                                for(int j=0; j<nlags; j++)
                                {
                                    int low = ch_it->second.GetLowerBound();
                                    double re = t120->ld.spec[j].re;
                                    double im = t120->ld.spec[j].im;
                                    std::cout<<ppkey<<" "<<pol_index<<" "<<ap<<" "<<j<<" "<<re<<","<<im<<std::endl;
                                    bl_data->at(pol_index, ap, low+j) =  std::complex<double>(re,im);
                                }
                            }
                            else
                            {
                                std::cout<<"non-spectral type-120 not supported."<<std::endl;
                            }
                        }
                    }
                }
            }

        }//end of index loop

    }













































}




std::string
MHOMK4CorelInterface::getstr(const char* char_array, std::size_t max_size)
{
    return std::string( char_array, std::min( strlen(char_array), max_size) );
}


}
