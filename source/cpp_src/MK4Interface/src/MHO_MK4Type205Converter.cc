#include "MHO_MK4Type205Converter.hh"
#include <iostream>
#include "MHO_MK4JSONDateConverter.hh"

const int NUMBEROFFFITCHAN = 16; //capitalize this later

//struct type_205_v0
//    {
//    char                record_id[3];           /* Standard 3-digit id */
//    char                version_no[2];          /* Standard 2-digit version # */
//    char                unused1[3];             /* Reserved space */
//    struct date         utc_central;            /* Central time of scan */
//    float               offset;                 /* Offset of FRT from scan ctr sec */
//    char                ffmode[8];              /* Fourfit execution modes */
//    float               search[6];              /* SBD, MBD, rate search windows */
//                                                /* (usec, usec, usec/sec) */
//    float               filter[8];              /* Various filter thresholds */
//    struct date         start;                  /* Start of requested data span */
//    struct date         stop;                   /* End of requested data span */
//    double              ref_freq;               /* Fourfit reference frequency Hz */
//    struct
//        {
//        char            ffit_chan_id;           /* Fourfit channel letter id */
//        char            unused;                 /* Alignment padding */
//        short           channels[4];            /* Indices into type 203 */
//        } ffit_chan[16];                        /* Fourfit channel id info */
//    };

//typedef struct date
//    {   
//    short year;
//    short day;
//    short hour;
//    short minute;
//    float second;
//    } date_struct;


namespace hops {

    json convertToJSON(const type_205& t) {
       int ffitChannel;
       json JSONFfitChannels[NUMBEROFFFITCHAN]; 

       // because ffit_can is an unnamed struct, we can't pass it to functions easily so the following had to be done
       for (ffitChannel = 0; ffitChannel < NUMBEROFFFITCHAN; ffitChannel++){
          JSONFfitChannels[ffitChannel] = {{"unused", std::string(&(t.ffit_chan[ffitChannel].unused), 1).c_str()}, 
                              {"ffit_chan_id", std::string(&(t.ffit_chan[ffitChannel].ffit_chan_id), 1).c_str()},      
                              {"channels", t.ffit_chan[ffitChannel].channels}
                            };
       }
        return {
          {"record_id", std::string(t.record_id, 3).c_str()},
          {"version_no", std::string(t.version_no, 2).c_str()},
          {"unused1", std::string(t.unused1, 2).c_str()},
          {"utc_central", convertDateToJSON(t.utc_central)},
          {"offset", t.offset},
          {"ffmode", std::string(t.ffmode, 8).c_str()},
          {"search", t.search},
          {"filter", t.filter},
          {"start", convertDateToJSON(t.start)},
          {"stop", convertDateToJSON(t.stop)},
          {"ref_freq", t.ref_freq},
          {"ffit_chan", JSONFfitChannels}
	      };
    }

}

