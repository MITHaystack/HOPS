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


// template< typename XType, size_t N>
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
MHOMK4CorelInterface::ExtractCorelFile()
{


        std::string root_filename;
        std::string corel_filename;
        bool have_file = false;

        static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                              {"root (vex) file", required_argument, 0, 'r'},
                                              {"corel file", required_argument, 0, 'f'}};

        static const char* optString = "hr:f:";

        while(true)
        {
            char optId = getopt_long(argc, argv, optString, longOptions, NULL);
            if (optId == -1)
                break;
            switch(optId)
            {
                case ('h'):  // help
                    std::cout << usage << std::endl;
                    return 0;
                case ('r'):
                    root_filename = std::string(optarg);
                    break;
                case ('f'):
                    corel_filename = std::string(optarg);
                    break;
                default:
                    std::cout << usage << std::endl;
                    return 1;
            }
        }

        MHOMK4VexInterface mk4vi;
        mk4vi.OpenVexFile(fVexFile);
        struct vex* vex_obj = mk4vi.GetVex();

        double ap_time_length = vex_obj->evex->ap_length;

        MHOMK4CorelInterface mk4ci;
        mk4ci.ReadCorelFile(corel_filename);
        struct mk4_corel* corel_obj = mk4ci.GetCorel();


        // MHOTokenizer tokenizer;
        // tokenizer.SetDelimiter(std::string("/"));
        // tokenizer.SetString(&corel_filename);
        // std::vector< std::string > tokens;
        // tokenizer.GetTokens(&tokens);
        // std::string corel_file_basename = tokens.back();
        // std::cout<<"corel file = "<<corel_file_basename<<std::endl;
        // tokenizer.SetDelimiter(std::string("."));
        // tokenizer.SetString(&corel_file_basename);
        // std::vector< std::string > tokens2;
        // tokenizer.GetTokens(&tokens2);
        // std::string baseline;
        // baseline = *(tokens2.begin());
        // std::cout<<"baseline = "<<baseline<<std::endl;

        //for the time being we are just going build the visibilities for a single baseline
        std::string baseline = mk4ci.getstr(corel_obj->t100->baseline, 2);
        std::cout<<"baseline = "<<baseline<<std::endl;

        std::cout<<"test"<<std::endl;
        std::cout<<"ap space"<< corel_obj->index->ap_space<<std::endl;

        //We need to determine 4 things:
        //(1) number of pol-products (npp)
        //(2) number of APs (nap)
        //(3) number of lags-per-channel (nlpc)
        //(4) number of channels (nc)
        //final data array has dimensions npp X nap X (nc X nlpc)

        //figure out the max number of APs and max lags, as well as the total number
        //of channels and their names
        size_t num_pp = 0;
        size_t num_aps = 0;
        size_t num_lags = 0; //not really number of lags, but rather, spectral points
        size_t num_channels = 0;
        std::set< std::pair<std::string, std::string > > channel_label_pairs;
        std::set< int > valid_aps;

        struct mk4_corel::index_tag* idx;
        for(int i=0; i<corel_obj->index_space; i++)
        {
            idx = corel_obj->index + i;
            for(int ap=0; ap<idx->ap_space; ap++)
            {
                struct type_120* t120 = idx->t120[ap];
                if(t120 != NULL)
                {
                    valid_aps.insert(ap);
                    if(t120->type == SPECTRAL)
                    {
                        int nlags = t120->nlags;
                        if(num_lags < nlags){num_lags = nlags;}
                    }
                }
            }

            struct type_101* t101;
            if( (t101 = idx->t101) != NULL)
            {
                std::string ref_chan_id = mk4ci.getstr(t101->ref_chan_id,8);
                std::string rem_chan_id = mk4ci.getstr(t101->rem_chan_id,8);
                channel_label_pairs.insert( std::pair<std::string, std::string> (ref_chan_id, rem_chan_id) );
            }
        }
        num_channels = channel_label_pairs.size();
        //we determined the max number of APs via max over valid indices, so the
        num_aps = valid_aps.size();

        //create a map of channel-pairs to interval labels (to be filled in with more information later)
        std::map< std::string, MHOIntervalLabel> channel_label_map;
        for(auto it = channel_label_pairs.begin(); it != channel_label_pairs.end(); it++)
        {
            MHOIntervalLabel tmp_label;
            tmp_label.Insert(std::string("ref_chan_id"), it->first);
            tmp_label.Insert(std::string("rem_chan_id"), it->second);
            std::string tmp_key = it->first + ":" + it->second;
            channel_label_map.insert(std::make_pair( tmp_key, tmp_label ) );
        }

        std::cout<<"number of channels = "<<num_channels<<std::endl;
        for(auto it = channel_label_pairs.begin(); it != channel_label_pairs.end(); it++)
        {
            std::cout<<"channel pairs = "<<it->first<<", "<<it->second<<std::endl;
        }

        std::set< std::string > pp_pairs;
        //now look up the station and channel info so we can fill out the frequency axis
        //also create a set of channel-labels
        int nst = vex_obj->ovex->nst;
        char ref_st = baseline[0];
        char rem_st = baseline[1];
        double ref_sky_freq, ref_bw, rem_sky_freq, rem_bw;
        char ref_net_sb, rem_net_sb, ref_pol, rem_pol;
        bool found_ref = false;
        bool found_rem = false;

        for(auto chpairIT = channel_label_pairs.begin(); chpairIT != channel_label_pairs.end(); chpairIT++)
        {
            std::string ref_chan_id = chpairIT->first;
            std::string rem_chan_id = chpairIT->second;
            found_ref = false;
            found_rem = false;
            for(int ist = 0; ist<nst; ist++)
            {
                if(ref_st == vex_obj->ovex->st[ist].mk4_site_id && !found_ref)
                {
                    //get the channel information of the reference station
                    for(size_t nch=0; nch<MAX_CHAN; nch++)
                    {
                        std::string chan_name = mk4ci.getstr( vex_obj->ovex->st[ist].channels[nch].chan_name,32);
                        if(chan_name == ref_chan_id)
                        {
                            ref_sky_freq = vex_obj->ovex->st[ist].channels[nch].sky_frequency;
                            ref_bw = vex_obj->ovex->st[ist].channels[nch].bandwidth;
                            ref_net_sb = vex_obj->ovex->st[ist].channels[nch].net_sideband;
                            ref_pol = vex_obj->ovex->st[ist].channels[nch].polarization;
                            found_ref = true;
                        }
                    }
                }

                if(rem_st == vex_obj->ovex->st[ist].mk4_site_id && !found_rem)
                {
                    for(size_t nch=0; nch<MAX_CHAN; nch++)
                    {
                        std::string chan_name = mk4ci.getstr( vex_obj->ovex->st[ist].channels[nch].chan_name,32);
                        if(chan_name == rem_chan_id)
                        {
                            rem_sky_freq = vex_obj->ovex->st[ist].channels[nch].sky_frequency;
                            rem_bw = vex_obj->ovex->st[ist].channels[nch].bandwidth;
                            rem_net_sb = vex_obj->ovex->st[ist].channels[nch].net_sideband;
                            rem_pol = vex_obj->ovex->st[ist].channels[nch].polarization;
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
                pp_pairs.insert(pp);

                //add the channel information to the labels
                std::string tmp_key = ref_chan_id + ":" + rem_chan_id;
                auto label_iter = channel_label_map.find(tmp_key);
                if(label_iter != channel_label_map.end())
                {
                    label_iter->second.Insert(std::string("ref_sky_freq"), ref_sky_freq);
                    label_iter->second.Insert(std::string("ref_bandwidth"), ref_bw);
                    label_iter->second.Insert(std::string("ref_net_sideband"), ref_net_sb);
                    label_iter->second.Insert(std::string("ref_polarization"), ref_pol);
                    label_iter->second.Insert(std::string("rem_sky_freq"), rem_sky_freq);
                    label_iter->second.Insert(std::string("rem_bandwidth"), rem_bw);
                    label_iter->second.Insert(std::string("rem_net_sideband"), rem_net_sb);
                    label_iter->second.Insert(std::string("rem_polarization"), rem_pol);

                    label_iter->second.DumpMap<std::string>();
                    label_iter->second.DumpMap<double>();
                    label_iter->second.DumpMap<char>();
                    label_iter->second.DumpMap<int>();
                }
            }
        }

        num_pp = pp_pairs.size();

        //for the time being we are implicitly assuming the frequency set-up
        //is the same for each polarization product, but we may want to make
        //this more robust against pathological cases
        //for now we just count the number of channels associated with  'XX'
        std::map< std::string, std::set< MHOIntervalLabel* > > pp_chan_set_map;
        std::map< std::string, std::vector< MHOIntervalLabel* > > pp_chan_vec_map;
        for(auto it = pp_pairs.begin(); it != pp_pairs.end(); it++)
        {
            pp_chan_set_map[*it] = std::set<MHOIntervalLabel*>();
            pp_chan_vec_map[*it] = std::vector< MHOIntervalLabel* >();
        }

        for(auto it = channel_label_map.begin(); it != channel_label_map.end(); it++)
        {
            char pol1;
            char pol2;
            it->second.Retrieve("ref_polarization", pol1);
            it->second.Retrieve("rem_polarization", pol2);
            std::string ppkey;
            ppkey.append(1,pol1);
            ppkey.append(1,pol2);
            auto indicator = pp_chan_set_map[ppkey].insert( &(it->second) );
            if(indicator.second)
            {
                pp_chan_vec_map[ppkey].push_back( &(it->second) );
            }
        }

        //make sure all pol-products share the same number of channels
        std::size_t num_channels_per_polprod = 0;
        for(auto it = pp_chan_set_map.begin(); it != pp_chan_set_map.end(); it++)
        {
            if(num_channels_per_polprod == 0)
            {
                num_channels_per_polprod = it->second.size();
            }
            else
            {
                if(num_channels_per_polprod != it->second.size() )
                {
                    std::cout<<"Warning: not all pol-products have the same number of channels!"<<std::endl;
                    std::cout<<it->first<<" : "<<it->second.size()<<std::endl;
                }
            }
        }

        std::cout<<"num channels per pol product = "<<num_channels_per_polprod<<std::endl;

        //now sort the channels by sky frequency
        chan_label_freq_predicate sort_pred;
        for(auto it = pp_pairs.begin(); it != pp_pairs.end(); it++)
        {
            std::sort( pp_chan_vec_map[*it].begin(), pp_chan_vec_map[*it].end(), sort_pred);
        }


        for(auto it = pp_pairs.begin(); it != pp_pairs.end(); it++)
        {
            for(auto it2 = pp_chan_vec_map[*it].begin(); it2 !=pp_chan_vec_map[*it].end(); it2++)
            {
                std::string chan_id;
                double sky_freq;
                (*it2)->Retrieve(std::string("ref_chan_id"),chan_id);
                (*it2)->Retrieve(std::string("ref_sky_freq"),sky_freq);
                std::cout<<chan_id<<" : "<<sky_freq<<std::endl;
            }
        }

        //now we can go ahead an create a container for all the visibilities
        std::size_t bl_dim[NDIM] = {num_pp, num_aps, (num_channels_per_polprod*num_lags)};
        baseline_data_type* bl_data = new baseline_data_type(bl_dim);

        //first label the pol-product axis
        std::size_t pp_count = 0;
        std::map<std::string, size_t> pp_index_lookup;
        for(auto it = pp_pairs.begin(); it != pp_pairs.end(); it++)
        {
            pp_index_lookup[*it] = pp_count;
            std::cout<<"pol-product @ "<<pp_count<<" = "<<*it<<std::endl;
            std::get<POLPROD_AXIS>(*bl_data)[pp_count] = *it;
            pp_count++;
        }

        //now assign values to the time (0-th dim) axis
        for(size_t ap=0; ap<num_aps; ap++)
        {
            std::get<TIME_AXIS>(*bl_data)[ap] = ap_time_length*ap;
        }

        //finally we need to label the frequency axis
        //this is trickier because the frequency axis is not continuous...
        //for the time being we assume the all pol-products have the same freq-axis
        std::size_t freq_count = 0;
        std::size_t ch_count = 0;
        for(auto it = pp_chan_vec_map[*(pp_pairs.begin())].begin(); it != pp_chan_vec_map[*(pp_pairs.begin())].end(); it++)
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
            (*it)->SetBounds(freq_count, freq_count + num_lags); //add the freq-axis bounds for this channel for later lookup

            //insert an appropriate label for this chunk of frequency data
            //set the bounds on the channel label, and add some info
            MHOIntervalLabel ch_label;
            ch_label.Insert(std::string("ch_sky_freq"), ref_sky_freq);
            ch_label.Insert(std::string("ch_bandwidth"), ref_bw);
            //add channel ID, for now this is just a number, but it could be anything
            std::stringstream ss;
            ss << ch_count;
            ch_label.Insert(std::string("channel"), ss.str());
            ch_label.SetBounds(freq_count, freq_count + num_lags);
            std::get<FREQ_AXIS>(*bl_data).InsertLabel(ch_label);

            //set up the frequency axis
            for(std::size_t sp=0; sp<num_lags; sp++)
            {
                double freq = calc_freq_bin(ref_sky_freq, rem_sky_freq, ref_bw, rem_bw, ref_net_sb, rem_net_sb, num_lags, sp);
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
        for(int i=0; i<corel_obj->index_space; i++)
        {
            idx = corel_obj->index + i;
            if( (t101 = idx->t101) != NULL)
            {
                //extract all of the type101 index records
                std::string ref_chan_id = mk4ci.getstr(t101->ref_chan_id,8);
                std::string rem_chan_id = mk4ci.getstr(t101->rem_chan_id,8);
                std::string key = ref_chan_id + ":" + rem_chan_id;
                auto ch_label_iter = channel_label_map.find(key);
                if( ch_label_iter != channel_label_map.end() )
                {
                    //now we want to extract the data in the type_120's
                    for(int ap=0; ap<idx->ap_space; ap++)
                    {
                        struct type_120* t120 = idx->t120[ap];
                        if(t120 != NULL)
                        {
                            if(t120->type == SPECTRAL)
                            {
                                std::string baseline = mk4ci.getstr(t120->baseline, 2);
                                ref_st = baseline[0];
                                rem_st = baseline[1];

                                //now look up the station and channel info so we can fill out the frequency axis
                                int nst = vex_obj->ovex->nst;
                                for(int ist = 0; ist<nst; ist++)
                                {
                                    if(ref_st == vex_obj->ovex->st[ist].mk4_site_id && !found_ref)
                                    {
                                        //get the channel information of the reference station
                                        for(size_t nch=0; nch<MAX_CHAN; nch++)
                                        {
                                            std::string chan_name = mk4ci.getstr( vex_obj->ovex->st[ist].channels[nch].chan_name,32);
                                            if(chan_name == ref_chan_id)
                                            {
                                                ref_sky_freq = vex_obj->ovex->st[ist].channels[nch].sky_frequency;
                                                ref_bw = vex_obj->ovex->st[ist].channels[nch].bandwidth;
                                                ref_net_sb = vex_obj->ovex->st[ist].channels[nch].net_sideband;
                                                ref_pol = vex_obj->ovex->st[ist].channels[nch].polarization;
                                                found_ref = true;
                                                break;
                                            }
                                        }

                                    }

                                    if(rem_st == vex_obj->ovex->st[ist].mk4_site_id && !found_rem)
                                    {
                                        for(size_t nch=0; nch<MAX_CHAN; nch++)
                                        {
                                            std::string chan_name = mk4ci.getstr( vex_obj->ovex->st[ist].channels[nch].chan_name,32);
                                            if(chan_name == rem_chan_id)
                                            {
                                                rem_sky_freq = vex_obj->ovex->st[ist].channels[nch].sky_frequency;
                                                rem_bw = vex_obj->ovex->st[ist].channels[nch].bandwidth;
                                                rem_net_sb = vex_obj->ovex->st[ist].channels[nch].net_sideband;
                                                rem_pol = vex_obj->ovex->st[ist].channels[nch].polarization;
                                                found_rem = true;
                                                break;
                                            }
                                        }

                                    }
                                    if(found_ref && found_rem){break;}
                                }

                                std::string ppkey;
                                ppkey.append(1,ref_pol);
                                ppkey.append(1,rem_pol);
                                size_t pol_index = pp_index_lookup[ppkey];

                                auto ch_it = channel_label_map.find(key);

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































































//     if(fHaveCorel)
//     {
//         //insert the type_100 meta data
//         std::cout<<"getting type_000 info"<<std::endl;
//         meta.Insert( std::string("type_000.record_id"), getstr(fCorel->id->record_id, 3) );
//         meta.Insert( std::string("type_000.version_no"), getstr(fCorel->id->version_no, 2) );
//         meta.Insert( std::string("type_000.unused1"), getstr(fCorel->id->unused1, 3) );
//         meta.Insert( std::string("type_000.date"), getstr(fCorel->id->date, 16) ); //max length 16
//         meta.Insert( std::string("type_000.name"), getstr(fCorel->id->name, 40) ); //max length 40
//
//
//         std::cout<<"getting type_100 info"<<std::endl;
//         meta.Insert( std::string("type_100.record_id"), getstr(fCorel->t100->record_id, 3) );
//         meta.Insert( std::string("type_100.version_no"), getstr(fCorel->t100->version_no, 2) );
//         meta.Insert( std::string("type_100.unused1"), getstr(fCorel->t100->unused1, 3) );
//         //meta.Insert( std::string("type_100.procdate"), std::string(fCorel->t100->procdate) );
//         meta.Insert( std::string("type_100.baseline"), getstr(fCorel->t100->baseline, 2) );
//         meta.Insert( std::string("type_100.rootname"), getstr(fCorel->t100->rootname, 34) ); //max length 34
//         meta.Insert( std::string("type_100.qcode"), getstr(fCorel->t100->qcode, 2) );
//         meta.Insert( std::string("type_100.unused2"), getstr(fCorel->t100->unused2, 6) );
//         meta.Insert( std::string("type_100.pct_done"), fCorel->t100->pct_done );
//         //meta.Insert( std::string("type_100.start"), fCorel->t100->start );
//         //meta.Insert( std::string("type_100.stop"), fCorel->t100->stop );
//         meta.Insert( std::string("type_100.ndrec"), fCorel->t100->ndrec );
//         meta.Insert( std::string("type_100.nindex"), fCorel->t100->nindex );
//         meta.Insert( std::string("type_100.nlags"), fCorel->t100->nlags );
//         meta.Insert( std::string("type_100.nblocks"), fCorel->t100->nblocks );
//
//
//         std::cout<<"dumping integer meta data"<<std::endl;
//         meta.DumpMap<int>();
//
//         std::cout<<"dumping short meta data"<<std::endl;
//         meta.DumpMap< short >();
//
//         std::cout<<"dumping float meta data"<<std::endl;
//         meta.DumpMap< float >();
//
//         std::cout<<"dumping string meta data"<<std::endl;
//         meta.DumpMap< std::string >();
//
//
//         int nalloc = fCorel->nalloc;
//         int index_space = fCorel->index_space;
//
//         std::cout<<"nalloc = "<<nalloc<<std::endl;
//         std::cout<<"index_space = "<<index_space<<std::endl;
//
//
//         int nindex = 0;
//         meta.Retrieve(std::string("type_100.nindex"), nindex );
//
//         // std::cout<<"sizeof 101 "<<sizeof(struct type_101)<<std::endl;
//         // std::vector< Type101Map > type101vector;
//         //
//         // std::cout<<"sizeof 120"<<sizeof(struct type_120)<<std::endl;
//         // std::vector< Type120Map > type120vector;
//
//         std::cout<<"test"<<std::endl;
//         std::cout<<"ap space"<< fCorel->index->ap_space<<std::endl;
//
//
//         struct type_101* ptr = fCorel->index->t101;
//         struct type_101* t101 = ptr;
//
//         //see fourfit set_pointers.c for some of the mk4_corel access logic
//         size_t max_ap = 0;
//         size_t max_nlags = 0;
//         std::set< std::string > channel_labels;
//
//         struct mk4_corel::index_tag* idx;
//         for(int i=0; i<fCorel->index_space; i++)
//         {
//             idx = fCorel->index + i;
//             if( (t101 = idx->t101) != NULL)
//             {
//                 Type101Map tmp101;
//                 //extract all of the type101 index records
//                 tmp101.Insert(std::string("type_101.record_id"), getstr(t101->record_id, 3) );
//                 tmp101.Insert(std::string("type_101.version_no"), getstr(t101->version_no, 2) );
//                 tmp101.Insert(std::string("type_101.status"), getstr(&(t101->status), 1) );
//                 tmp101.Insert(std::string("type_101.nblocks"), t101->nblocks);
//                 //tmp101.Insert(std::string("type_101.index"), t101->index);
//                 tmp101.Insert(std::string("type_101.index"), i);
//
//                 tmp101.Insert(std::string("type_101.primary"), t101->primary);
//                 tmp101.Insert(std::string("type_101.ref_chan_id"), getstr(t101->ref_chan_id,8) );
//                 tmp101.Insert(std::string("type_101.rem_chan_id"), getstr(t101->rem_chan_id,8) );
//                 tmp101.Insert(std::string("type_101.corr_board"), t101->corr_board);
//                 tmp101.Insert(std::string("type_101.corr_slot"), t101->corr_slot);
//                 tmp101.Insert(std::string("type_101.ref_chan"), t101->ref_chan );
//                 tmp101.Insert(std::string("type_101.rem_chan"), t101->rem_chan);
//                 tmp101.Insert(std::string("type_101.post_mortem"), t101->post_mortem);
//
//                 std::string ref_chan_id = getstr(t101->ref_chan_id,8);
//                 std::string rem_chan_id = getstr(t101->rem_chan_id,8);
//                 std::string channel_label = ref_chan_id + "-" + rem_chan_id;
//                 channel_labels.insert(channel_label);
//
//                 std::cout<<"t101 index @ idx "<<i<<" = "<<t101->index<<" and channel label = "<<channel_label<<std::endl;
//
//                 std::vector<int> tmp_blocks;
//                 for (int j = 0; j < (t101->nblocks); j++)
//                 {           /* Each block */
//                     tmp_blocks.push_back(t101->blocks[j]);
//                 }
//                 tmp101.Insert( std::string("type_101.blocks"), tmp_blocks);
//                 type101vector.push_back(tmp101);
//
//                 //now we want to extract the data in the type_120's
//                 //note that this data is dumped into the type120vector in a completely disorganized fashion (as it was stored)
//                 //and needs to be restructured later into a sensibly ordered array
//                 for(int ap=0; ap<idx->ap_space; ap++)
//                 {
//                     Type120Map tmp120;
//                     struct type_120* t120 = idx->t120[ap];
//                     if(t120 != NULL)
//                     {
//                         if(t120->type == SPECTRAL)
//                         {
//                             tmp120.Insert(std::string("type_120.record_id"), getstr(t120->record_id, 3) );
//                             tmp120.Insert(std::string("type_120.version_no"), getstr(t120->version_no, 2) );
//                             tmp120.Insert(std::string("type_120.type"), getstr(&(t120->type), 1) );
//                             tmp120.Insert(std::string("type_120.nlags"), t120->nlags);
//                             tmp120.Insert(std::string("type_120.baseline"), getstr(t120->baseline, 2) );
//                             tmp120.Insert(std::string("type_120.rootcode"), getstr(t120->rootcode, 6) );
//                             std::cout<<"t120 index @ ap "<<ap<<" = "<<t120->index<<std::endl;
// //                            // tmp120.Insert(std::string("type_120.index"), t120->index );
//                             tmp120.Insert(std::string("type_120.index"), i );
//                             tmp120.Insert(std::string("type_120.ap"),t120->ap );
//                             tmp120.Insert(std::string("type_120.fw"), t120->fw.weight );
//                             tmp120.Insert(std::string("type_120.status"), t120->status);
//                             tmp120.Insert(std::string("type_120.fr_delay"), t120->fr_delay );
//                             tmp120.Insert(std::string("type_120.delay_rate"), t120->delay_rate );
//                             std::vector< std::complex<double> > lag_data;
//                             for(int j=0; j<t120->nlags; j++)
//                             {
//                                 double re = t120->ld.spec[j].re;
//                                 double im = t120->ld.spec[j].im;
//                                 lag_data.push_back( std::complex<double>(re,im) );
//                                 //std::cout<<"("<<re<<","<<im<<")"<<std::endl;
//                             }
//                             tmp120.Insert(std::string("type_120.ld"), lag_data);
//
//                             if(t120->ap > max_ap){max_ap = t120->ap;}
//                             if(t120->nlags > max_nlags){max_nlags = t120->nlags;}
//                             type120vector.push_back(tmp120);
//
//                         }
//                         else
//                         {
//                             std::cout<<"non-spectral type-120 not supported."<<std::endl;
//                         }
//                     }
//                 }
//             }
//             else
//             {
//                 std::cout<<"got a different record id = "<<std::endl;//<< std::string(r->record_id,3) <<std::endl;
//             }
//
//         }//end of index loop
//
//     }//end of if MHOaveCorel

}




std::string
MHOMK4CorelInterface::getstr(const char* char_array, size_t max_size)
{
    return std::string( char_array, std::min( strlen(char_array), max_size) );
}


}
