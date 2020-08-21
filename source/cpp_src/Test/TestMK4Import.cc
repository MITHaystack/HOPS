#include <iostream>
#include <string>
#include <set>
#include <utility>
#include <map>
#include <getopt.h>

#include "HkMK4VexInterface.hh"
#include "HkMK4CorelInterface.hh"

#include "HkVectorContainer.hh"
#include "HkAxisPack.hh"
#include "HkTensorContainer.hh"

using namespace hops;

using visibility_type = std::complex<double>;

using frequency_axis_type = HkVectorContainer<double>;
using time_axis_type = HkVectorContainer<double>;

using channel_axis_pack = HkAxisPack< time_axis_type, frequency_axis_type>;
using channel_data_type = HkTensorContainer< visibility_type, channel_axis_pack >;


int main(int argc, char** argv)
{
    std::string usage = "TestMK4Import -r <root_filename> -f <corel_filename>";

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

    HkMK4VexInterface mk4vi;
    mk4vi.OpenVexFile(root_filename);
    struct vex* vex_obj = mk4vi.GetVex();

    double ap_time_length = vex_obj->evex->ap_length;

    HkMK4CorelInterface mk4ci;
    mk4ci.ReadCorelFile(corel_filename);
    struct mk4_corel* corel_obj = mk4ci.GetCorel();

    //for the time being we are just going build the visibilities for a single baseline
    std::string baseline = mk4ci.getstr(corel_obj->t100->baseline, 2);

    std::cout<<"test"<<std::endl;
    std::cout<<"ap space"<< corel_obj->index->ap_space<<std::endl;



    //TODO...we need to evaluate whether not this is a feasible approach (probably not)
    //since it is entirely possible for each baseline to have a different number of APs
    //or for each channel to have a different number of lags/spectral points
    //How would we store that? A fixed-size tensor may not a be good use of space, should
    //we use/implement a ragged tensor? Or re-organize the data so that
    //'channels' are not physically separate chunks of data, but instead just 'tags'
    //which are associated with chunks of data in a larger array -- this tag mechanism
    //could be used for other things like data flagging, etc.

    //figure out the max number of APs and max lags, as well as the total number
    //of channels and their names
    size_t max_aps = 0;
    size_t max_lags = 0;
    std::set< std::pair<std::string, std::string > > channel_label_pairs;

    struct mk4_corel::index_tag* idx;
    for(int i=0; i<corel_obj->index_space; i++)
    {
        idx = corel_obj->index + i;
        int num_aps = idx->ap_space;
        if(num_aps > max_aps){max_aps = num_aps;};
        for(int ap=0; ap<idx->ap_space; ap++)
        {
            struct type_120* t120 = idx->t120[ap];
            if(t120 != NULL)
            {
                if(t120->type == SPECTRAL)
                {
                    int nlags = t120->nlags;
                    if(max_lags < nlags){max_lags = nlags;}
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

    //create a map of channel-pairs to (empty) channel_data_containers
    size_t dim[2];
    dim[0] = max_aps;
    dim[1] = max_lags;
    std::map< std::pair<std::string, std::string >, channel_data_type* > channels;
    for(auto iter = channel_label_pairs.begin(); iter != channel_label_pairs.end(); iter++)
    {
        std::cout<<"channel pair: "<<iter->first<<", "<<iter->second<<std::endl;
        auto key = std::pair<std::string, std::string >(iter->first, iter->second);
        channels.insert( std::pair< std::pair<std::string, std::string >, channel_data_type* >( key, new channel_data_type(dim) ) );
    }

    //now lets fill them up
    struct type_101* t101 = nullptr;
    for(int i=0; i<corel_obj->index_space; i++)
    {
        idx = corel_obj->index + i;
        if( (t101 = idx->t101) != NULL)
        {
            //extract all of the type101 index records
            std::string ref_chan_id = mk4ci.getstr(t101->ref_chan_id,8);
            std::string rem_chan_id = mk4ci.getstr(t101->rem_chan_id,8);
            auto key = std::pair<std::string, std::string>(ref_chan_id, rem_chan_id);

            auto channel_elem = channels.find(key);
            if( channel_elem != channels.end() )
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
                            char ref_st = baseline[0];
                            char rem_st = baseline[1];

                            double ref_sky_freq, ref_bw, rem_sky_freq, rem_bw;
                            char ref_net_sb, rem_net_sb, ref_pol, rem_pol;
                            bool found_ref = false;
                            bool found_rem = false;

                            //now look up the station and channel info so we can fill out the frequncy axis
                            int nst = vex_obj->ovex->nst;
                            for(int ist = 0; ist<nst; ist++)
                            {
                                if(ref_st == vex_obj->ovex->st[ist].mk4_site_id && !found_ref)
                                {
                                    //get the channel information of the reference station
                                    size_t nch = 0;
                                    for(size_t nch; nch<MAX_CHAN; nch++)
                                    {
                                        std::string chan_name = mk4ci.getstr( vex_obj->ovex->st[ist].channels[nch].chan_name,32);
                                        if(chan_name == key.first)
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
                                    size_t nch = 0;
                                    for(size_t nch; nch<MAX_CHAN; nch++)
                                    {
                                        std::string chan_name = mk4ci.getstr( vex_obj->ovex->st[ist].channels[nch].chan_name,32);
                                        if(chan_name == key.second)
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

                            std::cout<<"ref_chan info:"<<std::endl;
                            std::cout<<"\t ref_sky_freq = "<<ref_sky_freq<<std::endl;
                            std::cout<<"\t ref_net_sb = "<<ref_net_sb<<std::endl;
                            std::cout<<"\t ref_bw = "<<ref_bw<<std::endl;
                            std::cout<<"\t ref_pol = "<<ref_pol<<std::endl;
                            std::cout<<"rem_chan info:"<<std::endl;
                            std::cout<<"\t rem_sky_freq = "<<rem_sky_freq<<std::endl;
                            std::cout<<"\t rem_net_sb = "<<rem_net_sb<<std::endl;
                            std::cout<<"\t rem_bw = "<<rem_bw<<std::endl;
                            std::cout<<"\t rem_pol = "<<rem_pol<<std::endl;

                            // tmp120.insert(std::string("type_120.record_id"), getstr(t120->record_id, 3) );
                            // tmp120.insert(std::string("type_120.version_no"), getstr(t120->version_no, 2) );
                            // tmp120.insert(std::string("type_120.type"), getstr(&(t120->type), 1) );
                            // tmp120.insert(std::string("type_120.nlags"), t120->nlags);
                            // tmp120.insert(std::string("type_120.baseline"), getstr(t120->baseline, 2) );
                            // tmp120.insert(std::string("type_120.rootcode"), getstr(t120->rootcode, 6) );
                            //
                            //
                            // tmp120.insert(std::string("type_120.index"), i );
                            // tmp120.insert(std::string("type_120.ap"),t120->ap );
                            // tmp120.insert(std::string("type_120.fw"), t120->fw.weight );
                            // tmp120.insert(std::string("type_120.status"), t120->status);
                            // tmp120.insert(std::string("type_120.fr_delay"), t120->fr_delay );
                            // tmp120.insert(std::string("type_120.delay_rate"), t120->delay_rate );
                            //std::vector< std::complex<double> > lag_data;
                            for(int j=0; j<t120->nlags; j++)
                            {
                                double re = t120->ld.spec[j].re;
                                double im = t120->ld.spec[j].im;
                                channel_elem->second->at(ap,j) = std::complex<double>(re,im);
                            }
                        }
                        else
                        {
                            std::cout<<"non-spectral type-120 not supported."<<std::endl;
                        }
                    }

                    //now assign values to the time (0-th dim) axis
                    //std::get<0>(channel_elem.second)(ap) = ap_time_length*ap;
                }


            }

        }
        else
        {
            std::cout<<"got a different record id = "<<std::endl;//<< std::string(r->record_id,3) <<std::endl;
        }

    }//end of index loop

    // std::cout<<"dump the type 101s"<<std::endl;
    // //text dump for debug
    // for(unsigned int i=0; i<type101vector.size(); i++)
    // {
    //     type101vector[i].dump_map<std::string>();
    //     type101vector[i].dump_map<short>();
    //     type101vector[i].dump_map<int>();
    // }


    //
    // //text dump for debug
    // for(unsigned int i=0; i<type120vector.size(); i++)
    // {
    //     type120vector[i].dump_map<std::string>();
    //     type120vector[i].dump_map<short>();
    //     type120vector[i].dump_map<int>();
    //     type120vector[i].dump_map<float>();
    // }


    //
    // size_t naps = max_ap + 1;
    // size_t nlags = max_nlags;
    // size_t dim[2];
    // dim[0] = naps;
    // dim[1] = max_nlags;
    // std::map< std::string, channel_data_type* > channels;
    // for(auto iter = channel_labels.begin(); iter != channel_labels.end(); iter++)
    // {
    //     channel_data_type* chan = new channel_data_type(dim);
    //     channels.insert( std::pair<std::string, channel_data_type* >(*iter, chan) );
    // }
    //
    // for(size_t i; i< type120vector.size(); i++)
    // {
    //     int index120, ap;
    //     type120vector[i].retrieve("type_120.index", index120);
    //     type120vector[i].retrieve("type_120.ap", ap);
    //
    //     std::string ref_chan_id;
    //     std::string rem_chan_id;
    //     std::string ref_chan;
    //     std::string rem_chan;
    //
    //     //get the channel info
    //     for(size_t j=0; j<type101vector.size(); j++)
    //     {
    //         int index101;
    //         type101vector[j].retrieve( std::string("type_101.index"), index101 );
    //         if( index101 == index120 )
    //         {
    //             type101vector[j].retrieve( std::string("type_101.ref_chan_id"), ref_chan_id );
    //             type101vector[j].retrieve( std::string("type_101.rem_chan_id"), rem_chan_id );
    //             //std::cout<<"found matching type 101, index = "<<index120<<std::endl;
    //             //std::cout<<"ref, rem chan ids = "<<ref_chan_id<<", "<<rem_chan_id<<std::endl;
    //         }
    //     }
    //
    //     std::vector< std::complex<double> > lag_data;
    //     type120vector[i].retrieve(std::string("type_120.ld"), lag_data);
    //
    //     std::string channel_label = ref_chan_id + "-" + rem_chan_id;
    //     //std::cout<<"channel label = "<<channel_label<<std::endl;
    //
    //     auto iter = channels.find(channel_label);
    //     if(iter != channels.end() )
    //     {
    //         for(size_t f = 0; f<lag_data.size(); f++)
    //         {
    //             iter->second->at(ap, f) = lag_data[f];
    //         }
    //     }
    //
    // }
    //
    //
    // for( auto iter = channels.begin(); iter != channels.end(); iter++)
    // {
    //     std::cout<<"test channel: "<<iter->first<<", "<<iter->second->at(0,1)<<std::endl;
    // }


    return 0;
}
