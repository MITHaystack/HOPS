#include "MHO_MK4Type200Converter.hh"
#include <iostream>
#include "MHO_MK4JSONDateConverter.hh"

// struct type_200
//     {
//     char                record_id[3];           /* Standard 3-digit id */
//     char                version_no[2];          /* Standard 2-digit version # */
//     char                unused1[3];             /* Reserved space */
//     short               software_rev[10];       /* Revision levels for online progs */
//     int                 expt_no;                /* Experiment number */
//     char                exper_name[32];         /* Observing program name */
//     char                scan_name[32];          /* Scan label from OVEX */
//     char                correlator[8];          /* Correlator identification */
//     struct date         scantime;               /* Scan time to 1 second */
//     int                 start_offset;           /* Nom. bline start rel. to scantime (s) */
//     int                 stop_offset;            /* Nom. bline stop rel. to scantime (s) */
//     struct date         corr_date;              /* Time of correlation */
//     struct date         fourfit_date;           /* Time of fourfit processing */
//     struct date         frt;                    /* Fourfit reference time */
//     };

//typedef struct date
//    {   
//    short year;
//    short day;
//    short hour;
//    short minute;
//    float second;
//    } date_struct;

namespace hops {

    json convertToJSON(const type_200& t) {
        return {
	    // logic to handle edge cases where the exper_name and scan_name are 32 chars and correlator is 8 chars
	    // this is a holdover from the previous fortran code and is an issue upstream with the c code
	    // a 32 char or 8 char array without null termination could be passed to this function and cause a memory overflow
	    {"record_id", std::string(t.record_id, 3).c_str()},
	    {"version_no", std::string(t.version_no, 2).c_str()},
	    {"software_rev", t.software_rev},
	    {"expt_no", t.expt_no},
	    {"exper_name", std::string(t.exper_name, 32).c_str()},
	    {"scan_name", std::string(t.scan_name, 32).c_str()},
	    {"correlator", std::string(t.correlator, 8).c_str()},
      {"scantime", convertDateToJSON(t.scantime)},
	    {"start_offset", t.start_offset},
	    {"stop_offset", t.stop_offset},
      {"corr_date", convertDateToJSON(t.corr_date)},
      {"fourfit_date", convertDateToJSON(t.fourfit_date)},
      {"frt", convertDateToJSON(t.frt)}

	    // the date unit of measurement requirement is currently unknown
        };
    }
}
