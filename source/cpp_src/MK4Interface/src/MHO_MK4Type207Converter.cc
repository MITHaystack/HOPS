#include "MHO_MK4Type207Converter.hh"
#include <iostream>
#include "MHO_MK4JSONDateConverter.hh"

const int REFANDREMSIZE = 64;

//struct sbandf
//    {
//    float               lsb;
//    float               usb;
//    };
//
//struct type_207
//    {
//    char                record_id[3];           /* Standard 3-digit id */
//    char                version_no[2];          /* Standard 2-digit version # */
//    char                unused1[3];             /* Reserved space */
//    int                 pcal_mode;              /* 10*ant1+ant2, meanings in control.h */
//    int                 unused2;                /* padding */
//    struct sbandf       ref_pcamp[64];          /* Phasecal amp for ref station */
//    struct sbandf       rem_pcamp[64];          /* Phasecal amp for rem station */
//    struct sbandf       ref_pcphase[64];        /* Phasecal phase for ref station */
//    struct sbandf       rem_pcphase[64];        /* Phasecal phase for rem station */
//    struct sbandf       ref_pcoffset[64];       /* Phasecal offset for ref station */
//    struct sbandf       rem_pcoffset[64];       /* Phasecal offset for rem station */
//    struct sbandf       ref_pcfreq[64];         /* Phasecal freq for ref station */
//    struct sbandf       rem_pcfreq[64];         /* Phasecal freq for rem station */
//    float               ref_pcrate;             /* Phasecal rate for ref station */
//    float               rem_pcrate;             /* Phasecal rate for rem station */
//    float               ref_errate[64];         /* Mean error rate for ref station */
//    float               rem_errate[64];         /* Mean error rate for rem station */
//    };

namespace hops {

    json sidebandsToJSON(const sbandf &t){
      return {
        {"lsb", t.lsb},
        {"usb", t.usb}
      };
    }

    json sidebandsArrayToJSON(const sbandf t[REFANDREMSIZE]) { 
      int i;
      json JSONsidebandsArray[REFANDREMSIZE];
 
      for (i = 0; i < REFANDREMSIZE; i++){
        JSONsidebandsArray[i] = sidebandsToJSON(t[i]);
      }
      return JSONsidebandsArray;
    }

    json floatToJSON (const float t[REFANDREMSIZE]) {
      int i;
      json errate;
      for (i = 0; i < REFANDREMSIZE; i++){
        errate[i] = t[i];
      }
      return errate;

    }

    json convertToJSON(const type_207& t) {
        return {
          {"record_id", std::string(t.record_id, 3).c_str()},
          {"version_no", std::string(t.version_no, 2).c_str()},
          {"unused1", std::string(t.unused1, 2).c_str()},
          {"pcal_mode", t.pcal_mode},
          {"unused2", t.unused2},
          {"ref_pcamp", sidebandsArrayToJSON(t.ref_pcamp)},
          {"rem_pcamp", sidebandsArrayToJSON(t.rem_pcamp)},
          {"ref_pcphase", sidebandsArrayToJSON(t.ref_pcphase)},
          {"rem_pcphase", sidebandsArrayToJSON(t.rem_pcphase)},
          {"ref_pcoffset", sidebandsArrayToJSON(t.ref_pcoffset)},
          {"rem_pcoffset", sidebandsArrayToJSON(t.rem_pcoffset)},
          {"ref_pcfreq", sidebandsArrayToJSON(t.ref_pcfreq)},
          {"rem_pcfreq", sidebandsArrayToJSON(t.rem_pcfreq)},
          {"rem_pcrate", t.rem_pcrate},
          {"ref_pcrate", t.ref_pcrate},
          {"ref_errate", floatToJSON(t.ref_errate)},
          {"rem_errate", floatToJSON(t.rem_errate)}
	      };
    }
    
}
