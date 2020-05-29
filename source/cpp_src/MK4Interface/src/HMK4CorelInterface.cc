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
        meta.insert( std::string("type_000.record_id"), std::string(fCorel->id->record_id, 3) );
        meta.insert( std::string("type_000.version_no"), std::string(fCorel->id->version_no, 2) );
        meta.insert( std::string("type_000.unused1"), std::string(fCorel->id->unused1, 3) );
        meta.insert( std::string("type_000.date"), std::string(fCorel->id->date, 16) );
        meta.insert( std::string("type_000.name"), std::string(fCorel->id->name, 40) );


        std::cout<<"getting type_100 info"<<std::endl;
        meta.insert( std::string("type_100.record_id"), std::string(fCorel->t100->record_id, 3) );
        meta.insert( std::string("type_100.version_no"), std::string(fCorel->t100->version_no, 2) );
        meta.insert( std::string("type_100.unused1"), std::string(fCorel->t100->unused1, 3) );
        //meta.insert( std::string("type_100.procdate"), std::string(fCorel->t100->procdate) );
        meta.insert( std::string("type_100.baseline"), std::string(fCorel->t100->baseline, 2) );
        meta.insert( std::string("type_100.rootname"), std::string(fCorel->t100->rootname, 34) );
        meta.insert( std::string("type_100.qcode"), std::string(fCorel->t100->qcode, 2) );
        meta.insert( std::string("type_100.unused2"), std::string(fCorel->t100->unused2, 6) );
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
        for(int i=0; i < nindex; i++)
        {
            Type101Map tmp;
            //extract all of the type101 index records

            ptr = &(fCorel->index->t101[i]);
            t101 = ptr;
            //check that the record_id info is a '101'
            //if( strncmp(ptr->record_id, "101", 3) == 0)
            {
                (void) printf ("type_101 record_id = %.3s ", t101->record_id);
                (void) printf (" version_no = %.2s \n", t101->version_no);
                (void) printf (" status = %#x ", t101->status);
                (void) printf (" nblocks = %d ", (t101->nblocks));
                (void) printf (" index = %d ", (t101->index));
                (void) printf (" primary = %d \n", (t101->primary));
                (void) printf (" ref_chan_id = %.8s ", t101->ref_chan_id);
                (void) printf (" rem_chan_id = %.8s ", t101->rem_chan_id);
                (void) printf (" corr_board = %d ", (t101->corr_board));
                (void) printf (" corr_slot = %d \n", (t101->corr_slot));
                (void) printf (" ref_chan = %d ", (t101->ref_chan));
                (void) printf (" rem_chan = %d ", (t101->rem_chan));
                (void) printf (" post_mortem = %#x \n", (t101->post_mortem));
                for (i = 0; i < (t101->nblocks); i++)
                {           /* Each block */
                    (void) printf (" blocks[%d] = 0x%8.8x ", i, (t101->blocks[i]));
                }

                int buffsize = sizeof(struct type_101);
                std::cout<< std::string(ptr->record_id,3)<<std::endl;
                int n = ptr->nblocks;
                tmp.insert(std::string("nblocks"), n);
                std::cout<<"nblocks = "<<n<<std::endl;
                std::cout<<"buffsize1: "<<buffsize<<std::endl;
                buffsize += (n-1)*sizeof(int);
                std::cout<<"buffsize2: "<<buffsize<<std::endl;

                //ptr += buffsize;
                //t101 = ptr;

                std::cout<<i<<", "<<ptr<<"  buff = "<<buffsize<<std::endl;

                //type101vector.push_back(tmp);
                // tmp.insert( std::string("type_101.record_id"), std::string(fCorel->id->record_id, 3) );
                // tmp.insert( std::string("type_101.version_no"), std::string(fCorel->id->version_no, 2) );
                // tmp.insert( std::string("type_101.unused1"), std::string(fCorel->id->unused1, 3) );
                // tmp.insert( std::string("type_101.date"), std::string(fCorel->id->date, 16) );
                // tmp.insert( std::string("type_101.name"), std::string(fCorel->id->name, 40) );
            }


        }

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



        // std::vector< HMultiTypeMap< std::string, std::string, short, int, std::vector<int> > > aType101Vector;

        // size_t nblocks =
        //
        //
        //
        // if (strncmp (t1->recId, "101", 3) == 0)
        //             {           /* Type 101? */
        //             /* Index number parameters */
        //             (void) printf ("%s type_101 record_id = %.3s ", me, t101->record_id);
        //             (void) printf (" version_no = %.2s \n", t101->version_no);
        //             (void) printf (" status = %#x ", t101->status);
        //             (void) printf (" nblocks = %d ", flip_short(t101->nblocks));
        //             (void) printf (" index = %d ", flip_short(t101->index));
        //             (void) printf (" primary = %d \n", flip_short(t101->primary));
        //             (void) printf (" ref_chan_id = %.8s ", t101->ref_chan_id);
        //             (void) printf (" rem_chan_id = %.8s ", t101->rem_chan_id);
        //             (void) printf (" corr_board = %d ", flip_short(t101->corr_board));
        //             (void) printf (" corr_slot = %d \n", flip_short(t101->corr_slot));
        //             (void) printf (" ref_chan = %d ", flip_short(t101->ref_chan));
        //             (void) printf (" rem_chan = %d ", flip_short(t101->rem_chan));
        //             (void) printf (" post_mortem = %#x \n", flip_int(t101->post_mortem));
        //             for (i = 0; i < flip_short(t101->nblocks); i++)
        //                 {           /* Each block */
        //                 (void) printf (" blocks[%d] = 0x%8.8x ", i, flip_int(t101->blocks[i]));
        //                 if (i % 3 == 2)
        //                     (void) printf ("\n");
        //                 }
        //             if (flip_short(t101->nblocks) % 3 != 0)
        //                 (void) printf ("\n");
        //             continue;


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
}




}
