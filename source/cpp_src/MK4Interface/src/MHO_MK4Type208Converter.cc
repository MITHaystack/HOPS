#include "MHO_MK4Type208Converter.hh"
#include <iostream>

//struct type_208 
//    {
//    char                record_id[3];           /* Standard 3-digit id */
//    char                version_no[2];          /* Standard 2-digit version # */
//    char                unused1[3];             /* Reserved space */
//    char                quality;                /* Fringe quality 0 to 9 */
//    char                errcode;                /* A to F, maybe others */
//    char                tape_qcode[6];          /* For A-file backward compat. */
//    double              adelay;                 /* Apriori delay at FRT (usec) */
//    double              arate;                  /* Apriori rate at FRT (usec/sec) */
//    double              aaccel;                 /* Apriori accel at FRT (usec/sec^2) */
//    double              tot_mbd;                /* Total observed MBD (usec) */
//    double              tot_sbd;                /* Total observed SBD (usec) */
//    double              tot_rate;               /* Total observed rate (usec/sec) */
//    double              tot_mbd_ref;            /* Total observed MBD (usec) at ref stn epoch */
//    double              tot_sbd_ref;            /* Total observed SBD (usec) at ref stn epoch */
//    double              tot_rate_ref;           /* Total observed rate (usec/sec) at ref stn epoch */
//    float               resid_mbd;              /* MBD residual to model (usec) */
//    float               resid_sbd;              /* SBD residual to model (usec) */
//    float               resid_rate;             /* Rate residual to model (usec/sec) */
//    float               mbd_error;              /* MBD error calc'd from data (usec) */
//    float               sbd_error;              /* SBD error calc'd from data (usec) */
//    float               rate_error;             /* Rate error calc'd from data (usec/sec) */
//    float               ambiguity;              /* MBD ambiguity (usec) */
//    float               amplitude;              /* Coherent amplitude (corr. coeff.) */
//    float               inc_seg_ampl;           /* Incoherent segment addition amp. */
//    float               inc_chan_ampl;          /* Incoherent channel addition amp. */
//    float               snr;                    /* SNR in sigmas */
//    float               prob_false;             /* Probability of false detection */
//    float               totphase;               /* Total observed fringe phase (deg) */
//    float               totphase_ref;           /* Total phase at ref stn epoch */
//    float               resphase;               /* Residual earth-centered phase (deg) */
//    float               tec_error;              // std dev of tec estimate (TEC units)
//    };

namespace hops {

json convertToJSON(const type_208 &t) {
  return {{"record_id", std::string(t.record_id, 3).c_str()},
          {"version_no", std::string(t.version_no, 2).c_str()},
          {"unused1", std::string(t.unused1, 2).c_str()},
          {"quality", std::string(&(t.quality), 1).c_str()},
          {"errcode", std::string(&(t.errcode), 1).c_str()},
          {"tape_qcode", std::string(t.tape_qcode, 6).c_str()},
          {"adelay", t.adelay},
          {"arate", t.arate},
          {"aaccel", t.aaccel},
          {"tot_mbd", t.tot_mbd},
          {"tot_sbd", t.tot_sbd},
          {"tot_rate", t.tot_rate},
          {"tot_mbd_ref", t.tot_mbd_ref},
          {"tot_sbd_ref", t.tot_sbd_ref},
          {"tot_rate_ref", t.tot_rate_ref},
          {"resid_mbd", t.resid_mbd},
          {"resid_sbd", t.resid_sbd},
          {"resid_rate", t.resid_rate},
          {"mbd_error", t.mbd_error},
          {"sbd_error", t.sbd_error},
          {"rate_error", t.rate_error},
          {"ambiguity", t.ambiguity},
          {"amplitude", t.amplitude},
          {"inc_seg_ampl", t.inc_seg_ampl},
          {"inc_chan_ampl", t.inc_chan_ampl},
          {"snr", t.snr},
          {"prob_false", t.prob_false},
          {"totphase", t.totphase},
          {"totphase_ref", t.totphase_ref},
          {"resphase", t.resphase},
          {"tec_error", t.tec_error}};
}

} // namespace hops
