#include "HkMK4CorelInterface.hh"

#include "HkMultiTypeMap.hh"

#include <array>
#include <vector>
#include <cstring>
#include <complex>

namespace hops
{




template< typename XType, size_t N>
std::array<XType, N> create_and_fill_array(XType values[N])
{
    std::array<XType, N> arr;
    for(size_t i=0; i<N; i++)
    {
        arr[i] = values[i];
    }
    return arr;
}


HkMK4CorelInterface::HkMK4CorelInterface():
    fHaveCorel(false)
{
    fCorel = (struct mk4_corel *) calloc ( 1, sizeof(struct mk4_corel) );

}

HkMK4CorelInterface::~HkMK4CorelInterface()
{
    clear_mk4corel(fCorel);
    free(fCorel);
}

void
HkMK4CorelInterface::ReadCorelFile(const std::string& filename)
{
    std::cout<<"reading corel file"<<std::endl;
    if(fHaveCorel)
    {
        std::cout<<"clearing corel struct"<<std::endl;
        clear_mk4corel(fCorel);
    }

    //have to copy filename for const_cast, as mk4 lib doesn't respect const
    std::string fname = filename;
    std::cout<<"calling read_mk4corel: on: "<<fname<<std::endl;
    int retval = read_mk4corel( const_cast<char*>(fname.c_str()), fCorel );
    if(retval == 0)
    {
        fHaveCorel = true;
        std::cout<<"success"<<std::endl;
    }
    else
    {
        fHaveCorel = false;
        std::cout<<"failure"<<std::endl;
    }

    std::cout<<"done read"<<std::endl;
}


void
HkMK4CorelInterface::ExportCorelFile(Type100MetaData& meta, std::vector< Type101Map >& type101vector, std::vector< Type120Map >& type120vector)
{
    if(fHaveCorel)
    {
        //insert the type_100 meta data
        std::cout<<"getting type_000 info"<<std::endl;
        meta.insert( std::string("type_000.record_id"), getstr(fCorel->id->record_id, 3) );
        meta.insert( std::string("type_000.version_no"), getstr(fCorel->id->version_no, 2) );
        meta.insert( std::string("type_000.unused1"), getstr(fCorel->id->unused1, 3) );
        meta.insert( std::string("type_000.date"), getstr(fCorel->id->date, 16) ); //max length 16
        meta.insert( std::string("type_000.name"), getstr(fCorel->id->name, 40) ); //max length 40


        std::cout<<"getting type_100 info"<<std::endl;
        meta.insert( std::string("type_100.record_id"), getstr(fCorel->t100->record_id, 3) );
        meta.insert( std::string("type_100.version_no"), getstr(fCorel->t100->version_no, 2) );
        meta.insert( std::string("type_100.unused1"), getstr(fCorel->t100->unused1, 3) );
        //meta.insert( std::string("type_100.procdate"), std::string(fCorel->t100->procdate) );
        meta.insert( std::string("type_100.baseline"), getstr(fCorel->t100->baseline, 2) );
        meta.insert( std::string("type_100.rootname"), getstr(fCorel->t100->rootname, 34) ); //max length 34
        meta.insert( std::string("type_100.qcode"), getstr(fCorel->t100->qcode, 2) );
        meta.insert( std::string("type_100.unused2"), getstr(fCorel->t100->unused2, 6) );
        meta.insert( std::string("type_100.pct_done"), fCorel->t100->pct_done );
        //meta.insert( std::string("type_100.start"), fCorel->t100->start );
        //meta.insert( std::string("type_100.stop"), fCorel->t100->stop );
        meta.insert( std::string("type_100.ndrec"), fCorel->t100->ndrec );
        meta.insert( std::string("type_100.nindex"), fCorel->t100->nindex );
        meta.insert( std::string("type_100.nlags"), fCorel->t100->nlags );
        meta.insert( std::string("type_100.nblocks"), fCorel->t100->nblocks );


        std::cout<<"dumping integer meta data"<<std::endl;
        meta.dump_map<int>();

        std::cout<<"dumping short meta data"<<std::endl;
        meta.dump_map< short >();

        std::cout<<"dumping float meta data"<<std::endl;
        meta.dump_map< float >();

        std::cout<<"dumping string meta data"<<std::endl;
        meta.dump_map< std::string >();


        int nalloc = fCorel->nalloc;
        int index_space = fCorel->index_space;

        std::cout<<"nalloc = "<<nalloc<<std::endl;
        std::cout<<"index_space = "<<index_space<<std::endl;


        int nindex = 0;
        meta.retrieve(std::string("type_100.nindex"), nindex );

        // std::cout<<"sizeof 101 "<<sizeof(struct type_101)<<std::endl;
        // std::vector< Type101Map > type101vector;
        //
        // std::cout<<"sizeof 120"<<sizeof(struct type_120)<<std::endl;
        // std::vector< Type120Map > type120vector;

        std::cout<<"test"<<std::endl;
        std::cout<<"ap space"<< fCorel->index->ap_space<<std::endl;


        struct type_101* ptr = fCorel->index->t101;
        struct type_101* t101 = ptr;

        //see fourfit set_pointers.c for some of the mk4_corel access logic
        struct mk4_corel::index_tag* idx;
        for(int i=0; i<fCorel->index_space; i++)
        {
            idx = fCorel->index + i;
            if( (t101 = idx->t101) != NULL)
            {
                Type101Map tmp101;
                //extract all of the type101 index records
                tmp101.insert(std::string("type_101.record_id"), getstr(t101->record_id, 3) );
                tmp101.insert(std::string("type_101.version_no"), getstr(t101->version_no, 2) );
                tmp101.insert(std::string("type_101.status"), getstr(&(t101->status), 1) );
                tmp101.insert(std::string("type_101.nblocks"), t101->nblocks);
                tmp101.insert(std::string("type_101.index"), t101->index);
                tmp101.insert(std::string("type_101.primary"), t101->primary);
                tmp101.insert(std::string("type_101.ref_chan_id"), getstr(t101->ref_chan_id,8) );
                tmp101.insert(std::string("type_101.rem_chan_id"), getstr(t101->rem_chan_id,8) );
                tmp101.insert(std::string("type_101.corr_board"), t101->corr_board);
                tmp101.insert(std::string("type_101.corr_slot"), t101->corr_slot);
                tmp101.insert(std::string("type_101.ref_chan"), t101->ref_chan );
                tmp101.insert(std::string("type_101.rem_chan"), t101->rem_chan);
                tmp101.insert(std::string("type_101.post_mortem"), t101->post_mortem);
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
                            tmp120.insert(std::string("type_120.index"), t120->index );
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
                                std::cout<<"("<<re<<","<<im<<")"<<std::endl;
                            }
                            tmp120.insert(std::string("type_120.ld"), lag_data);



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

        std::cout<<"dump the type 101s"<<std::endl;


        //text dump for debug
        for(unsigned int i=0; i<type101vector.size(); i++)
        {
            type101vector[i].dump_map<std::string>();
            type101vector[i].dump_map<short>();
            type101vector[i].dump_map<int>();
        }

        std::cout<<"dump the type 120s"<<std::endl;

        //text dump for debug
        for(unsigned int i=0; i<type120vector.size(); i++)
        {
            type120vector[i].dump_map<std::string>();
            type120vector[i].dump_map<short>();
            type120vector[i].dump_map<int>();
            type120vector[i].dump_map<float>();
        }

    }//end of if HkaveCorel

}




std::string
HkMK4CorelInterface::getstr(const char* char_array, size_t max_size)
{
    return std::string( char_array, std::min( strlen(char_array), max_size) );
}


}
