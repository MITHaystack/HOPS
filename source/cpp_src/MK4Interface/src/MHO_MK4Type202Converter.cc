#include "MHO_MK4Type200Converter.hh"
#include <iostream>

// struct type_202
//    {
//    char                record_id[3];           /* Standard 3-digit id */
//    char                version_no[2];          /* Standard 2-digit version # */
//    char                unused1[3];             /* Reserved space */
//    char                baseline[2];            /* 2-char baseline ID */
//    char                ref_intl_id[2];         /* Reference station int'l ID */
//    char                rem_intl_id[2];         /* Reference station int'l ID */
//    char                ref_name[8];            /* Reference station name */
//    char                rem_name[8];            /* Remote station name */
//    char                ref_tape[8];            /* Reference station tape VSN */
//    char                rem_tape[8];            /* Remote station tape VSN */
//    short               nlags;                  /* # lags used for correlation */
//    double              ref_xpos;               /* Station X-coord (meters) */
//    double              rem_xpos;               /* Station X-coord (meters) */
//    double              ref_ypos;               /* Station Y-coord (meters) */
//    double              rem_ypos;               /* Station Y-coord (meters) */
//    double              ref_zpos;               /* Station Z-coord (meters) */
//    double              rem_zpos;               /* Station Z-coord (meters) */
//    double              u;                      /* Fringes/arcsec E-W 1GHz */
//    double              v;                      /* Fringes/arcsec N-S 1GHz */
//    double              uf;                     /* mHz/arcsec/GHz in R.A. */
//    double              vf;                     /* mHz/arcsec/GHz in dec. */
//    float               ref_clock;              /* Ref station clock (usec) */
//    float               rem_clock;              /* Rem station clock (usec) */
//    float               ref_clockrate;          /* Ref clockrate (sec/sec) */
//    float               rem_clockrate;          /* Rem clockrate (sec/sec) */
//    float               ref_idelay;             /* Ref station instr. delay (usec) */
//    float               rem_idelay;             /* Rem station instr. delay (usec) */
//    float               ref_zdelay;             /* Ref station z.atm. delay (usec) */
//    float               rem_zdelay;             /* Rem station z.atm. delay (usec) */
//    float               ref_elev;               /* Elevation at ref. antenna (deg) */
//    float               rem_elev;               /* Elevation at rem. antenna (deg) */
//    float               ref_az;                 /* Azimuth at ref. antenna (deg) */
//    float               rem_az;                 /* Azimuth at rem. antenna (deg) */
//    };

namespace hops {

    json convertToJSON(const type_202& t) {
        return {
	    {"record_id", t.record_id},
	    {"version_no", t.version_no},
	    // logic to handle edge cases where the exper_name and scan_name are 32 chars and correlator is 8 chars
	    // this is a holdover from the previous fortran code and is an issue upstream with the c code
	    // a 32 char or 8 char array without null termination could be passed to this function and cause a memory overflow
	    {"baseline", std::string(t.baseline, 2).c_str()},
	    {"ref_intl_id", std::string(t.ref_intl_id, 2).c_str()},
	    {"rem_intl_id", std::string(t.rem_intl_id, 2).c_str()},
	    {"ref_name", std::string(t.ref_name, 8).c_str()},
	    {"rem_name", std::string(t.rem_name, 8).c_str()},
	    {"ref_tape", std::string(t.ref_tape, 8).c_str()},
	    {"rem_tape", std::string(t.rem_tape, 8).c_str()},
	    {"nlags", t.nlags},
	    {"ref_xpos", t.ref_xpos},
	    {"rem_xpos", t.rem_xpos},
	    {"ref_zpos", t.ref_zpos},
	    {"rem_zpos", t.rem_zpos},
	    {"rem_ypos", t.rem_ypos},
	    {"rem_zpos", t.rem_zpos},
	    {"ref_clock", t.ref_clock},
	    {"rem_clock", t.rem_clock},
	    {"ref_clockrate", t.ref_clockrate},
	    {"rem_clockrate", t.rem_clockrate},
	    {"ref_idelay", t.ref_idelay},
	    {"rem_idelay", t.rem_idelay},
	    {"ref_zdelay", t.ref_zdelay},
	    {"rem_zdelay", t.rem_zdelay},
	    {"ref_elev", t.ref_elev},
	    {"rem_elev", t.rem_elev},
	    {"ref_az", t.ref_az},
	    {"rem_az", t.rem_az},
	    {"u", t.u},
	    {"v", t.v},
	    {"uf", t.uf},
	    {"vf", t.vf},

        };
    }
}
