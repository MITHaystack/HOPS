#include "MHO_EstimatePCManual.hh"

/*
 * If invoked, estimate ph phase and
 * delay values to use in manual mode.
 * pc_phase_? and delay_offs_? values report.
 *
 * This is adapted from fearfit test code. gbc 5/8/2017
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// // #include "msg.h"
// // #include "mk4_data.h"
// // #include "param_struct.h"
// // #include "pass_struct.h"
//
// // extern char control_filename[];
// // extern char progname[];
// // extern struct mk4_fringe fringe;
// // extern struct type_param param;
// // extern struct type_status status;
// //
// static int stnpol[2][4] = {0, 1, 0, 1, 0, 1, 1, 0}; // [stn][pol] = 0:L, 1:R
//
// /*
//  * Rename the program name to make the output easier to parse and use
//  */
// // static char opname[256];
// // static void save_pn(void)
// // {
// //     // int ln = strlen(progname);
// //     // strncpy(opname, progname, sizeof(opname));
// //     // strncpy(progname, "*est", ln-1);
// // }
// // static void rest_pn(void)
// // {
// //     // strcpy(progname, opname);
// // }
//
// /* some things for a library, later */
// static char *pol_string(int pol)
// {
//     char *polstr;
//     switch(pol) {
//     case 0: polstr = "LL"; break;
//     case 1: polstr = "RR"; break;
//     case 2: polstr = "LR"; break;
//     case 3: polstr = "RL"; break;
//     default: polstr = "??"; break;
//     }
//     return(polstr);
// }
// /* return the proper lower case letter */
// static int pol_letter(int pol, int rr)
// {
//     char *polstr = pol_string(pol);
//     switch (polstr[rr]) {
//     case 'L': return('l'); break;
//     case 'R': return('r'); break;
//     }
//     return('?');
// }
//
//
// /* move phase to principal branch */
// static double pbranch(double phase)
// {
//     phase = ( fmod(phase, 360.0) );
//     if (phase < -180.0){ phase += 360.0; }
//     if (phase >  180.0){ phase -= 360.0; }
//     return phase;
// }
//
//
//
//
//
//
//
// /*
//  * This routine calculates adjustments to channel phases
//  * designed to remove the multiband delay (however anchored).
//  */
// static void est_phases(struct type_pass *pass, int first, int final, int rr, int keep)
// {
//     static char buf[720], tmp[80], *pb;
//     int ch, ss, pol, nd;
//     double inp_phase, est_phase, sbmult, delta_delay, phase_bias;
//     char *epb = getenv("HOPS_EST_PC_BIAS");
//     char *epd = getenv("HOPS_EST_PC_DLYM");
//
//     // *progname = 0;
//     // msg("*est: phases on %s station", 1, rr ? "ref" : "rem");
//
//     /* support for bias operation */
//     if (keep)
//     {
//         phase_bias = (epb) ? atof(epb) : 0.0;
//         // msg("*est: phase bias %f (mod res phase is %f)", 3,
//         //     phase_bias, status.coh_avg_phase * (180.0 / M_PI));
//     }
//     // if (epb || epd)
//         // msg("*est: HOPS_EST_PC_BIAS %s ..._DLYM %s", 3, epb, epd);
//
//     /* header for the section */
//     pol = pol_letter(pass->pol, !rr);
//     sprintf(buf,
//         "if station %c\n pc_phases_%c ",fringe.t202->baseline[!rr], pol);
//     for (ch = first, pb = buf + strlen(buf); ch <= final; ch++, pb++)
//         *pb = pass->pass_data[ch].freq_code;
//     *pb = 0;
//     // msg(buf, 3);
//
//     for (buf[nd = ss = 0] = 0, ch = first; ch <= final; ch++)
//     {
//         /* assume it is all usb or lsb for this estimate */
//         sbmult = (status.total_usb_frac > 0) ? 1.0 : -1.0;
//         est_phase = status.pc_phase[ch][0][stnpol[0][pass->pol]]
//                   - status.pc_phase[ch][1][stnpol[1][pass->pol]];
//         est_phase *= 180.0 / M_PI;  /* radians to degrees */
//         inp_phase = pbranch(est_phase);
//
//         /* what we need to do to remove the multiband delay */
//         delta_delay = (param.mbd_anchor == MODEL)
//                     ? fringe.t208->resid_mbd
//                     : fringe.t208->resid_mbd - fringe.t208->resid_sbd;
//         /* allow this factor to be adjusted */
//         delta_delay *= (epd) ? atof(epd) : 1.0;
//         est_phase += sbmult * (arg_complex(status.fringe[ch]) * 180.0 / M_PI
//                   + 360.0 * delta_delay *
//                     (pass->pass_data[ch].frequency - fringe.t205->ref_freq));
//
//         /* bias the phase calculation to preserve existing resid phase */
//         if (keep) est_phase += phase_bias;
//
//         /* canonicalize for comparision */
//         est_phase = pbranch(est_phase);
//         if (fabs(inp_phase - est_phase) > 0.01) nd ++;
//
//         /* remove input phase values */
//         if (rr) inp_phase =   status.pc_phase[ch][1][stnpol[1][pass->pol]];
//         else    inp_phase = - status.pc_phase[ch][0][stnpol[0][pass->pol]];
//         est_phase += inp_phase * 180.0 / M_PI;
//
//         if (rr) est_phase = pbranch(est_phase);
//         else    est_phase = pbranch(-est_phase);
//         snprintf(tmp, sizeof(tmp), " %+8.3f", est_phase);
//         strncat(buf, tmp, sizeof(buf)-1);
//
//         /* eight phases per line for a line length of 73 */
//         if (++ss == 8)
//         {
//             // msg(buf, 3);
//             buf[ss = 0] = 0;
//         }
//     }
//     // if (buf[0]) msg(buf, 3);
//     // msg("*est: phases %s (%d)", 2, nd ? "converging" : "converged", nd);
// }
//
//
//


