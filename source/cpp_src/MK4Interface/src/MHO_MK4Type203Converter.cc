#include "MHO_MK4Type203Converter.hh"
#include "../../../c_src/applications/aedit/include/sizelimits.h"
#include <iostream>

const int NUMBEROFCHANNELS = 8*MAXFREQ;

//struct ch_struct
//    {
//    short               index;                  /* Index from type-1 file (t101) */
//    unsigned short int  sample_rate;            // Ksamp/sec (has max of 65.536 MSamp/s)
//    char                refsb;                  /* Ref ant sideband (U/L) */
//    char                remsb;                  /* Rem ant sideband (U/L) */
//    char                refpol;                 /* Ref ant polarization (R/L) */
//    char                rempol;                 /* Rem ant polarization (R/L) */
//    double              ref_freq;               /* Sky freq at ref station (MHz) */
//    double              rem_freq;               /* Sky freq at rem station (MHz) */
//    char                ref_chan_id[8];         /* Ref station channel ID */
//    char                rem_chan_id[8];         /* Rem station channel ID */
//    };

//struct type_203_v0
//    {
//    char                record_id[3];           /* Standard 3-digit id */
//    char                version_no[2];          /* Standard 2-digit version # */
//    char                unused1[3];             /* Reserved space */
//    struct ch_struct    channels[32];           /* channel-by-channel info */
//    };

//struct type_203 
//    {
//    char                record_id[3];           /* Standard 3-digit id */
//    char                version_no[2];          /* Standard 2-digit version # */
//    char                unused1[3];             /* Reserved space */
//    struct ch_struct    channels[8*MAXFREQ];    /* channel-by-channel info 512*/
//    };


namespace hops {

    json convertToJSON(const type_203& t) {
        return {
          {"record_id", std::string(t.record_id, 3).c_str()},
          {"version_no", std::string(t.version_no, 2).c_str()},
          {"channels", convertChannelArrayToJSON(t)} 
	      };
    }

    json convertChannelArrayToJSON (const type_203 &t) {
      int channel;
      json JSONChannels[NUMBEROFCHANNELS];
    
	      for (channel = 0; channel < NUMBEROFCHANNELS; channel++) {
          // Note: this does not check if channel is valid (initialized or empty)
          JSONChannels[channel] = convertChannelToJSON(t.channels[channel]);

        }
        return JSONChannels;
    }

    json convertChannelToJSON (const ch_struct &t){
      return {
        {"index", t.index},
        {"sample_rate", t.sample_rate},
        {"refsb", t.refsb},
        {"remsb", t.remsb},
        {"rempol", t.rempol},
        {"ref_freq", t.ref_freq},
        {"rem_freq", t.rem_freq},
        {"ref_chan_id", t.rem_freq},
        {"rem_chan_id", t.rem_chan_id}

      };

    }

    void printChannelArray (const type_203 &t) {
      int channel;

      std::cout << "converting channels" << std::endl;
	      for (channel = 0; channel < NUMBEROFCHANNELS; channel++) {
            std::cout << "Channel index " << channel << ": " << t.channels[channel].index << "\n";
            std::cout << "Channel refsb " << channel << ": " << t.channels[channel].refsb << "\n";
	      }

    }
}
