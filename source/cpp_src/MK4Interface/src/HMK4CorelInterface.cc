#include "HMK4CorelInterface.hh"

#include "HMultiTypeMap.hh"

#include <array>
#include <vector>
#include <cstring>

namespace hops
{


typedef HMultiTypeMap< std::string, std::string, short, int, std::vector<int> > Type101Map;


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


HMK4CorelInterface::HMK4CorelInterface():
    fHaveCorel(false)
{
    fCorel = (struct mk4_corel *) calloc ( 1, sizeof(struct mk4_corel) );

}

HMK4CorelInterface::~HMK4CorelInterface()
{
    clear_mk4corel(fCorel);
    free(fCorel);
}

void
HMK4CorelInterface::ReadCorelFile(const std::string& filename)
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
HMK4CorelInterface::ExportCorelFile()
{
    if(fHaveCorel)
    {
        //for now just do the POD data types

        HMultiTypeMap< std::string, std::string, int, short, float, double > meta;

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

        std::cout<<"sizeof 101 "<<sizeof(struct type_101)<<std::endl;
        std::vector< Type101Map > type101vector;

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
                Type101Map tmp;
                //extract all of the type101 index records
                tmp.insert(std::string("type_101.record_id"), getstr(t101->record_id, 3) );
                tmp.insert(std::string("type_101.version_no"), getstr(t101->version_no, 2) );
                tmp.insert(std::string("type_101.status"), getstr(t101->version_no, 1) );
                tmp.insert(std::string("type_101.nblocks"), t101->nblocks);
                tmp.insert(std::string("type_101.index"), t101->index);
                tmp.insert(std::string("type_101.primary"), t101->primary);
                tmp.insert(std::string("type_101.ref_chan_id"), getstr(t101->ref_chan_id,8) );
                tmp.insert(std::string("type_101.rem_chan_id"), getstr(t101->rem_chan_id,8) );
                tmp.insert(std::string("type_101.corr_board"), t101->corr_board);
                tmp.insert(std::string("type_101.corr_slot"), t101->corr_slot);
                tmp.insert(std::string("type_101.ref_chan"), t101->ref_chan );
                tmp.insert(std::string("type_101.rem_chan"), t101->rem_chan);
                tmp.insert(std::string("type_101.post_mortem"), t101->post_mortem);
                std::vector<int> tmp_blocks;
                for (int j = 0; j < (t101->nblocks); j++)
                {           /* Each block */
                    tmp_blocks.push_back(t101->blocks[j]);
                }
                tmp.insert( std::string("type_101.blocks"), tmp_blocks);
                type101vector.push_back(tmp);

                }
                else
                {
                    std::cout<<"got a different record id = "<<std::endl;//<< std::string(r->record_id,3) <<std::endl;
                }
            }

