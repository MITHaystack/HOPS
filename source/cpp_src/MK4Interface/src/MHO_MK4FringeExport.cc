#include "MHO_MK4FringeExport.hh"

#include "MHO_MultiTypeMap.hh"
#include <array>

namespace hops
{


MHO_MK4FringeExport::MHO_MK4FringeExport()
{

}

MHO_MK4FringeExport::~MHO_MK4FringeExport()
{

}


void
MHO_MK4FringeExport::ExportFringeFile()
{

}


int MHO_MK4FringeExport::fill_200( struct type_200 *t200)
{
    clear_200(t200);

    // extern struct mk4_corel cdata; 
    // time_t tm;
    // struct tm *utc_now, *gmtime(const time_t*);
    // int int_reftime, year, day, hour, minute, second;
    // 
    // clear_200 (t200);
    // 
    // #ifdef HAVE_CONFIG_H
    // t200->software_rev[0] = HOPS_SVN_REV;
    // #endif
    // t200->expt_no = root->exper_num;
    // strcpy (t200->exper_name, root->exper_name);
    // strcpy (t200->scan_name, root->scan_name);
    // strncpy (t200->correlator, root->correlator, 8);
    // memcpy (&(t200->scantime), &(root->start_time), sizeof (struct date));
    // t200->start_offset = param->start_offset;
    // t200->stop_offset = param->stop_offset;
    //                                     /* Extract correlation date */
    //                                     /* from type-1 file in memory */
    // if (sscanf (cdata.id->date, "%4d%3d-%2d%2d%2d", 
    //                 &year, &day, &hour, &minute, &second) != 5)
    //     //msg ("Warning: unable to get correlation date from type-1 file", 2);
    // else
    //     {
    //     t200->corr_date.year = year;
    //     t200->corr_date.day = day;
    //     t200->corr_date.hour = hour;
    //     t200->corr_date.minute = minute;
    //     t200->corr_date.second = second;
    //     }
    //                                     /* Get current time (redundant with */
    //                                     /* time in id record, no harm done) */
    // tm = time (NULL);
    // utc_now = gmtime (&tm);
    // t200->fourfit_date.year = utc_now->tm_year + 1900;
    // t200->fourfit_date.day = utc_now->tm_yday + 1;
    // t200->fourfit_date.hour = utc_now->tm_hour;
    // t200->fourfit_date.minute = utc_now->tm_min % 100;
    // t200->fourfit_date.second = utc_now->tm_sec % 100;
    //                                     /* Convert fourfit reference time to */
    //                                     /* standard date format */
    // t200->frt.year = t200->scantime.year;
    // t200->frt.second = fmod ((double)param->reftime,  60.0);
    // int_reftime = param->reftime;       /* In seconds */
    // int_reftime /= 60;                  /* Now in minutes */
    // t200->frt.minute = int_reftime % 60;
    // int_reftime /= 60;                  /* Now in hours */
    // t200->frt.hour = int_reftime % 24;
    // t200->frt.day = int_reftime / 24 + 1; /* days start with 001 */
    // 
    return 0;
}

int MHO_MK4FringeExport::fill_201( struct type_201 *t201)
{
    clear_201(t201);

    // clear_201 (t201);
    // 
    // strncpy (t201->source, root->src.source_name, 32);
    // memcpy (&(t201->coord), &(root->src.position), sizeof (struct sky_coord));
    // if (root->src.position_ref_frame == J2000) t201->epoch = 2000;
    // else if (root->src.position_ref_frame == B1950) t201->epoch = 1950;
    // memcpy (&(t201->coord_date), &(root->src.position_epoch), 
    //                                      sizeof (struct date));
    // t201->ra_rate = root->src.ra_rate;
    // t201->dec_rate = root->src.dec_rate;
    // 
    //                                     // NOTE!!!! Differential TEC is accidentally
    //                                     // in the sense of reference_tec - remote_tec
    //                                     // This differs from all other diff. quantities
    // t201->dispersion = param->ion_diff;
    //                                     /* Ignore pulsar parameters for now */
    return 0;

}

int MHO_MK4FringeExport::fill_202( struct type_202 *t202)
{
    clear_202(t202);

    // char refst, remst;
    // int i;
    // double refepoch, remepoch, frt, refdiff, remdiff, lambda;
    // extern double time_to_double(struct date);
    // struct station_struct *ref, *rem;
    // struct station_log *lref, *lrem;
    // struct date tempdate;
    // extern struct mk4_sdata sdata[];
    // struct mk4_sdata *refsd, 
    //                  *remsd;
    // clear_202 (t202);
    // 
    // strncpy (t202->baseline, param->baseline, 2);
    //                                     /* Get station structs from root */
    // refst = param->baseline[0];
    // remst = param->baseline[1];
    // ref = rem = NULL;
    // for (i=0; i<root->ovex->nst; i++)
    //     {
    //     if (root->ovex->st[i].mk4_site_id == refst) 
    //         ref = root->ovex->st + i;
    //     if (root->ovex->st[i].mk4_site_id == remst) 
    //         rem = root->ovex->st + i;
    //     }
    // if ((ref == NULL) || (rem == NULL))
    //     {
    //     if (ref == NULL)
    //         //msg ("Failed to find station '%c' in ovex file", 2, refst);
    //     else
    //         //msg ("Failed to find station '%c' in ovex file", 2, remst);
    //     return (-1);
    //     }
    // strncpy (t202->ref_intl_id, ref->site_id, 2);
    // strncpy (t202->rem_intl_id, rem->site_id, 2);
    // strncpy (t202->ref_name, ref->site_name, 8);
    // strncpy (t202->rem_name, rem->site_name, 8);
    // t202->nlags = param->nlags;
    // t202->ref_xpos = ref->coordinates[0];
    // t202->rem_xpos = rem->coordinates[0];
    // t202->ref_ypos = ref->coordinates[1];
    // t202->rem_ypos = rem->coordinates[1];
    // t202->ref_zpos = ref->coordinates[2];
    // t202->rem_zpos = rem->coordinates[2];
    //                                     /* Fourfit ref time is relative to start of year */
    //                                     /* So need to convert to secs since 1980 */
    // tempdate.year = root->ovex->start_time.year;
    // tempdate.day = 1;
    // tempdate.hour = 0;
    // tempdate.minute = 0;
    // tempdate.second = 0.0;
    // frt = time_to_double (tempdate) + param->reftime;
    //                                     /* Ref station clockrate ref time */
    // if (ref->clockrate != 0.0)
    //     refepoch = time_to_double (ref->clockrate_epoch);
    // else refepoch = frt;
    // refdiff = frt - refepoch;
    // if (fabs (refdiff) > 3.0e5)
    //     //msg ("Warning, ref station clockrate epoch highly discrepant from FRT\n"
    //          "frt = %12.2f, ref epoch = %12.2f", 1, frt, refepoch);
    //                                     /* Rem station clockrate ref time */
    // if (rem->clockrate != 0.0)
    //     remepoch = time_to_double (rem->clockrate_epoch);
    // else remepoch = frt;
    // remdiff = frt - remepoch;
    // if (fabs (remdiff) > 3.0e5)
    //     //msg ("Warning, rem station clockrate epoch highly discrepant from FRT\n"
    //          "frt = %12.2f, ref epoch = %12.2f", 1, frt, remepoch);
    //                                     /* Adjust clocks to frt for clockrate */
    // t202->ref_clock = (ref->clock_early + (refdiff * ref->clockrate)) * 1.0e6;
    // t202->rem_clock = (rem->clock_early + (remdiff * rem->clockrate)) * 1.0e6;
    // t202->ref_clockrate = ref->clockrate;
    // t202->rem_clockrate = rem->clockrate;
    //                                     /* Instrumental delays(?) here */
    // t202->ref_zdelay = ref->zenith_atm * 1.0e6;
    // t202->rem_zdelay = rem->zenith_atm * 1.0e6;
    //                                     /* elevations/azimuths here */
    // for (i=0; i<MAXSTATIONS; i++)       // find correct splines
    //     if (sdata[i].t300 != NULL)
    //         {
    //         if (sdata[i].t300->id == refst)
    //             refsd = sdata + i;
    //         if (sdata[i].t300->id == remst) 
    //             remsd = sdata + i;
    //         }
    // if ((refsd == NULL) || (remsd == NULL))
    //     {
    //     //msg ("Could not find stations in t303 records in fill_202()", 2);
    //     return (-1);
    //     }
    // 
    //                                     // should actually evaluate these polys at frt
    // if (refsd->model[0].t303[0] != NULL && remsd->model[0].t303[0] != NULL)
    //     {
    //     t202->ref_elev = refsd->model[0].t303[0]->elevation[0];
    //     t202->rem_elev = remsd->model[0].t303[0]->elevation[0];
    //     t202->ref_az   = refsd->model[0].t303[0]->azimuth[0];
    //     t202->rem_az   = remsd->model[0].t303[0]->azimuth[0];
    //                                     // Baseline u,v in fr / asec
    //                                     // should evaluate these polys at frt, too!
    //     lambda = 299.792458 / param->ref_freq; // wavelength (m)
    //     t202->u = 4.848137e-6 * (remsd->model[0].t303[0]->u[0] 
    //                            - refsd->model[0].t303[0]->u[0]) / lambda;
    //     t202->v = 4.848137e-6 * (remsd->model[0].t303[0]->v[0] 
    //                            - refsd->model[0].t303[0]->v[0]) / lambda;
    //     }
    //                                     /* Now find stations in lvex for VSNs */
    // for (i=0; i<root->lvex->nstation; i++)
    //     {
    //     if (root->lvex->stn[i].station == refst)
    //         strncpy (t202->ref_tape, root->lvex->stn[i].vsn, 8);
    //     if (root->lvex->stn[i].station == remst)
    //         strncpy (t202->rem_tape, root->lvex->stn[i].vsn, 8);
    //     }
    return 0;
}

int MHO_MK4FringeExport::fill_203( struct type_203 *t203)
{

    clear_203(t203);

//     extern struct mk4_corel cdata; 
//     struct station_struct *refst, *remst;
//     struct chan_struct *refch, *remch;
//     int i, ch, chfound, rootch;
//     struct type_101 *t101;
//     clear_203 (t203);
//                                         /* Get stations from root */
//     refst = remst = NULL;
//     for (i=0; i<root->nst; i++)
//         {
//         if (root->st[i].mk4_site_id == param->baseline[0]) refst = root->st + i;
//         if (root->st[i].mk4_site_id == param->baseline[1]) remst = root->st + i;
//         }
//     if ((refst == NULL) || (remst == NULL))
//         {
//         if (refst == NULL)
//             //msg ("Failed to find station '%c' in ovex file", 
//                                                 2, param->baseline[0]);
//         else
//             //msg ("Failed to find station '%c' in ovex file", 
//                                                 2, param->baseline[1]);
//         return (-1);
//         }
// 
// 
//     ch = 0;
//     for (i=0; i<cdata.index_space; i++)
//         {
//         t101 = cdata.index[i].t101;
//                                         /* Empty slot? */
//         if (t101 == NULL) continue;
//                                         /* Is this a mirror? If so, skip */
// /*         if (i != t101->primary) continue; */
// 
//         t203->channels[ch].index = t101->index;
//                                         /* Assume this is constant for all channels */
//                                         /* and both stations */
//         t203->channels[ch].sample_rate = 1.0 / param->samp_period;
//         strncpy (t203->channels[ch].ref_chan_id, t101->ref_chan_id, 8);
//         strncpy (t203->channels[ch].rem_chan_id, t101->rem_chan_id, 8);
//                                         /* Find the channel name in root */
//         chfound = 0;
//         for (rootch=0; rootch<MAX_CHAN; rootch++)
//             {
//             if (strncmp (t203->channels[ch].ref_chan_id, 
//                         refst->channels[rootch].chan_name, 8) == 0)
//                 {
//                 t203->channels[ch].refsb = refst->channels[rootch].net_sideband;
//                 t203->channels[ch].refpol = refst->channels[rootch].polarization;
//                 t203->channels[ch].ref_freq = refst->channels[rootch].sky_frequency;
//                 chfound++;
//                 }
//             if (strncmp (t203->channels[ch].rem_chan_id, 
//                         remst->channels[rootch].chan_name, 8) == 0)
//                 {
//                 t203->channels[ch].remsb = remst->channels[rootch].net_sideband;
//                 t203->channels[ch].rempol = remst->channels[rootch].polarization;
//                 t203->channels[ch].rem_freq = remst->channels[rootch].sky_frequency;
//                 chfound++;
//                 }
//             }
//         if (chfound != 2)
//             {
//             //msg ("Could not find channel (%s,%s) in root", 2, t101->ref_chan_id,
//                                     t101->rem_chan_id);
//             return (-1);
//             }
//                                         /* Point to next t203 channel */
//         ch++;
//         if (ch == 8 * MAXFREQ)      // ensure there aren't too many channels
//             {
//             //msg ("Too many (%d) t101 channels for t203 record", 2, ch);
//             return -1;
//             }
//         }
// 
    return 0;

}

int MHO_MK4FringeExport::fill_204( struct type_204 *t204)
{
    clear_204(t204);

    // extern char control_filename[], *control_string, version_no[];
    // char *dummy;
    // struct stat buf;
    // struct tm *mod_time, *gmtime(const time_t*);
    // 
    // clear_204 (t204);
    //                                     /* Insert ff_version number */
    // t204->ff_version[0] = version_no[0] - '0';;
    // t204->ff_version[1] = version_no[2] - '0';
    //                                     /* Assume this is run from a */
    //                                     /* standard mk4 environment */
    // dummy = getenv ("ARCH");
    // if (dummy != NULL) strncpy (t204->platform, dummy, 8);
    // else strcpy (t204->platform, "unknown");
    // 
    // strncpy (t204->control_file, control_filename, 96);
    //                                     /* Look up modification date */
    // if (stat (control_filename, &buf) != 0)
    //     //msg ("Failure statting control file '%s'", 1, control_filename);
    // else
    //     {
    //     mod_time = gmtime (&(buf.st_mtime));
    //     t204->ffcf_date.year = mod_time->tm_year + 1900;
    //     t204->ffcf_date.day = mod_time->tm_yday + 1;
    //     t204->ffcf_date.hour = mod_time->tm_hour;
    //     t204->ffcf_date.minute = mod_time->tm_min % 100;
    //     t204->ffcf_date.second = mod_time->tm_sec % 100;
    //     }
    //                                     /* From parse_cmdline() */
    // strncpy (t204->override, control_string, 128);
    // if (strlen (control_string) > 127)
    //     {
    //     //msg ("Warning, command line override string in type 204 record", 1);
    //     //msg ("truncated at 128 characters", 1);
    //     t204->override[124] = t204->override[125] = t204->override[126] = '.';
    //     t204->override[127] = 0;
    //     }
    // 
    return 0;

}

int MHO_MK4FringeExport::fill_205( struct type_203 *t203, struct type_205 *t205)
{
    clear_205(t205);

    // int i, j, ch, nch, int_time, sb, ind,
    //     nchan, nfreqs;
    // struct freq_corel *fc;
    // nchan = (strncmp (t203->version_no, "00", (size_t)2) == 0) ? 32 : 8*MAXFREQ;
    // 
    // clear_205 (t205);
    //                                     /* For now, UCT central is same as FRT */
    // t205->utc_central.year = root->start_time.year;
    // t205->utc_central.second = fmod ((double)param->reftime,  60.0);
    // int_time = param->reftime;       /* In seconds */
    // int_time /= 60;                  /* Now in minutes */
    // t205->utc_central.minute = int_time % 60;
    // int_time /= 60;                  /* Now in hours */
    // t205->utc_central.hour = int_time % 24;
    // t205->utc_central.day = int_time / 24 + 1; /* doy starts at 001 */
    // t205->offset = 0.0;
    //                                     /* Skip fourfit execution modes for now */
    // 
    //                                     /* Search windows */
    // t205->search[0] = param->win_sb[0];
    // t205->search[1] = param->win_sb[1];
    // t205->search[2] = param->win_dr[0];
    // t205->search[3] = param->win_dr[1];
    // t205->search[4] = param->win_mb[0];
    // t205->search[5] = param->win_mb[1];
    //                                     /* Filtering thresholds NYI */
    // 
    //                                     /* Start and stop times for this pass */
    // t205->start.year = root->start_time.year;
    // t205->start.second = fmod ((double)pass->start,  60.0);
    // int_time = pass->start;
    // int_time /= 60;                  /* Now in minutes */
    // t205->start.minute = int_time % 60;
    // int_time /= 60;                  /* Now in hours */
    // t205->start.hour = int_time % 24;
    // t205->start.day = int_time / 24 + 1;
    // 
    // t205->stop.year = root->start_time.year;
    // t205->stop.second = fmod ((double)pass->stop,  60.0);
    // int_time = pass->stop;
    // int_time /= 60;                  /* Now in minutes */
    // t205->stop.minute = int_time % 60;
    // int_time /= 60;                  /* Now in hours */
    // t205->stop.hour = int_time % 24;
    // t205->stop.day = int_time / 24 + 1;
    // 
    // t205->ref_freq = param->ref_freq;
    // 
    // nfreqs = 0;
    // for (ch=0; ch<MAXFREQ; ch++)
    //     {
    //     fc = pass->pass_data + ch;
    //     if (fc->frequency == 0.0 || nfreqs >= pass->nfreq) 
    //         continue;
    //     nfreqs++;
    //     t205->ffit_chan[ch].ffit_chan_id = fc->freq_code;
    //     nch = 0;
    //     for (sb=0; sb<2; sb++)
    //         {
    //         ind = sb + 2 * pass->pol;
    //         if (fc->index[ind] <= 0) 
    //             continue;
    //         for (j=0; j<nchan; j++)
    //             if (fc->index[ind] == t203->channels[j].index) 
    //                 break;
    //         if (j == nchan)
    //             {
    //             //msg ("Could not find index number %d in type 203 record", 
    //                                             2, fc->index[ind]);
    //             return (-1);
    //             }
    //         if (nch >= 4)
    //             {
    //             //msg ("Error - more than 4 correlator indices in ffit chan '%c'", 
    //                                             2, fc->freq_code);
    //             return (-1);
    //             }
    //         t205->ffit_chan[ch].channels[nch] = j;
    //         nch++;
    //         }
    //     }
    // 
    return 0;

}

int MHO_MK4FringeExport::fill_206( struct type_206 *t206)
{
    clear_206(t206);

    // int int_start, max, min, fr, num_ap, samp_per_ap;
    // extern struct type_filter filter;
    // 
    // clear_206 (t206);
    // 
    // t206->start.year = root->start_time.year;
    // t206->start.second = fmod ((double)param->start,  60.0);
    // int_start = param->start;               /* In seconds */
    // int_start /= 60;                        /* Now in minutes */
    // t206->start.minute = int_start % 60;
    // int_start /= 60;                        /* Now in hours */
    // t206->start.hour = int_start % 24;
    // t206->start.day = int_start / 24 + 1;   // add 1 to make doy
    // 
    // t206->first_ap = pass->ap_off;
    // t206->last_ap = pass->ap_off + pass->num_ap;
    //                                         /* Take account of fractional APs */
    // if (pass->channels > 0)
    //     t206->intg_time = status->total_ap_frac * param->acc_period / pass->channels;
    // 
    // // this arises from bandwidth editing, see discussion in adjust_snr.c
    // if (status->tot_sb_bw_aperr > 0.0)
    //     t206->intg_time += status->tot_sb_bw_aperr * param->acc_period / pass->channels;
    // 
    // min = max = status->ap_num[0][0] + status->ap_num[1][0];
    // samp_per_ap = param->acc_period / param->samp_period;
    // for (fr = 0; fr < pass->nfreq; fr++)
    //     {
    //     t206->accepted[fr].usb = status->ap_num[0][fr];
    //     t206->accepted[fr].lsb = status->ap_num[1][fr];
    //     num_ap = status->ap_num[0][fr] + status->ap_num[1][fr];
    //     if (num_ap > max) max = num_ap;
    //     if (num_ap < min) min = num_ap;
    //                                         /* Number of samples by freq/sband */
    //     t206->weights[fr].usb = status->ap_frac[0][fr] * samp_per_ap;
    //     t206->weights[fr].lsb = status->ap_frac[1][fr] * samp_per_ap;
    //     }
    // if (max > 0) t206->accept_ratio = (float)(100 * min) / (float)max;
    // t206->discard = (float)(100 * filter.ndiscard) / 
    //                     (float)(status->total_ap + filter.ndiscard);
    // 
    // t206->ratesize = status->drsp_size;
    // t206->mbdsize = status->grid_points;
    // t206->sbdsize = param->nlags * 4;
    // 
    return 0;

}

int MHO_MK4FringeExport::fill_207( struct type_207 *t207)
{
    clear_207(t207);

    // int i, j, pol;
    // int stnpol[2][4] = {0, 1, 0, 1, 0, 1, 1, 0}; // [stn][pol] = 0:L, 1:R
    // float ref_errate, rem_errate, nreftrk, nremtrk;
    // double midband;
    // struct freq_corel *p;
    // 
    // midband = 2.5e-4 / param->samp_period;  // midband freq in KHz
    // clear_207 (t207);
    //                                     /* Phasecal information */
    // t207->pcal_mode = 10 * param->pc_mode[0] + param->pc_mode[1];
    // for (i=0; i<pass->nfreq; i++)
    //     {
    //     t207->ref_pcamp[i].lsb = status->pc_amp[i][0][stnpol[0][pass->pol]];
    //     t207->rem_pcamp[i].lsb = status->pc_amp[i][1][stnpol[1][pass->pol]];
    //     t207->ref_pcphase[i].lsb = status->pc_meas[i][0][stnpol[0][pass->pol]] * DEGRAD;
    //     t207->rem_pcphase[i].lsb = status->pc_meas[i][1][stnpol[1][pass->pol]] * DEGRAD;
    //     t207->ref_pcoffset[i].lsb = status->pc_offset[i][0][stnpol[0][pass->pol]];
    //     t207->rem_pcoffset[i].lsb = status->pc_offset[i][1][stnpol[1][pass->pol]];
    //     t207->ref_pcamp[i].usb = status->pc_amp[i][0][stnpol[0][pass->pol]];
    //     t207->rem_pcamp[i].usb = status->pc_amp[i][1][stnpol[1][pass->pol]];
    //     t207->ref_pcphase[i].usb = status->pc_meas[i][0][stnpol[0][pass->pol]] * DEGRAD;
    //     t207->rem_pcphase[i].usb = status->pc_meas[i][1][stnpol[1][pass->pol]] * DEGRAD;
    //     t207->ref_pcoffset[i].usb = status->pc_offset[i][0][stnpol[0][pass->pol]];
    //     t207->rem_pcoffset[i].usb = status->pc_offset[i][1][stnpol[1][pass->pol]];
    //                                     /* LSB unused for now  rjc 2001.6.19 */
    //     if (param->pc_mode[0] == MULTITONE)
    //         t207->ref_pcfreq[i].usb = midband;
    //     else
    //         t207->ref_pcfreq[i].usb = pass->pass_data[i].pc_freqs[0][pass->pci[0][i]];
    //     if (param->pc_mode[1] == MULTITONE)
    //         t207->rem_pcfreq[i].usb = midband;
    //     else
    //         t207->rem_pcfreq[i].usb = pass->pass_data[i].pc_freqs[1][pass->pci[1][i]];
    //     }
    // 
    // t207->ref_pcrate = status->pc_rate[0];
    // t207->rem_pcrate = status->pc_rate[1];
    //                                     /* Mean error rates, sidebands averaged */
    // for (i=0; i<pass->nfreq; i++)
    //     {
    //     pol = pass->pol;
    //     p = pass->pass_data + i;
    //     nreftrk = nremtrk = 0.0;
    //     ref_errate = rem_errate = 0.0;
    //                                     /* Add up rates for all tracks */
    //     for (j=0; j<16; j++)
    //         {
    //         if ((pol == 0) || (pol == 2))
    //             if (p->trk_lcp[0][j] >= 0)
    //                 { ref_errate += p->mean_lcp_trk_err[0][j];nreftrk += 1.0; }
    //         if ((pol == 1) || (pol == 3))
    //             if (p->trk_rcp[0][j] >= 0)
    //                 { ref_errate += p->mean_rcp_trk_err[0][j];nreftrk += 1.0; }
    //         if ((pol == 0) || (pol == 3))
    //             if (p->trk_lcp[1][j] >= 0)
    //                 { rem_errate += p->mean_lcp_trk_err[1][j];nremtrk += 1.0; }
    //         if ((pol == 1) || (pol == 2))
    //             if (p->trk_rcp[1][j] >= 0)
    //                 { rem_errate += p->mean_rcp_trk_err[1][j];nremtrk += 1.0; }
    //         }
    //                                     /* Record arithmetic average */
    //     if (nreftrk > 0.0) 
    //         t207->ref_errate[i] = ref_errate / nreftrk;
    //     if (nremtrk > 0.0) 
    //         t207->rem_errate[i] = rem_errate / nremtrk;
    //     }
    // 
    return 0;

}

int MHO_MK4FringeExport::fill_208( struct type_202 *t202, struct type_208 *t208)
{
    clear_208(t208);

    // int fr, ap;
    // extern struct mk4_sdata sdata[]; 
    // struct mk4_sdata *refsd, *remsd;
    // double adelay, arate, aaccel, temp, adelay_ref, arate_ref;
    // double apphase_ref, ref_stn_delay, ambig;
    // double delta_mbd;               // change in mbd to get into desired ambiguity
    // double delta_f;                 // difference between ref freq and nearest freq grid pt
    // char qcode, errcode, tqcode[6];
    // 
    // 
    // extern int
    // compute_model (
    // struct type_param *param,
    // struct mk4_sdata *sdata,
    // struct type_202 *t202,
    // double *delay,
    // double *rate,
    // double *accel,
    // double *delay_ref,
    // double *rate_ref,
    // double *ref_stn_delay);
    // 
    // extern int
    // compute_qf (
    // struct type_pass *pass,
    // struct type_param *param,
    // struct type_status *status,
    // char *qcode,
    // char *errcode,
    // char *tape_qcode);
    // 
    // 
    // clear_208 (t208);
    // 
    // //Add in an indicator of which polarization product we have:
    // //Encode pass and param pol values in the unused1 padding portion of the type 208
    // t208->unused1[0] = pass->pol + POLCHAR_OFFSET; 
    // t208->unused1[1] = param->pol + POLCHAR_OFFSET;
    // t208->unused1[2] = '\0';
    // 
    //                                     /* Compute apriori model */
    // if (compute_model (param, sdata, t202, &adelay, &arate, &aaccel,
    //                    &adelay_ref, &arate_ref, &ref_stn_delay) != 0)
    //     {
    //     //msg ("Model computation fails in fill_208()", 2);
    //     return (-1);
    //     }
    // //msg ("baseline model delay, rate, accel = %g, %g, %g", 0, adelay,arate,aaccel);
    //                                     /* Quality/error codes */
    // if (compute_qf (pass, param, status, &qcode, &errcode, tqcode) != 0)
    //     {
    //     //msg ("Quality/error code computation fails in fill_208()", 2);
    //     return (-1);
    //     }
    // t208->quality = qcode;
    // t208->errcode = errcode;
    // strncpy (t208->tape_qcode, tqcode, 6);
    //                                     /* Convert to usec */
    // t208->adelay = adelay * 1.0e6;
    // t208->arate = arate * 1.0e6;
    // t208->aaccel = aaccel * 1.0e6;
    //                                     /* Totals, residuals, and errors */
    //                                     // status values assigned by update(...GLOBAL)
    //                                     // and subsequenly updated by interp(max555)
    // t208->tot_mbd = t208->adelay + status->mbd_max_global;
    // t208->tot_sbd = t208->adelay + status->sbd_max;
    //                                     // anchor total mbd to sbd if desired
    // ambig = 1.0 / status->freq_space;
    // if (param->mbd_anchor == SBD)
    //     {
    //     delta_mbd = ambig * floor ((t208->tot_sbd - t208->tot_mbd) / ambig + 0.5);
    //     t208->tot_mbd += delta_mbd;
    //     }
    // 
    // t208->tot_rate = t208->arate + status->corr_dr_max;
    //                                     /* ref. stn. time-tagged observables are
    //                                      * approximated by combining retarded a prioris
    //                                      * with non-retarded residuals */
    // t208->tot_mbd_ref  = adelay_ref * 1e6 + status->mbd_max_global;
    // t208->tot_sbd_ref  = adelay_ref * 1e6 + status->sbd_max;
    //                                     // anchor ref mbd as above
    // if (param->mbd_anchor == SBD)
    //     t208->tot_mbd_ref += ambig 
    //                        * floor ((t208->tot_sbd_ref - t208->tot_mbd_ref) / ambig + 0.5);
    // t208->tot_rate_ref = arate_ref * 1e6 + status->corr_dr_max;
    // 
    // t208->resid_mbd = status->mbd_max_global;
    // t208->resid_sbd = status->sbd_max;
    // t208->resid_rate = status->corr_dr_max;
    // t208->mbd_error = (status->nion == 0) ?
    //     (float)(1.0 / (2.0 * M_PI * status->freq_spread * status->snr)) :
    //     1e-3 * status->ion_sigmas[0];
    //     //msg ("mbd sigma w/ no ionosphere %f with ion %f ps", 1, 
    //         (double)(1e6 / (2.0 * M_PI * status->freq_spread * status->snr)), 1e3 * status->ion_sigmas[0]);
    //                                     /* get proper weighting for sbd error estimate */
    // status->sbavg = 0.0;
    // for (fr = 0; fr < pass->nfreq; fr++)
    //     for (ap = pass->ap_off; ap < pass->ap_off + pass->num_ap; ap++) 
    //         status->sbavg += pass->pass_data[fr].data[ap].sband;
    // status->sbavg /= status->total_ap;
    // t208->sbd_error = (float)(sqrt (12.0) * status->sbd_sep * 4.0
    //             / (2.0 * M_PI * status->snr * (2.0 - fabs (status->sbavg) )));
    // temp = status->total_ap * param->acc_period / pass->channels;
    // t208->rate_error = (float)(sqrt(12.0) 
    //                     / ( 2.0 * M_PI * status->snr * param->ref_freq * temp));
    // 
    // t208->ambiguity = 1.0 / status->freq_space;
    // t208->amplitude = status->delres_max/10000.;
    // t208->inc_seg_ampl = status->inc_avg_amp;
    // t208->inc_chan_ampl = status->inc_avg_amp_freq;
    // t208->snr = status->snr;
    // t208->prob_false = status->prob_false;
    // status->apphase = fmod (param->ref_freq * t208->adelay * 360.0, 360.0);
    // t208->totphase = fmod (status->apphase + status->coh_avg_phase
    //                     * (180.0/M_PI) , 360.0);
    //                                     /* Ref stn frame apriori delay usec */
    // adelay_ref *= 1.0e6;
    //                                     /* ref_stn_delay in sec, rate in usec/sec */
    // adelay_ref -= ref_stn_delay * t208->resid_rate;
    // apphase_ref = fmod (param->ref_freq * adelay_ref * 360.0, 360.0);
    // t208->totphase_ref = fmod (apphase_ref + status->coh_avg_phase
    //                     * (180.0/M_PI) , 360.0);
    // t208->resphase = fmod (status->coh_avg_phase * (180.0/M_PI), 360.0);
    //                                 // adjust phases for mbd ambiguity
    // if (param->mbd_anchor == SBD)
    //     {
    //     delta_f = fmod (param->ref_freq - pass->pass_data[0].frequency, status->freq_space);
    //     //msg ("delta_mbd %g delta_f %g", 1, delta_mbd, delta_f);
    //     t208->totphase += 360.0 * delta_mbd * delta_f;
    //     t208->totphase = fmod(t208->totphase, 360.0);
    //     t208->resphase += 360.0 * delta_mbd * delta_f;
    //     t208->resphase = fmod(t208->resphase, 360.0);
    //     }
    // 
    // //msg ("residual phase %f", 1, t208->resphase);
    // 
    // t208->tec_error = (status->nion) ? status->ion_sigmas[2] : 0.0;
    // 
    return 0;

}

int MHO_MK4FringeExport::fill_210( struct type_210 *t210)
{
    clear_210(t210);

    // int i;
    // 
    // clear_210 (t210);
    //                                     /* Precalculated in make_plotdata() */
    // for (i=0; i<pass->nfreq; i++)
    //     {
    //     t210->amp_phas[i].ampl = (float)abs_complex( status->fringe[i] ) / 10000.0;
    //     t210->amp_phas[i].phase = (float)arg_complex( status->fringe[i] ) * 180.0 / pi;
    //     }
    // 
    return 0;

}

int MHO_MK4FringeExport::fill_212( int fr, struct type_212 *t212)
{
    clear_212(t212);

    // int i, ap_212, nap, ap, nrec, aprec, phase, pcal1, pcal2, nrec_per_fr, nalloc;
    // double factor;
    // struct data_corel *datum;
    // extern struct type_plot plot;
    // 
    // clear_212 (t212);
    // 
    // nap = pass->num_ap;
    // t212->nap = nap;
    // t212->first_ap = pass->ap_off;
    // t212->channel = fr;
    // t212->sbd_chan = status->max_delchan;
    //                                     /* Loop over the aps for this pass */
    // for (ap = pass->ap_off; ap < pass->ap_off + nap; ap++)
    //     {
    //                                     /* Location in 212 array starts at 0 */
    //     ap_212 = ap - pass->ap_off;
    //                                     /* Ptr to element in main data array */
    //     datum = pass->pass_data[fr].data + ap;
    //                                     /* Data missing, put in -1 */
    //                                     /* Check on weights is insurance */
    //     if ((datum->flag == 0) || (plot.weights[fr][ap] == 0))
    //         {
    //         t212->data[ap_212].amp = -1.0;
    //         t212->data[ap_212].phase = 0.0;
    //         t212->data[ap_212].weight = 0.0;
    //         continue;
    //         }
    //                                     /* Amplitude and phase */
    //     t212->data[ap_212].amp = abs_complex( plot.phasor[fr][ap] ) * status->amp_corr_fact;
    //     t212->data[ap_212].phase = arg_complex( plot.phasor[fr][ap] );
    //     t212->data[ap_212].weight = plot.weights[fr][ap];
    //     }
    // 
    return 0;

}

int MHO_MK4FringeExport::fill_222( struct type_222 **t222)
{
    clear_222(*t222);

    // int setstr_len, cf_len, setstr_pad, cf_pad, full_size, i;
    // unsigned int setstr_hash = 0;
    // unsigned int cf_hash = 0;
    // 
    // //now allocate the necessary amount of memory
    // if(param->set_string_buff != NULL)
    //     {
    //     setstr_len = strlen(param->set_string_buff);
    //     }
    // else
    //     {
    //     setstr_len = 0;
    //     }
    // 
    // if(param->control_file_buff != NULL)
    //     {
    //     cf_len = strlen(param->control_file_buff);
    //     }
    // else
    //     {
    //     cf_len = 0;
    //     }
    // 
    // //find next largest multiple of 8 bytes
    // setstr_pad = (( setstr_len + 7 ) & ~7) + 8;
    // cf_pad = ( (cf_len + 7 ) & ~7) + 8;
    // full_size = sizeof(struct type_222) + setstr_pad + cf_pad; 
    // 
    // /* Allocate space for output record */
    // *t222 = (struct type_222*) malloc ( full_size );
    // if (*t222 == NULL)
    //     {
    //     //msg ("Memory allocation failure in fill_222()", 2);
    //     return (-1);
    //     }
    // 
    // //now do the hashing
    // setstr_hash = adler32_checksum( (unsigned char*) param->set_string_buff, setstr_len);
    // cf_hash = adler32_checksum( (unsigned char*) param->control_file_buff, cf_len);
    // 
    // /* Fill it in */
    // strncpy ( (*t222)->record_id, "222", 3);
    // strncpy ( (*t222)->version_no, "00", 2);
    // (*t222)->padded = 0;
    // (*t222)->setstring_hash = setstr_hash;
    // (*t222)->control_hash = cf_hash;
    // (*t222)->setstring_length = setstr_len;
    // (*t222)->cf_length = cf_len;
    // 
    // memcpy ( (*t222)->control_contents, param->set_string_buff, setstr_len );
    // for(i=setstr_len; i<setstr_pad; i++){ ((*t222)->control_contents)[i] = '\0';}
    // 
    // //set the starting position of the control contents to the right place
    // memcpy ( &( ((*t222)->control_contents)[setstr_pad] ),  param->control_file_buff, cf_len);
    // for(i=setstr_pad+cf_len; i<setstr_pad+cf_pad; i++){ ((*t222)->control_contents)[i] = '\0';}
    // 
    return 0;

}

int MHO_MK4FringeExport::fill_230( int fr, int ap, struct type_230 *t230)
{
    clear_230(t230);

    // struct data_corel *datum;
    // hops_complex value;
    // static hops_complex work_array[4 * MAXLAG];
    // double theta;
    // int i, j, lag, nl;
    // int stnpol[2][4] = {0, 1, 0, 1, 0, 1, 1, 0}; // [stn][pol] = 0:L, 1:R
    // extern struct type_status status;
    // static int fftsize = 0;
    // static fftw_plan fftplan;
    // 
    // clear_230 (t230);
    // 
    // datum = pass->pass_data[fr].data + ap;
    // 
    // t230->nspec_pts = 2 * param->nlags;
    // t230->frq = fr;
    // t230->ap = ap;
    // t230->usbweight = datum->usbfrac;
    // t230->lsbweight = datum->lsbfrac;
    // if (datum->flag == 0)
    //     t230->usbweight = t230->lsbweight = -1.0;
    // 
    // for (i = 0; i < 4 * MAXLAG; i++)
    //     work_array[i] = 0.0;
    //                                     /* Fill padded work array */
    // nl = param->nlags;
    // if (fftsize != 4 * nl)
    //     {
    //     fftsize = 4 * nl;
    //     fftplan = fftw_plan_dft_1d (fftsize, (fftw_complex*) work_array, (fftw_complex*) work_array, FFTW_FORWARD, FFTW_MEASURE);
    //     }
    // 
    // for (lag = 0; lag < nl * 2; lag++)
    //     {
    //     j = lag - nl;
    //     if (j < 0)
    //         j += 4 * nl;
    //     // value = datum->sbdelay[lag];
    //                                     /* Remove mean phasecal */
    //     // theta = (status.pc_phase[fr][1][stnpol[1][pass->pol]]
    //     //       - status.pc_phase[fr][0][stnpol[0][pass->pol]]);
    //     // work_array[j] = c_mult (value, c_exp (theta));
    //     work_array[j] = datum->sbdelay[lag];
    //     }
    //                                     /* FFT sband delay to xpower spectrum */
    // fftw_execute (fftplan);
    //                                     /* Sort back into xpower array */
    // for (i = 0; i < 2*nl; i++)
    //    {
    //    j = nl - i;
    //    if (j < 0) j += 4*nl;
    //    t230->xpower[i].real = real_comp(work_array[j]);
    //    t230->xpower[i].imag = imag_comp(work_array[j]);
    //    }
    // 
    return 0;

}

int MHO_MK4FringeExport::fill_fringe_info(char *filename)
{

    struct type_200 t200;
    struct type_201 t201;
    struct type_202 t202;
    struct type_203 t203;
    struct type_204 t204;
    struct type_205 t205;
    struct type_206 t206;
    struct type_207 t207;
    struct type_208 t208;
    struct type_210 t210;
    struct type_000 t2_id;

    double sband_err, ref_freq;
    int error, nap, xpow_len, fr, ap, size_of_t212, size_of_t230, recno;
    char buf[256];
    char *t212_array, *t230_array, *address;
    //extern int write_xpower;
    struct mk4_fringe fringe;
    //extern struct type_param param;
    //extern struct type_status status;
    
                                        /* Init */
    clear_mk4fringe (&fringe);
    
    ref_freq = 0.0;//param.ref_freq;
    
    strcpy (buf, filename);
    if (init_000 (&t2_id, filename) != 0)
        {
        //msg ("Error filling in id record", 2);
        return (-1);
        }
    
    error = fill_200(&t200);
    error += fill_201(&t201);
    error += fill_202(&t202);
    error += fill_203(&t203);
    error += fill_204(&t204);
    error += fill_205(&t203, &t205);
    error += fill_206(&t206);
    error += fill_207(&t207);
    error += fill_208(&t202, &t208);
    error += fill_210(&t210);
    
    fringe.id = &t2_id;
    fringe.t200 = &t200;
    fringe.t201 = &t201;
    fringe.t202 = &t202;
    fringe.t203 = &t203;
    fringe.t204 = &t204;
    fringe.t205 = &t205;
    fringe.t206 = &t206;
    fringe.t207 = &t207;
    fringe.t208 = &t208;
    fringe.t210 = &t210;
    //                                     /* Type 212 (ap-by-ap data) records */
    //                                     /* Allocate memory as a block */
    // nap = pass->num_ap;
    // size_of_t212 = sizeof (struct type_212) + 12*(nap-1);
    // if ((nap % 2) == 1) size_of_t212 += 12;
    // t212_array = (char *)malloc (pass->nfreq * size_of_t212);
    // if (t212_array == NULL)
    //     {
    //     //msg ("Failure allocating memory for type 212 records!", 2);
    //     return (0);
    //     }
    //                                     /* record the allocation */
    // fringe.allocated[fringe.nalloc] = t212_array;
    // fringe.nalloc += 1;
    //                                     /* Fill in records and pointers */
    // fringe.n212 = pass->nfreq;
    // for (fr=0; fr<pass->nfreq; fr++)
    //     {
    //     address = t212_array + (fr * size_of_t212);
    //     fringe.t212[fr] = (struct type_212 *)address;
    //     error += fill_212 (pass, &status, &param, fr, fringe.t212[fr]);
    //     }
    //                                     /* Cross power spectra (if requested) */
    // if (write_xpower)
    //     {
    //                                     /* Allocate memory as a block */
    //     xpow_len = 16 * 2 * param.nlags;
    //     size_of_t230 = sizeof (struct type_230) - sizeof (hops_complex) + xpow_len;
    //     t230_array = (char *)malloc (pass->nfreq * nap * size_of_t230);
    //     if (t230_array == NULL)
    //         {
    //         //msg ("Failure allocating memory for type 230 records!", 2);
    //         return (0);
    //         }
    //                                     /* record the allocation */
    //     fringe.allocated[fringe.nalloc] = t230_array;
    //     fringe.nalloc += 1;
    //                                     /* Loop over all freqs, aps */
    //     recno = 0;
    //     for (fr=0; fr<pass->nfreq; fr++)
    //         for (ap = pass->ap_off; ap < pass->ap_off + nap; ap++)
    //             {
    //             address = t230_array + recno * size_of_t230;
    //             fringe.t230[recno] = (struct type_230 *)address;
    //             error += fill_230 (pass, &param, fr, ap, fringe.t230[recno]);
    //             recno++;
    //             }
    //     fringe.n230 = recno;
    //     }
    // 
    // if (error != 0)
    //     //msg ("Warning - some or all of the output records were not filled", 2);
    // 
    // status.amp_err = status.delres_max / status.snr;
    // status.resid_phase = status.coh_avg_phase * ( 180.0 / M_PI);
    // status.mod_resid_phase *= 180.0 / M_PI;
    // sband_err = sqrt (1.0 + 3.0 * (status.sbavg * status.sbavg));
    // status.phase_err = (status.nion == 0) ?
    //     180.0 * sband_err / (M_PI * status.snr) :
    //     360.0 * status.ion_sigmas[1];
    // status.resid_ph_delay = status.coh_avg_phase / (2.0 * M_PI *ref_freq);
    // status.ph_delay_err = sband_err / (2.0 * M_PI * status.snr * ref_freq);
    

    return 0;

}

}//end of namespace