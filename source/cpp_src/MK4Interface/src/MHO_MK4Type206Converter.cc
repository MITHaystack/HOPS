#include "MHO_MK4Type206Converter.hh"
#include "MHO_MK4JSONDateConverter.hh"
#include <iostream>

const int REASONSARRAYSIZE = 64;
const int NUMBEROFREASONARRAYS = 8;

//typedef struct date
//    {
//    short year;
//    short day;
//    short hour;
//    short minute;
//    float second;
//    } date_struct;

//struct sidebands
//    {
//    short               lsb;
//    short               usb;
//    };

//struct sbweights
//    {
//    double              lsb;
//    double              usb;
//    };

//struct type_206
//    {
//    char                record_id[3];           /* Standard 3-digit id */
//    char                version_no[2];          /* Standard 2-digit version # */
//    char                unused1[3];             /* Reserved space */
//    struct date         start;                  /* Time at start of AP zero */
//    short               first_ap;               /* Number of 1st valid AP */
//    short               last_ap;                /* Number of last valid AP */
//    struct sidebands    accepted[64];           /* APs accepted by chan/sband */
//    struct sbweights    weights[64];            /* Samples per channel/sideband */
//    float               intg_time;              /* Effective integration time (sec) */
//    float               accept_ratio;           /* % ratio min/max data accepted */
//    float               discard;                /* % data discarded */
//    struct sidebands    reason1[64];            /* APs filtered by chan/sband */
//    struct sidebands    reason2[64];            /* APs filtered by chan/sband */
//    struct sidebands    reason3[64];            /* APs filtered by chan/sband */
//    struct sidebands    reason4[64];            /* APs filtered by chan/sband */
//    struct sidebands    reason5[64];            /* APs filtered by chan/sband */
//    struct sidebands    reason6[64];            /* APs filtered by chan/sband */
//    struct sidebands    reason7[64];            /* APs filtered by chan/sband */
//    struct sidebands    reason8[64];            /* APs filtered by chan/sband */
//    short               ratesize;               /* Size of fringe rate transform */
//    short               mbdsize;                /* Size of MBD transform */
//    short               sbdsize;                /* Size of SBD transform */
//    char                unused2[6];             /* Padding */
//    };

namespace hops {
mho_json sidebandsToJSON(const sidebands &t) {
  return {{"lsb", t.lsb}, {"usb", t.usb}};
}

mho_json sidebandsArrayToJSON(const sidebands t[64]) {
  int i;
  mho_json JSONsidebandsArray;//[REASONSARRAYSIZE];

  for (i = 0; i < REASONSARRAYSIZE; i++) {
    JSONsidebandsArray[i] = sidebandsToJSON(t[i]);
  }
  return JSONsidebandsArray;
}

mho_json sbweightsToJSON(const sbweights &t) {
  return {{"lsb", t.lsb}, {"usb", t.usb}};
}

mho_json sbweightsArrayToJSON(const sbweights t[64]) {
  int i;
  mho_json JSONsidebandsArray;//[REASONSARRAYSIZE];

  for (i = 0; i < REASONSARRAYSIZE; i++) {
    JSONsidebandsArray[i] = sbweightsToJSON(t[i]);
  }
  return JSONsidebandsArray;
}

mho_json convertToJSON(const type_206 &t) {
  return {
      {"record_id", std::string(t.record_id, 3).c_str()},
      {"version_no", std::string(t.version_no, 2).c_str()},
      {"unused1", std::string(t.unused1, 3).c_str()},
      {"start", convertDateToJSON(t.start)},
      {"first_ap", t.first_ap},
      {"last_ap", t.last_ap},
      {"last_ap", t.last_ap},
      {"accepted", sidebandsArrayToJSON(t.accepted)},
      {"weights", sbweightsArrayToJSON(t.weights)},
      {"intg_time", t.intg_time},
      {"accept_ratio", t.accept_ratio},
      {"discard", t.discard},
      {"reason1", sidebandsArrayToJSON(t.reason1)},
      {"reason2", sidebandsArrayToJSON(t.reason2)},
      {"reason3", sidebandsArrayToJSON(t.reason3)},
      {"reason4", sidebandsArrayToJSON(t.reason4)},
      {"reason5", sidebandsArrayToJSON(t.reason5)},
      {"reason6", sidebandsArrayToJSON(t.reason6)},
      {"reason7", sidebandsArrayToJSON(t.reason7)},
      {"reason8", sidebandsArrayToJSON(t.reason8)},
      {"ratesize", t.ratesize},
      {"mbdsize", t.mbdsize},
      {"sbdsize", t.sbdsize},
      {"unused2", std::string(t.unused2, 6).c_str()},
  };
}

} // namespace hops
