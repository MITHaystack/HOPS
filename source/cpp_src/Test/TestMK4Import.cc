#include <iostream>
#include <string>
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

    HkMK4CorelInterface mk4ci;
    mk4ci.ReadCorelFile(corel_filename);
    struct mk4_corel* corel_obj = mk4ci.GetCorel();

    //for the time being we are just going build the visibilities for a single baseline
    std::string baseline = mk4ci.getstr(corel_obj->t100->baseline, 2) );

    std::cout<<"test"<<std::endl;
    std::cout<<"ap space"<< corel_obj->index->ap_space<<std::endl;

    struct type_101* t101 = corel_obj->index->t101;

    //see fourfit set_pointers.c for some of the mk4_corel access logic
    size_t max_ap = 0;
    size_t max_nlags = 0;
    std::set< std::string > channel_labels;

    struct mk4_corel::index_tag* idx;
    for(int i=0; i<corel_obj->index_space; i++)
    {
        idx = corel_obj->index + i;
        size_t num_aps = idx->ap_space;

        if( (t101 = idx->t101) != NULL)
        {
            //extract all of the type101 index records
            int nblocks = t101->nblocks);

            std::string ref_chan_id = getstr(t101->ref_chan_id,8) );
            std::string rem_chan_id = mk4ci.getstr(t101->rem_chan_id,8) )
            std::string channel_label = ref_chan_id + "-" + rem_chan_id;
            channel_labels.insert(channel_label);

            std::vector<int> tmp_blocks;
            for (int j = 0; j < (t101->nblocks); j++)
            {           /* Each block */
                tmp_blocks.push_back(t101->blocks[j]);
            }
            tmp101.insert( std::string("type_101.blocks"), tmp_blocks);
            type101vector.push_back(tmp101);



            //now we want to extract the data in the type_120's
            //note that this data is dumped into the type120vector in a completely disorganized fashion (as it was stored)
            //and needs to be restructured later into a sensibly ordered array
            for(int ap=0; ap<idx->ap_space; ap++)
            {
                Type120Map tmp120;
                struct type_120* t120 = idx->t120[ap];
                if(t120 != NULL)
                {
                    if(t120->type == SPECTRAL)
                    {
                        tmp120.insert(std::string("type_120.record_id"), getstr(t120->record_id, 3) );
                        tmp120.insert(std::string("type_120.version_no"), getstr(t120->version_no, 2) );
                        tmp120.insert(std::string("type_120.type"), getstr(&(t120->type), 1) );
                        tmp120.insert(std::string("type_120.nlags"), t120->nlags);
                        tmp120.insert(std::string("type_120.baseline"), getstr(t120->baseline, 2) );
                        tmp120.insert(std::string("type_120.rootcode"), getstr(t120->rootcode, 6) );
//                            // tmp120.insert(std::string("type_120.index"), t120->index );
                        tmp120.insert(std::string("type_120.index"), i );
                        tmp120.insert(std::string("type_120.ap"),t120->ap );
                        tmp120.insert(std::string("type_120.fw"), t120->fw.weight );
                        tmp120.insert(std::string("type_120.status"), t120->status);
                        tmp120.insert(std::string("type_120.fr_delay"), t120->fr_delay );
                        tmp120.insert(std::string("type_120.delay_rate"), t120->delay_rate );
                        std::vector< std::complex<double> > lag_data;
                        for(int j=0; j<t120->nlags; j++)
                        {
                            double re = t120->ld.spec[j].re;
                            double im = t120->ld.spec[j].im;
                            lag_data.push_back( std::complex<double>(re,im) );
                            //std::cout<<"("<<re<<","<<im<<")"<<std::endl;
                        }
                        tmp120.insert(std::string("type_120.ld"), lag_data);

                        if(t120->ap > max_ap){max_ap = t120->ap;}
                        if(t120->nlags > max_nlags){max_nlags = t120->nlags;}
                        type120vector.push_back(tmp120);

                    }
                    else
                    {
                        std::cout<<"non-spectral type-120 not supported."<<std::endl;
                    }
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



    size_t naps = max_ap + 1;
    size_t nlags = max_nlags;
    size_t dim[2];
    dim[0] = naps;
    dim[1] = max_nlags;
    std::map< std::string, channel_data_type* > channels;
    for(auto iter = channel_labels.begin(); iter != channel_labels.end(); iter++)
    {
        channel_data_type* chan = new channel_data_type(dim);
        channels.insert( std::pair<std::string, channel_data_type* >(*iter, chan) );
    }

    for(size_t i; i< type120vector.size(); i++)
    {
        int index120, ap;
        type120vector[i].retrieve("type_120.index", index120);
        type120vector[i].retrieve("type_120.ap", ap);

        std::string ref_chan_id;
        std::string rem_chan_id;
        std::string ref_chan;
        std::string rem_chan;

        //get the channel info
        for(size_t j=0; j<type101vector.size(); j++)
        {
            int index101;
            type101vector[j].retrieve( std::string("type_101.index"), index101 );
            if( index101 == index120 )
            {
                type101vector[j].retrieve( std::string("type_101.ref_chan_id"), ref_chan_id );
                type101vector[j].retrieve( std::string("type_101.rem_chan_id"), rem_chan_id );
                //std::cout<<"found matching type 101, index = "<<index120<<std::endl;
                //std::cout<<"ref, rem chan ids = "<<ref_chan_id<<", "<<rem_chan_id<<std::endl;
            }
        }

        std::vector< std::complex<double> > lag_data;
        type120vector[i].retrieve(std::string("type_120.ld"), lag_data);

        std::string channel_label = ref_chan_id + "-" + rem_chan_id;
        //std::cout<<"channel label = "<<channel_label<<std::endl;

        auto iter = channels.find(channel_label);
        if(iter != channels.end() )
        {
            for(size_t f = 0; f<lag_data.size(); f++)
            {
                iter->second->at(ap, f) = lag_data[f];
            }
        }

    }


    for( auto iter = channels.begin(); iter != channels.end(); iter++)
    {
        std::cout<<"test channel: "<<iter->first<<", "<<iter->second->at(0,1)<<std::endl;
    }


    return 0;
}
