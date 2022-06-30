#include "MHO_MK4Type204Converter.hh"
#include <iostream>

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

    json convertToJSON(const type_204& t) {
        return {
          {"record_id", std::string(t.record_id, 3).c_str()},
          {"version_no", std::string(t.version_no, 2).c_str()},
          {"ff_version", t.ff_version},
          {"platform", std::string(t.platform, 8).c_str()},
          {"control_file", std::string(t.control_file, 96).c_str()},

          // the date unit of measurement requirent is currently uknown
          // ffcf_date data
          {"year", nlohmann::detail::value_t::null},
          {"day", nlohmann::detail::value_t::null},
          {"hour", nlohmann::detail::value_t::null},
          {"minute", nlohmann::detail::value_t::null},
          {"second", nlohmann::detail::value_t::null},

          {"overrid", std::string(t.override, 128).c_str()}
	
	      };
    }

}
