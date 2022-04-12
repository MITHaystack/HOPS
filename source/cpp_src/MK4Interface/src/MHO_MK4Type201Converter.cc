#include "MHO_MK4Type201Converter.hh"
#include <iostream>

// the following struct is found in the hops directory in source/c_src/dfio/include/type_201.h
//struct type_201 
//    {
//    char                record_id[3];           /* Standard 3-digit id */
//    char                version_no[2];          /* Standard 2-digit version # */
//    char                unused1[3];             /* Reserved space */
//    char                source[32];             /* Source name from OVEX */
//    struct sky_coord    coord;                  /* Source coordinates */
//    short               epoch;                  /* 1950 or 2000 */
//    char                unused2[2];             /* Padding */
//    struct date         coord_date;             /* Date of coordinate meas. */
//    double              ra_rate;                /* Proper motion (rad/sec) */
//    double              dec_rate;               /* Proper motion (rad/sec) */
//    double              pulsar_phase[4];        /* Polynomial coeffs for timing */
//    double              pulsar_epoch;           /* Reference time for polynomial */
//    double              dispersion;             /* Pulsar dispersion measure */
//    };

//typedef struct sky_coord
//    {
//    short       ra_hrs;
//    short       ra_mins;
//    float       ra_secs;
//    short       dec_degs;
//    short       dec_mins;
//    float       dec_secs;
//    } sky_coord_struct;

namespace hops {

    json convertToJSON(const type_201& t) {
        return {
	    // logic to handle edge cases where the source is 32 chars 
	    // this is a holdover from the previous fortran code and is an issue upstream with the c code
	    // a 32 char array without null termination could be passed to this function and cause a memory overflow
	    {"record_id", std::string(t.record_id, 3).c_str()},
	    {"version_no", std::string(t.version_no, 2).c_str()},
	    {"source", std::string(t.source, 32).c_str()},
	    {"coord ra_hrs", t.coord.ra_hrs}, 
	    {"coord ra_mins", t.coord.ra_mins},
	    {"coord ra_secs", t.coord.ra_secs},
	    {"coord dec_degs", t.coord.dec_degs}, 
	    {"coord dec_mins", t.coord.dec_mins},
	    {"coord dec_secs", t.coord.dec_secs},
	    {"epoch", t.epoch},
	    {"pulsar_phase", t.pulsar_phase},
	    {"pulsar_epoch", t.pulsar_epoch},
	    {"dispersion", t.dispersion},

	    // The date unit of measurement requirement are currently unknown
	    {"coord_date", "null"}
        };
    }
}
