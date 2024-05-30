// #include <math.h>
// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>

// #include "msg.h"
// #include "mk4_data.h"
// #include "param_struct.h"
// #include "pass_struct.h"

// extern char control_filename[];
// extern char progname[];
// extern struct mk4_fringe fringe;
// extern struct type_param param;
// extern struct type_status status;
// 
// static int stnpol[2][4] = {0, 1, 0, 1, 0, 1, 1, 0}; // [stn][pol] = 0:L, 1:R

/*
 * Rename the program name to make the output easier to parse and use
 */
// static char opname[256];
// static void save_pn(void)
// {
//     // int ln = strlen(progname);
//     // strncpy(opname, progname, sizeof(opname));
//     // strncpy(progname, "*est", ln-1);
// }
// static void rest_pn(void)
// {
//     // strcpy(progname, opname);
// }

// /* generate information about where the results came from */
// static void masthead(int mode, char *rf, struct type_pass *pass, int first_ch, int final_ch)
// {
    // msg("rf:  %s", 3, rf);
    // msg("cf:  %s", 3, control_filename);
    // msg("on: %.8s - %.8s [%c%c] fq %c pol %s ch %c..%c mode %03X", 3,
    //     fringe.t202->ref_name, fringe.t202->rem_name,
    //     fringe.t202->baseline[0], fringe.t202->baseline[1],
    //     pass->pass_data[0].fgroup, pol_string((int)pass->pol),
    //     pass->pass_data[first_ch].freq_code,
    //     pass->pass_data[final_ch].freq_code, mode);
    // msg("snr %.3f amp %.6f phs %.6f", 3,
    //     status.snr, status.delres_max, fringe.t208->resphase);
    // msg("sbd %.6f mbd %.6f frr %.6f", 3,
    //     status.sbd_max, status.mbd_max_global,
    //     status.dr_max_global * param.ref_freq);
// }



// /* a comparison routine for use by qsort() */
// static int sbd_cmp(const void *a, const void *b)
// {
//     double da = *(double*)a, db = *(double*)b;
//     if (da<db) return(-1);
//     if (da>db) return( 1);
//     return(0);
// }
// 






// 
// /*
//  * This routine adjusts delays according to the method specified in 'how':
//  *    0x02: use median channel SBD
//  *    0x04: average channel SBD
//  *    0x08: use total SBD channel
//  *    0x10: use original SBD values
//  *    0x20: use heuristics to discard outliers
//  */
// static void adj_delays(double sbd[], double esd[], double delta_delay, int first, int final, int rr, int how)
// {
//     static double cpy[MAXFREQ];
//     double tol, medly, ave, tot = status.sbd_max * 1e3;
//     int ch, med;
// 
//     /* start with a clean slate */
//     for (ch = first; ch <= final; ch++) esd[ch] = 0.0;
// 
//     /* for methods requiring a median value */
//     if ( how & 0x026 ) 
//     {
//         for (ch = first, ave=0.0; ch <= final; ch++) 
//         {
//             cpy[ch] = sbd[ch];
//             ave += sbd[ch];
//         }
//         ave /= (final - first + 1);
//         qsort(cpy + first, final - first + 1, sizeof(double), &sbd_cmp);
//         med = (first + final) / 2;
//         medly = cpy[med];
//         // msg("*est: median,average,total delays are %.3f,%.3f,%.3f",3,
//         //     medly,ave,tot);
//     }
// 
//     /* heuristic is to replace outliers with the median delay */
//     if (how & 0x20) 
//     {
//         tol = fabs(cpy[med] - tot);
//         // msg("*est: tolerance %.3f, retaining %.3f+/-%.3f",3,tol,medly,3*tol);
//         for (ch = first; tol > 0 && ch <= final; ch++)
//             if (fabs( (sbd[ch] - medly) / tol ) > 3) sbd[ch] = medly;
//         /* recompute average */
//         for (ch = first, ave=0.0; ch <= final; ch++) ave += sbd[ch];
//         ave /= (final - first + 1);
//         // msg("*est: revised average delay is %.3f",3,ave);
//     }
// 
//     if (how & 0x02) 
//     {            /* use the median value */
//         // msg("*est: using median delay (mode %x)",3,how);
//         for (ch = first; ch <= final; ch++){ esd[ch] = medly - delta_delay; }
//     }
//     else if (how & 0x04) 
//     {            /* compute and use average */
//         // msg("*est: using ave delay (mode %x)",3,how);
//         for (ch = first; ch <= final; ch++){ esd[ch] = ave - delta_delay; }
//     }
//     else if (how & 0x08) 
//     {            /* use total SBD value */
//         // msg("*est: using total SBD delay (mode %x)",3,how);
//         for (ch = first; ch <= final; ch++){ esd[ch] = tot - delta_delay; }
//     }
//     else if (how & 0x10) 
//     {            /* use the measured values */
//         // msg("*est: using measured SBD delay (mode %x)",3,how);
//         for (ch = first; ch <= final; ch++){ esd[ch] = sbd[ch] - delta_delay; }
//     }
// 
//     if (!rr) 
//     {
//         for (ch = first; ch <= final; ch++){ esd[ch] = -esd[ch]; }
//     }
// 
// }

