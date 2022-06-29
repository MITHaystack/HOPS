#include "MHO_MK4Type203Converter.hh"
#include <iostream>

const int numberOfChannels = 512;

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
	    // use a for loop to loop over the channels
	    // use a seperate function that just processes a channel struct
	    // return a json object for those channel and push in to a list
	    // stop when channel struct is no longer populated
	    // figure out what to do with garbage later
	    //get each channel
      // pass array of channel structs to function
      //json jsonChannels = convertChannelArrayToJSON(t);
        //pass array back to JSON library
        return {
          // logic to handle edge cases where the record_id and version_no are 32 chars and correlator is 8 chars
          // this is a holdover from the previous fortran code and is an issue upstream with the c code
          // a 32 char or 8 char array without null termination could be passed to this function and cause a memory overflow
          {"record_id", std::string(t.record_id, 3).c_str()},
          {"version_no", std::string(t.version_no, 2).c_str()},
	
          {"channels", convertChannelArrayToJSON(t) }
	      };
    }

    // 1. have a function that turns ONE channel in JSON
    // 2. map the channels in to JSON
    // 3. set field in parent JSON object to the JSON object that contains an array of channels in JSON
    json convertChannelArrayToJSON (const type_203 &t) {
      int channel;
      json JSONChannels[512];
    
	      for (channel = 0; channel < numberOfChannels; channel++) {
          //check if channel is valid (initialized or empty)
          //append to array here...
          JSONChannels[channel] = convertChannelToJSON(t, channel);

        }
        return JSONChannels;
    }

    json convertChannelToJSON (const type_203 &t, int channel) {
      return {
        {"index", t.channels[channel].index},
        {"sample_rate", t.channels[channel].sample_rate},
        {"refsb", t.channels[channel].refsb},
        {"remsb", t.channels[channel].remsb},
        {"rempol", t.channels[channel].rempol},
        {"ref_freq", t.channels[channel].ref_freq},
        {"rem_freq", t.channels[channel].rem_freq},
        {"ref_chan_id", t.channels[channel].rem_freq},
        {"rem_chan_id", t.channels[channel].rem_chan_id}

      };

    }

    //json convertChannelArrayToJSON (const type_203 &t) {
    void printChannelArray (const type_203 &t) {
      int channel;

      std::cout << "converting channels" << std::endl;
	      for (channel = 0; channel < numberOfChannels; channel++) {
            std::cout << "Channel index " << channel << ": " << t.channels[channel].index << "\n";
            std::cout << "Channel refsb " << channel << ": " << t.channels[channel].refsb << "\n";
            //get each channel
            //check if valid channel (make helper function isValid() check if first char is "" empty string or index is -1)
            //json result = convertChannelStructToJSON(t.channels[channel]);
            // pushback to array

	      }

    }
}
