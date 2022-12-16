#include "MHO_MK4Type204Converter.hh"
#include "MHO_MK4JSONDateConverter.hh"
#include <iostream>

//typedef struct date
//    {   
//    short year;
//    short day;
//    short hour;
//    short minute;
//    float second;
//    } date_struct;

//struct type_204 
//    {
//    char                record_id[3];           /* Standard 3-digit id */
//    char                version_no[2];          /* Standard 2-digit version # */
//    char                unused1[3];             /* Reserved space */
//    short               ff_version[2];          /* Fourfit revision level */
//    char                platform[8];            /* hppa, linux, alpha etc */
//    char                control_file[96];       /* Control file full pathname */
//    struct date         ffcf_date;              /* Control file mod. date */
//    char                override[128];          /* Command line override string */
//    };

namespace hops {

json convertToJSON(const type_204 &t) {
  return {{"record_id", std::string(t.record_id, 3).c_str()},
          {"version_no", std::string(t.version_no, 2).c_str()},
          {"ff_version", t.ff_version},
          {"platform", std::string(t.platform, 8).c_str()},
          {"control_file", std::string(t.control_file, 96).c_str()},
          {"ffcf_date", convertDateToJSON(t.ffcf_date)},
          {"override", std::string(t.override, 128).c_str()}

  };
}

} // namespace hops