            for(unsigned int i=0; i<type101vector.size(); i++)
            {
                type101vector[i].dump_map<std::string>();
                type101vector[i].dump_map<short>();
                type101vector[i].dump_map<int>();
            }






        }//end of index space loop

        //
        //
        // /* ** Type-120 record? ** */
        //         else if (strncmp (t1->recId, "120", 3) == 0)
        //             {
        //             /* * Yes.  Calculate the length of the additional read * */
        //             /* Special case variable-length record */
        //             nbuff = sizeof (struct type_120) - sizeof (union lag_data) - k;
        //             nlags = flip_short(t120->nlags);
        //             /* (We've already read enough of a 120 to see type and nlags) */
        //             if (t120->type == COUNTS_PER_LAG)   /* 1 */
        //                 nbuff += nlags * sizeof (struct counts_per_lag);
        //             else if (t120->type == COUNTS_GLOBAL)   /* 2 */
        //                 nbuff += sizeof (struct counts_global) +
        //                 (nlags - 1) * sizeof (struct lag_tag);
        //             else if (t120->type == AUTO_GLOBAL) /* 3 */
        //                 nbuff += sizeof (struct auto_global) + (nlags - 1) * sizeof (int);
        //             else if (t120->type == AUTO_PER_LAG)    /* 4 */
        //                 nbuff += nlags * sizeof (struct auto_per_lag);
        //             else if (t120->type == SPECTRAL)    /* 5 */
        //                 nbuff += nlags * sizeof (struct spectral);
        //             else
        //                 {       /* No other type-120 types */
        //                 (void) fprintf (stderr,
        //                     "%s%s type-120 %d at %dB?\n",
        //                     me, ERRMSG, t120->type, tlast);
        //                 return (-20);   /* Error */
        //                 }
        //             countt[3]++;        /* Increment record count */
        //             }
        //
        //
        // /* * Type-120 record? * */
        //         if (strncmp (t1->recId, "120", 3) == 0)
        //             {           /* Type 120? */
        //             /* Sorted lag data */
        //             (void) printf ("%s got type 120 \n", me);
        //             (void) printf (" record_id = %.3s ", t120->record_id);
        //             (void) printf ("version_no = %.2s ", t120->version_no);
        //             (void) printf ("type = %d \n", t120->type);
        //             nlags = flip_short(t120->nlags);
        //             (void) printf (" nlags = %d ", nlags);
        //             (void) printf ("baseline = %.2s ", t120->baseline);
        //             (void) printf ("rootcode = %.6s ", t120->rootcode);
        //             (void) printf ("index = %d ", flip_int(t120->index));
        //             (void) printf ("ap = %d \n", flip_int(t120->ap));
        //             (void) printf (" weight = %8.4f ", flip_float(t120->fw.weight));
        //             (void) printf ("status = %#8.8x ", flip_int(t120->status));
        //             /* (void) printf("bitshift = %f ", t120->bitshift); */
        //             (void) printf ("fr_delay = %d ", flip_int(t120->fr_delay));
        //             /* (void) printf("fbit = %f \n", t120->fbit); */
        //             (void) printf ("delay_rate = %d \n", flip_int(t120->delay_rate));
        //             if (t120->type == COUNTS_GLOBAL)
        //                 {
        //                 (void) printf (" cg.cosbits = 0x%8.8x ", flip_int(pcg->cosbits));
        //                 (void) printf ("cg.sinbits = 0x%8.8x \n", flip_int(pcg->sinbits));
        //                 }
        //             else if (t120->type == AUTO_GLOBAL)
        //                 (void) printf (" ag.cosbits = 0x%8.8x \n", flip_int(pag->cosbits));
        //             for (j = 0; j < nlags; j++)
        //                 {           /* Through union lag_data */
        //                 (void) printf (" j =%3d ", j);
        //                 if (t120->type == COUNTS_PER_LAG)
        //                     {
        //                     (void) printf (" coscor = 0x%8.8x %9d ",
        //                                flip_int(pcpl[j].coscor), flip_int(pcpl[j].coscor));
        //                     (void) printf (" cosbits = 0x%8.8x %9d \n",
        //                                flip_int(pcpl[j].cosbits), flip_int(pcpl[j].cosbits));
        //                     (void) printf ("         sincor = 0x%8.8x %9d ",
        //                                flip_int(pcpl[j].sincor), flip_int(pcpl[j].sincor));
        //                     (void) printf (" sinbits = 0x%8.8x %9d \n",
        //                                flip_int(pcpl[j].sinbits), flip_int(pcpl[j].sinbits));
        //                     }
        //                 else if (t120->type == COUNTS_GLOBAL)
        //                     {
        //                     (void) printf (" lags[].coscor = 0x%8.8x %9d ",
        //                                flip_int(pcg->lags[j].coscor), flip_int(pcg->lags[j].coscor));
        //                     (void) printf (" lags[].sincor = 0x%8.8x %9d \n",
        //                                flip_int(pcg->lags[j].sincor), flip_int(pcg->lags[j].sincor));
        //                     }
        //                 else if (t120->type == AUTO_PER_LAG)
        //                     {
        //                     (void) printf (" cosbits = 0x%8.8x %9d ",
        //                                flip_int(papl[j].cosbits), flip_int(papl[j].cosbits));
        //                     (void) printf (" coscor = 0x%8.8x %9d \n",
        //                                flip_int(papl[j].coscor), flip_int(papl[j].coscor));
        //                     }
        //                 else if (t120->type == AUTO_GLOBAL)
        //                     (void) printf (" coscor[] = 0x%8.8x %9d \n",
        //                        flip_int(pag->coscor[j]), flip_int(pag->coscor[j]));
        //                 else if (t120->type == SPECTRAL)
        //                     {
        //                     (void) printf (" real %9.6f  imag %9.6f\n",
        //                                flip_float (psp[j].re), flip_float (psp[j].im));
        //                     }
        //                 }           /* End of for j lags */
        //             continue;
        //             }           /* End of if type 120 */
        //



        // struct type_101
        //     {
        //     char         record_id[3];          /* Standard 3-digit id */
        //     char         version_no[2];         /* Standard 2-digit version # */
        //     char         status;                /* Reserved space */
        //     short        nblocks;               /* Needed up front for IO library */
        //     short        index;                 /* Index number */
        //     short        primary;               /* Index number of primary 101 */
        //     char         ref_chan_id[8];        /* Ref station channel id */
        //     char         rem_chan_id[8];        /* Rem station channel id */
        //     short        corr_board;            /* Correlator board serial # */
        //     short        corr_slot;             /* Correlator board slot */
        //     short        ref_chan;              /* Ref station SU channel number */
        //     short        rem_chan;              /* Rem station SU channel number */
        //     int          post_mortem;           /* 32 1-bit flags */
        //     int          blocks[1];             /* One entry per block in snake */
        //     };



        // struct mk4_corel
        //     {
        //     void *allocated[MAXIND + 4];                /* Ignore type 120 recs */
        //     int nalloc;
        //     char *file_image;
        //     struct type_000 *id;
        //     struct type_100 *t100;
        //     int index_space;
        //     struct index_tag
        //         {
        //         struct type_101 *t101;
        //         int ap_space;
        //         struct type_120 **t120;
        //         } *index;
        //     };


        // struct type_000
        //     {
        //     char                record_id[3];           /* Standard 3-digit id */
        //     char                version_no[2];          /* Standard 2-digit version # */
        //     char                unused1[3];             /* Reserved space */
        //     char                date[16];               /* Creation date " yyyyddd-hhmmss " */
        //     char                name[40];               /* exp/scan/name, null-terminated */
        //     };

        // struct type_100
        //     {
        //     char         record_id[3];          /* Standard 3-digit id */
        //     char         version_no[2];         /* Standard 2-digit version # */
        //     char         unused1[3];            /* Reserved space */
        //     struct date  procdate;              /* Correlation time */
        //     char         baseline[2];           /* Standard baseline id */
        //     char         rootname[34];          /* Root filename, null-terminated */
        //     char         qcode[2];              /*         meta.insert( std::string("type_100.pct_done"), fCorel->t100->pct_done );Quality code of correlation */
        //     char         unused2[6];            /* Padding */
        //     float        pct_done;              /* 0-100% of scheduled data processed */
        //     struct date  start;                 /* Time of first AP */
        //     struct date  stop;                  /* Time of last AP */
        //     int          ndrec;                 /* Number of data records */
        //     int          nindex;                /* Number of index numbers present */
        //     short        nlags;                 /* # of lags in a type_120 record */

        //     short        nblocks;               /* # blocks per index number */
        //     };

        // struct type_101
        //     {
        //     char         record_id[3];          /* Standard 3-digit id */
        //     char         version_no[2];         /* Standard 2-digit version # */
        //     char         status;                /* Reserved space */
        //     short        nblocks;               /* Needed up front for IO library */
        //     short        index;                 /* Index number */
        //     short        primary;               /* Index number of primary 101 */
        //     char         ref_chan_id[8];        /* Ref station channel id */
        //     char         rem_chan_id[8];        /* Rem station channel id */
        //     short        corr_board;            /* Correlator board serial # */
        //     short        corr_slot;             /* Correlator board slot */
        //     short        ref_chan;              /* Ref station SU channel number */
        //     short        rem_chan;              /* Rem station SU channel number */
        //     int          post_mortem;           /* 32 1-bit flags */
        //     int          blocks[1];             /* One entry per block in snake */
        //     };

        // struct type_120
        //     {
        //     char            record_id[3];   /* Standard 3-digit id                    */
        //     char            version_no[2];  /* Standard 2-digit version #             */
        //     char            type;           /* Data type (defines above)              */
        //     short           nlags;          /* Needed by IO library                   */
        //     char            baseline[2];    /* Standard baseline id                   */
        //     char            rootcode[6];    /* Root suffix                            */
        //     int             index;          /* Index number for type 101 rec.         */
        //     int             ap;             /* Acc period number                      */
        //     union flag_wgt  fw;             // either flag or weight for lag or spectral
        //     int             status;         /* Up to 32 status bits                   */
        //     int             fr_delay;       /* Mid-AP fractional delay (bits * 2^32)  */
        //     int             delay_rate;     /* Mid-AP delay rate (bits/sysclk * 2^32) */
        //     union lag_data  ld;             /* Correlation counts                     */
        //     };
        //
        //
        // union lag_data
        //     {
        //     struct counts_per_lag cpl[1];
        //     struct counts_global  cg;
        //     struct auto_per_lag  apl[1];
        //     struct auto_global  ag;
        //     struct spectral spec[1];
        //     };
        //
        // union flag_wgt
        //     {
        //     int             flag;           /* Up to 32 correlation flags             */
        //     float           weight;         // in spectral mode: ap weight (0.0-1.0)
        //     };

}




std::string
HMK4CorelInterface::getstr(const char* char_array, size_t max_size)
{
    return std::string( char_array, std::min( strlen(char_array), max_size) );
}




}
