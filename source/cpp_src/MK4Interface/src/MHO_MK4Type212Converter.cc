#include "MHO_MK4Type212Converter.hh"
#include "MHO_MK4JSONDateConverter.hh"
#include <iostream>

//struct newphasor
//    {
//    float amp;
//    float phase;
//    float weight;                   /* Requested by L. Petrov */
//    };
//
//struct type_212
//    {
//    char            record_id[3];   /* Standard 3-digit id */
//    char            version_no[2];  /* Standard 2-digit version # */
//    char            unused;
//    short           nap;            /* Needed by IO library */
//    short           first_ap;       /* Number of first ap in record */
//    short           channel;        /* fourfit channel number */
//    short           sbd_chan;       /* Singleband delay channel */
//    char            unused2[2];
//    struct newphasor data[1];        /* data values, variable length array */
//    };

namespace hops {

mho_json newphasorToJSON(const newphasor &t) {
  return {{"amp", t.amp}, {"phase", t.phase}, {"weight", t.weight}};
}

mho_json dataToJSON(const newphasor *t, int dataSize) {
  int i;
  mho_json JSONArray;

  for (i = 0; i < dataSize; i++) {
    JSONArray.push_back(newphasorToJSON(t[i]));
  }

  return JSONArray;
}

mho_json convertToJSON(const type_212 &t, int dataSize) {
  return {{"record_id", std::string(t.record_id, 3).c_str()},
          {"version_no", std::string(t.version_no, 2).c_str()},
          {"nap", t.nap},
          {"first_ap", t.first_ap},
          {"channel", t.channel},
          {"sbd_chan", t.sbd_chan},
          {"data", dataToJSON(t.data, t.nap)}};
}

} // namespace hops