namespace hops
{

void
MHO_EstimatePCManual::est_pc_manual(int mode)
{
    // int first_ch, final_ch;
    // int doref, dophs, dodly, dooff, domrp;

    std::string rootfile = fParameterStore->GetAs<std::string>("/files/rootfile");

    std::string ref_id = fParameterStore->GetAs<std::string>("/ref_station/mk4id");
    std::string rem_id = fParameterStore->GetAs<std::string>("/rem_station/mk4id");

    std::string key = "/control/station/pc_mode";
    std::string default_pcmode;
    bool have_default = fParameterStore->Get(key, default_pcmode);
    if(!have_default){default_pcmode = "manual";}

    std::string ref_pcmode;
    key = "/control/station/" + ref_id + "/pc_mode";
    bool have_refmode = fParameterStore->Get(key, ref_pcmode);
    if(!have_refmode){ref_pcmode = default_pcmode;}
    std::string rem_pcmode;

    key = "/control/station/" + rem_id + "/pc_mode";
    bool have_remmode = fParameterStore->Get(key, rem_pcmode);
    if(!have_remmode){rem_pcmode = default_pcmode;}

    //this is only enabled for pc_mode manual...why? TODO FIXME
    if(ref_pcmode != "manual" || rem_pcmode != "manual"){return;}

    //determine mode logic
    bool doref = (mode>0) ? 1 : 0;
    if (!doref) mode *= -1; /* so that mode is now positive   */
    bool dophs = mode & 0x001;   /* per-channel phase correction   */
    bool dodly = mode & 0x13e;   /* 0x02 0x04 0x08 0x10 0x20 0x100 */
    bool dooff = mode & 0x040;   /* estimate phase offset value    */
    bool domrp = mode & 0x080;   /* phase bias HOPS_EST_PC_BIAS    */

    // first_ch = (param.first_plot == 0) ? 0 : param.first_plot;
    // final_ch = (param.nplot_chans == 0) ? pass->nfreq : param.nplot_chans;
    // final_ch += first_ch - 1;
    // masthead(mode, rootfile, pass, first_ch, final_ch);

    //compute pc_phases for all channels of the visibility array
    //if (dophs){ est_phases(doref, domrp); }

    // if (dodly) est_delays(pass, first_ch, final_ch, doref, dodly);
    // if (dooff) est_offset(pass, doref);
    // msg("*-----------------------------------"
    //     "------------------------------------",3);

    //rest_pn();
    // msg("done with  pc phases and delays", 1);
}



MHO_EstimatePCManual::MHO_EstimatePCManual(){};
MHO_EstimatePCManual::~MHO_EstimatePCManual(){};


bool
MHO_EstimatePCManual::ExecuteImpl(const visibility_type* in)
{


    return true;
}





}//end of namespace