// /*
//  *  This routine calculates adjustments to channel delays
//  *  designed to remove the sbd.  Several methods and options
//  *  are available via the 'how' argument.  See adj_delays().
//  */
// static void est_delays(struct type_pass *pass, int first, int final, int rr, int how)
// {
//     static char buf[720], tmp[80];
//     static double sbd[MAXFREQ], rdy[MAXFREQ], esd[MAXFREQ];
//     double delta_delay;
//     int ch, ss, pol, nd;
//     char *pb, *epd = getenv("HOPS_EST_PC_MDLY");
// 
//     // *progname = 0;
//     // msg("*est: delays on %s station", 1, rr ? "ref" : "rem");
//     // if (epd) msg("*est: HOPS_EST_PC_MDLY %s", 3, epd);
// 
//     /* restrict operation to only one delay calculation */
//     if ((((how & 0x02)>>1) + ((how & 0x04)>>2) +
//          ((how & 0x08)>>3) + ((how & 0x10)>>4)) > 1) 
//     {
//         // msg("*est: too many delay modes selected: 0x%02x",3,how);
//         return;
//     }
// 
//     /* consider a delay correction due to mbd */
//     if (how & 0x100) 
//     {
//         //delta_delay = (param.mbd_anchor == MODEL)
//         //            ? fringe.t208->resid_mbd
//         //            : fringe.t208->resid_mbd - fringe.t208->resid_sbd;
//         delta_delay = fringe.t208->resid_mbd;
//         delta_delay *= ((epd) ? atof(epd) : 1.0) * 1000.0;
//         // msg("*est: post-MDLY sbd adjustment %f ns", 3, delta_delay);
//     } 
//     else 
//     {
//         delta_delay = 0.0;
//     }
// 
//     /* build an array of per-channel sbd values */
//     for (ch = first; ch <= final; ch++) 
//     {
//         /* Cf. status.sbdbox[MAXFREQ] <=> status.sbd_max */
//         sbd[ch] = (status.sbdbox[ch] - param.nlags - 1) * status.sbd_sep;
//         sbd[ch] *= 1000.0;  /* us to ns */
//         if (!rr) sbd[ch] = - sbd[ch];
// 
//         /* calculate original delays */
//         rdy[ch] = (rr)
//                 ? pass->control.delay_offs[ch].ref
//                 + pass->control.delay_offs_pol[ch][stnpol[0][pass->pol]].ref
//                 : pass->control.delay_offs[ch].rem
//                 + pass->control.delay_offs_pol[ch][stnpol[1][pass->pol]].rem;
//     }
// 
//     /* make sense of it */
//     adj_delays(sbd, esd, delta_delay, first, final, rr, how);
// 
//     /* header for the section */
//     pol = pol_letter(pass->pol, !rr);
//     sprintf(buf,
//         "if station %c\n delay_offs_%c ",fringe.t202->baseline[!rr], pol);
//     for (ch = first, pb = buf + strlen(buf); ch <= final; ch++, pb++)
//         *pb = pass->pass_data[ch].freq_code;
//     *pb = 0;
//     // msg(buf, 3);
// 
//     for (buf[nd = ss = 0] = 0, ch = first; ch <= final; ch++) 
//     {
//         esd[ch] += rdy[ch];     /* work relative to input value */
//         if (fabs(esd[ch] - rdy[ch]) > 0.01) nd ++;
//         snprintf(tmp, sizeof(tmp), " %+8.3f", esd[ch]);
//         strncat(buf, tmp, sizeof(buf)-1);
// 
//         /* eight delays per line for a line length of 73 */
//         if (++ss == 8) 
//         {
//             // msg(buf, 3);
//             buf[ss = 0] = 0;
//         }
//     }
// 
//     //if (buf[0]) msg(buf, 3);
//     // msg("*est: delays %s (%d)", 2, nd ? "converging" : "converged", nd);
// }


// 
// static void est_offset(struct type_pass *pass, int rr)
// {
//     double ofs;
//     int ip = stnpol[rr][pass->pol];
//     // *progname = 0;
//     // msg("*est phs %.3f ip-%d ref %.3f rem %.3f",3,
//     //     fringe.t208->resphase, ip, pass->control.pc_phase_offset[ip].ref,
//     //     pass->control.pc_phase_offset[ip].rem);
//     ofs = (rr)
//         ? (fringe.t208->resphase   - pass->control.pc_phase_offset[ip].ref )
//         : (- fringe.t208->resphase + pass->control.pc_phase_offset[ip].rem );
//     // msg("if station %c\n pc_phase_offset_%c %+8.3f",3,
//     //     fringe.t202->baseline[!rr], pol_letter(pass->pol, !rr), ofs);
// }
