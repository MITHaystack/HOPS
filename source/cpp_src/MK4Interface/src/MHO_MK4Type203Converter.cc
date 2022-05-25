#include "MHO_MK4Type203Converter.hh"
#include <iostream>

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
	    // logic to handle edge cases where the record_id and version_no are 32 chars and correlator is 8 chars
	    // this is a holdover from the previous fortran code and is an issue upstream with the c code
	    // a 32 char or 8 char array without null termination could be passed to this function and cause a memory overflow
	    {"record_id", std::string(t.record_id, 3).c_str()},
	    {"version_no", std::string(t.version_no, 2).c_str()},
	
	    // use a for loop to loop over the channels
	    // use a seperate function that just processes a channel struct
	    // return a json object for those channel and push in to a list
	    // stop when channel struct is no longer populated
	    // figure out what to do with garbage later
	    // (There are 512 coppies of all of the fields in the ch_struct)
	    {"channels", {
	        {"index", t.channels->index},
		{"sample_rate", t.channels->sample_rate},
		{"refsb", std::string(t.channels->refsb, 1).c_str()},
		{"remsb", std::string(t.channels->remsb, 1).c_str()},
		{"rempol", std::string(t.channels->rempol, 1).c_str()},
		{"ref_freq", t.channels->ref_freq},
		{"rem_freq", t.channels->rem_freq},
		{"ref_chan_id", std::string(t.channels->ref_chan_id, 8).c_str()},
		{"rem_chan_id", std::string(t.channels->rem_chan_id, 8).c_str()}
	    }}
        };
    }
}
