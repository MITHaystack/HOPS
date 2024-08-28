#include "MHO_MK4Type210Converter.hh"
#include <iostream>

const int AMPPHASE = 64;

//struct polars
//    {
//    float               ampl;                   /* Correlation coefficient */
//    float               phase;                  /* Degrees */
//    };
//
//struct type_210
//    {
//    char                record_id[3];           /* Standard 3-digit id */
//    char                version_no[2];          /* Standard 2-digit version # */
//    char                unused1[3];             /* Reserved space */
//    struct polars       amp_phas[64];           /* Residual fringe amp/phase */
//    };

namespace hops {

mho_json polarsToJSON(const polars &t) {
  return {{"ampl", t.ampl}, {"phase", t.phase}};
}

mho_json ampPhaseArrayToJSON(const polars t[AMPPHASE]) {
  int i;
  mho_json JSONArray;//[AMPPHASE];

  for (i = 0; i < AMPPHASE; i++) {
    JSONArray[i] = polarsToJSON(t[i]);
  }
  return JSONArray;
}

mho_json convertToJSON(const type_210 &t) {
  return {{"record_id", std::string(t.record_id, 3).c_str()},
          {"version_no", std::string(t.version_no, 2).c_str()},
          {"unused1", std::string(t.unused1, 3).c_str()},
          {"amp_phase", ampPhaseArrayToJSON(t.amp_phas)}};
}

} // namespace hops
