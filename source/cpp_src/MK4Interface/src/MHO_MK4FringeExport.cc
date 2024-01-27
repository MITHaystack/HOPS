#include "MHO_MK4FringeExport.hh"

#include "MHO_LegacyDateConverter.hh"

#include "MHO_MK4Type200Converter.hh"
#include "MHO_MK4Type201Converter.hh"
#include "MHO_MK4Type202Converter.hh"
#include "MHO_MK4Type203Converter.hh"
#include "MHO_MK4Type204Converter.hh"

#include "MHO_MK4Type208Converter.hh"


#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>


#include <algorithm>
#include <array>

#define LOCK_STATUS_OK 0

namespace hops
{


MHO_MK4FringeExport::MHO_MK4FringeExport()
{

}

MHO_MK4FringeExport::~MHO_MK4FringeExport()
{

}


int MHO_MK4FringeExport::fill_200( struct type_200 *t200)
{

    bool ok;
    clear_200(t200);

    //set to zero for now
    t200->software_rev[0] = 0; //HOPS_SVN_REV;

    std::string exper_num;
    ok = fPStore->Get("/vex/experiment_number", exper_num);
    if(!ok){exper_num = "9999";}
    t200->expt_no = std::atoi(exper_num.c_str()); // root->exper_num;

    FillString( &(t200->exper_name[0]), "/vex/experiment_name", 32);
    FillString( &(t200->scan_name[0]), "/vex/scan/name", 32);
    FillString( &(t200->correlator[0]), "/correlator/name", 32, "DiFX");
    FillDate(&(t200->scantime), "/vex/scan/start");
    FillInt(t200->start_offset, "/control/selection/start", 0);
    FillInt(t200->stop_offset, "/control/selection/stop", 0);

    //get the current time
    legacy_hops_date now_date = MHO_LegacyDateConverter::Now();
    FillDate(&(t200->fourfit_date), now_date);

    //TODO FIXME -- store and retrieve the correlation date info in HOPS4
    //currently we do not have the correlation date information, so just the current time
    FillDate(&(t200->corr_date), now_date);

    //write out the fourfit reference time 
    FillDate(&(t200->frt), "/vex/scan/fourfit_reftime");

    mho_json j = convertToJSON(*t200);
    std::cout<<"type 200 json = "<<j.dump(2)<<std::endl;

    return 0;
}

int MHO_MK4FringeExport::fill_201( struct type_201 *t201)
{

    bool ok;
    clear_201(t201);

    FillString( &(t201->source[0]), "/vex/scan/source/name", 32);

    std::string source_ra;
    ok = fPStore->Get("/vex/scan/source/ra", source_ra);
    if(!ok){source_ra = "00h00m00.0s"; }
    std::string source_dec;
    ok = fPStore->Get("/vex/scan/source/dec", source_dec);
    if(!ok){source_dec = "00d00'00.0\"";}

    struct sky_coord src_coords;
    int check = convert_sky_coords(src_coords, source_ra, source_dec);
    if(check != 0){msg_error("mk4interface", "error converting source coordinates" << eom); return -1;}
    t201->coord.ra_hrs = src_coords.ra_hrs;
    t201->coord.ra_mins = src_coords.ra_mins;
    t201->coord.ra_secs = src_coords.ra_secs;
    t201->coord.dec_degs = src_coords.dec_degs;
    t201->coord.dec_mins = src_coords.dec_mins;
    t201->coord.dec_secs = src_coords.dec_secs;

    //TODO FIXME, just use 2000 for now
    //this is stored in the root file as "ref_coord_frame", but we have yet to extract it and add to the parameter store
    t201->epoch = 2000;
    ///t201->epoch = 1950;

    //TODO FIXME, this optional parameter (src.position_epoch) may or may not be present in the vex/root file
    FillDate(&(t201->coord_date), "/vex/scan/source/source_position_epoch");

    //TODO FIXME, these optional parameters may or may not be present in the vex/root file
    FillDouble(t201->ra_rate, "/vex/scan/source/ra_rate");
    FillDouble(t201->dec_rate, "/vex/scan/source/dec_rate");

    // NOTE!!!! Differential TEC is accidentally
    // in the sense of reference_tec - remote_tec
    // This differs from all other diff. quantities
    FillDouble(t201->dispersion, "/fringe/ion_diff");

    // /* Ignore the rest of pulsar parameters for now */
    mho_json j = convertToJSON(*t201);
    std::cout<<"type 201 json = "<<j.dump(2)<<std::endl;

    return 0;

}

int MHO_MK4FringeExport::fill_202( struct type_202 *t202)
{
    bool ok;
    clear_202(t202);

    
    FillString( &(t202->baseline[0]), "/config/baseline", 2);
    FillString( &(t202->ref_intl_id[0]), "/ref_station/site_id", 2);
    FillString( &(t202->rem_intl_id[0]), "/rem_station/site_id", 2);
    FillString( &(t202->ref_name[0]), "/ref_station/site_name", 8);
    FillString( &(t202->rem_name[0]), "/rem_station/site_name", 8);
    FillString( &(t202->ref_tape[0]), "/ref_station/tape_name", 8); //obsolete
    FillString( &(t202->rem_tape[0]), "/rem_station/tape_name", 8); //obsolete

    short nlags = fPStore->GetAs<int>("/config/nlags");
    t202->nlags = (short) nlags;

    FillDouble(t202->ref_xpos, "ref_station/position/x/value");
    FillDouble(t202->ref_ypos, "ref_station/position/y/value");
    FillDouble(t202->ref_zpos, "ref_station/position/z/value");
    FillDouble(t202->rem_xpos, "rem_station/position/x/value");
    FillDouble(t202->rem_ypos, "rem_station/position/y/value");
    FillDouble(t202->rem_zpos, "rem_station/position/z/value");

    #pragma message("TODO FIXME -- finish t202")

    t202->u = 0.0;
    t202->v = 0.0;
    t202->uf = 0.0;
    t202->vf = 0.0;
    t202->ref_clock = 0.0;
    t202->rem_clock = 0.0;
    t202->ref_clockrate = 0.0;
    t202->rem_clockrate = 0.0;
    t202->ref_idelay = 0.0;
    t202->rem_idelay = 0.0;
    t202->ref_zdelay = 0.0;
    t202->rem_zdelay = 0.0;
    t202->ref_elev = 0.0;
    t202->rem_elev = 0.0;
    t202->ref_az = 0.0;
    t202->rem_az = 0.0;

    mho_json j = convertToJSON(*t202);
    std::cout<<"type 202 json = "<<j.dump(2)<<std::endl;

    return 0;
}

int MHO_MK4FringeExport::fill_203( struct type_203 *t203)
{
    clear_203(t203);
    std::size_t nchannels = MAX_CHAN;
    FillChannels( &(t203->channels[0]) , nchannels);

    mho_json j = convertToJSON(*t203);
    std::cout<<"type 203 json = "<<j.dump(2)<<std::endl;

    return 0;
}

int MHO_MK4FringeExport::fill_204( struct type_204 *t204)
{
    clear_204(t204);

    t204->ff_version[0];
    t204->ff_version[0];

    std::string tmp = getenv("HOPS_ARCH");
    char_clear( &(t204->platform[0]), 8);
    strncpy(&(t204->platform[0]), tmp.c_str(), std::min(8, (int) tmp.size() ) );

    FillString(&(t204->control_file[0]), "/files/control_file", 96);

    /* Look up modification date */
    struct stat buf;
    if( stat(t204->control_file, &buf) == 0)
    {
        struct tm *mod_time;
        mod_time = gmtime (&(buf.st_mtime));
        t204->ffcf_date.year = mod_time->tm_year + 1900;
        t204->ffcf_date.day = mod_time->tm_yday + 1;
        t204->ffcf_date.hour = mod_time->tm_hour;
        t204->ffcf_date.minute = mod_time->tm_min % 100;
        t204->ffcf_date.second = mod_time->tm_sec % 100;
    }

    //this is the text after 'set' on the command line, Ignore for now
    std::string set_string = "unknown";
    char_clear( &(t204->override[0]), 128);
    strncpy(&(t204->override[0]), set_string.c_str(), std::min(128, (int) set_string.size() ) );

    mho_json j = convertToJSON(*t204);
    std::cout<<"type 204 json = "<<j.dump(2)<<std::endl;

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
    bool ok;
    clear_208(t208);

    std::string qcode;
    ok = fPStore->Get("/fringe/quality_code", qcode);
    if(!ok){qcode = "0";}
    t208->quality = qcode[0];

    std::string errcode;
    ok = fPStore->Get("/fringe/error_code", errcode);     //TODO implement the error code calc!
    errcode = " ";
    t208->errcode = errcode[0];

    //not used
    strncpy(t208->tape_qcode, "000000", 6);

    FillDouble(t208->adelay, "/model/adelay");
    FillDouble(t208->arate, "/model/arate");
    FillDouble(t208->aaccel, "/model/aaccel");
    FillDouble(t208->tot_mbd, "/fringe/total_mbdelay");
    FillDouble(t208->tot_sbd, "/fringe/total_sbdelay");
    FillDouble(t208->tot_rate, "/fringe/total_drate");

    #pragma message("TODO FIXME -- the totals for the reference station are not yet calculated")
    FillDouble(t208->tot_mbd_ref, "/fringe/total_mbdelay_ref");
    FillDouble(t208->tot_sbd_ref, "/fringe/total_mbdelay_ref");
    FillDouble(t208->tot_rate_ref, "/fringe/total_mbdelay_ref");

    FillFloat(t208->resid_mbd, "/fringe/mbdelay");
    FillFloat(t208->resid_sbd, "/fringe/sbdelay");
    FillFloat(t208->resid_rate, "/fringe/drate");
    FillFloat(t208->resid_mbd, "/fringe/mbd_error");
    FillFloat(t208->resid_mbd, "/fringe/sbd_error");

    FillFloat(t208->rate_error, "/fringe/drate_error");
    FillFloat(t208->ambiguity, "/fringe/ambiguity");
    FillFloat(t208->amplitude, "/fringe/famp");

    #pragma message("TODO FIXME inc_seg, inc_chan amps, and PFD are in plot data, move to parameter store?")
    FillFloat(t208->inc_seg_ampl, "/fringe/inc_seg_ampl");
    FillFloat(t208->inc_chan_ampl, "/fringe/inc_chan_ampl");
    FillFloat(t208->snr, "/fringe/snr");
    FillFloat(t208->prob_false, "/fringe/pfd");
    FillFloat(t208->totphase, "/fringe/tot_phase");
    FillFloat(t208->totphase_ref, "/fringe/tot_phase_ref"); //DOES NOT EXIST YET
    FillFloat(t208->resphase, "/fringe/resid_phase");
    FillFloat(t208->tec_error, "/fringe/tec_error"); //DOES NOT EXIST YET

    mho_json j = convertToJSON(*t208);
    std::cout<<"type 208 json = "<<j.dump(2)<<std::endl;

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

//dummy, just clears the structure
int MHO_MK4FringeExport::fill_221( struct type_221* t221)
{
    char version[3];
    strncpy (t221->record_id, "221", 3);
    sprintf (version, "%02d", T221_VERSION);
    strncpy (t221->version_no, version, 2);
    t221->unused1 = ' ';
    t221->padded = 0;
    t221->ps_length = 0;
    t221->pplot[0] = '\0';

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

    //extern struct type_param param;
    //extern struct type_status status;
    
                                        /* Init */
    clear_mk4fringe (&fringe);
    
    ref_freq = 0.0;//param.ref_freq;
    
    strcpy (buf, filename);
    int val = init_000 (&t2_id, filename);
    if(  val != 0)
    {
        std::cout<<"error t000: "<<val<<std::endl;
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

    std::cout<<"done filling"<<std::endl;


                                        /* Type 212 (ap-by-ap data) records */
                                        /* Allocate memory as a block */
    nap = fPStore->GetAs<int>("/config/total_naps");
    int nfreq = 32;
    size_of_t212 = sizeof (struct type_212) + 12*(nap-1);
    if ((nap % 2) == 1) size_of_t212 += 12;
    t212_array = (char *)malloc (nfreq * size_of_t212);
    if (t212_array == NULL)
    {
        //msg ("Failure allocating memory for type 212 records!", 2);
        return (0);
    }
                                        /* record the allocation */
    fringe.allocated[fringe.nalloc] = t212_array;
    fringe.nalloc += 1;


    //                                     /* Fill in records and pointers */
    fringe.n212 = nfreq;
    for (fr=0; fr < nfreq; fr++)
    {
        address = t212_array + (fr * size_of_t212);
        fringe.t212[fr] = (struct type_212 *)address;
        // error += fill_212 (pass, &status, &param, fr, fringe.t212[fr]);
        error += fill_212(fr, fringe.t212[fr]);
    }
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



int
MHO_MK4FringeExport::output(std::string filename)
{
    std::cout<<"output"<<std::endl;

    char fringe_name[256];
    for(std::size_t i=0; i<256; i++){fringe_name[i] = '\0';}

    if(filename.size() > 255)
    {
        msg_fatal("mk4interface", "filename exceeds max length of 256." << eom);
        std::exit(1);
    }
    strncpy(fringe_name, filename.c_str(), filename.size());

    char sg;
    int i, dret;
    char **fplot;
    int the_seq_no;
    bool test_mode = false;

    //struct type_221 *t221;

    struct type_222 *t222;

    // // for locking, see below and include/write_lock_mechanism.h
    int lock_retval = LOCK_STATUS_OK;//LOCK_PROCESS_NO_PRIORITY;
    // char lockfile_name[512] = {'\0'};

    /* Generate information to create fringe plot */
    /* Some of this also goes into fringe file */

    //try to get a lock on the root directory in order to write the fringe
    //this is used to signal any other possible fourfit processes in this
    //directory that we are about to create a file so we can avoid name 
    //collisions.  The lock persists from point of acqusition until the
    //eventual call to write_mk4fringe() below.
    // FIXME: should worry about stale locks if ^C is hit.
    // if(!test_mode)
    //     {
    //     struct fileset fset;
    //     //wait until we are the next process allowed to write an output file
    //     lock_retval = wait_for_write_lock(root->ovex->filename,
    //         lockfile_name, &fset);
    //     //this is the last filenumber that exists on disk
    //     the_seq_no = fset.maxfile;
    //     }
    // else
    //     {
    //     // in test mode, nothing should be written, so the number is moot.
    //     the_seq_no = -1;
    //     }

    // /* create_fname() will put the next seq number into the fringe name */
    // the_seq_no++;
    //                                 /* Figure out the correct, full pathname */
    // if (create_fname (root->ovex, pass, the_seq_no, fringe_name) != 0)
    //     {
    //     msg ("Error figuring out proper fringe filename", 2);
    //     return (1);
    //     }
        /* Fill in fringe file structure */
    if(fill_fringe_info(fringe_name) != 0)
    {
        //msg ("Error filling fringe records", 2);
        return 1;
    }


    // 
    // if (make_postplot (root->ovex, pass, fringe_name, &t221) != 0)
    //     {
    //     msg ("Error creating postscript plot", 2);
    //     return (1);
    //     }

    struct type_221 t221;
    fill_221(&t221);
    fringe.t221 = &t221;

    // fringe.t221 = NULL;

    //fringe.t221 = t221;
    //fringe.t221->ps_length = strlen (fringe.t221->pplot);
            /* Record the memory allocation */
    // fringe.allocated[fringe.nalloc] = fringe.t221;
    // fringe.nalloc += 1;
    
           /* Fill in the control file record */
           /* if desired */
    fringe.t222 = NULL;
    // if(param.gen_cf_record)
    // {
    //     if (fill_222 (&param, &t222) != 0)
    //     {
    //         //msg ("Error filling control record", 2);
    //         return (1);
    //     }
    // 
    //     fringe.t222 = t222;
    //             /* Record the memory allocation */
    //     fringe.allocated[fringe.nalloc] = fringe.t222;
    //     fringe.nalloc += 1;
    // }

            /* Actually write output fringe file */
    //if( !test_mode)
    {
        //if( lock_retval == LOCK_STATUS_OK)
        {
            //kludge to get fourfit to feed the generated fringe file name 
            //(but nothing else) as a return value to a
            //a python calling script (requires passing option "-m 4"); see
            //e.g. chops/source/python_src/hopstest_module/hopstestb/hopstestb.py
            //around line 74 in the FourFitThread class.
            //if(msglev==4){msg ("%s",4,fringe_name);} //iff msglev=4
            std::cout<<"writing fringe file with name "<<filename<<std::endl;
            int val = write_mk4fringe(&fringe, fringe_name);
            std::cout<<"write fringe retval = "<<val<<std::endl;
            // if(write_mk4fringe(&fringe, fringe_name) != 0)
            // {
            //     // pause 50ms, if a lock file was created, delete it now
            //     //usleep(50000); remove_lockfile();
            //     //msg ("Error writing fringe file", 2);
            //     return 1;
            // }
            //if a lock file was created, delete it now
            //usleep(50000); remove_lockfile();
        }
        // else
        // {
        //     //msg ("Error getting write lock on directory.", 2);
        //     return 1;
        // }
    }

    return 0;
}


int
MHO_MK4FringeExport::convert_sky_coords(struct sky_coord& coords, std::string ra, std::string dec)
{
    short ra_hrs = 0;
    short ra_mins = 0;
    float ra_secs = 0;
    short dec_degs = 0;
    short dec_mins = 0;
    float dec_secs = 0;

    // "source": {
    //   "dec": "73d27'30.0174\"",
    //   "name": "0016+731",
    //   "ra": "00h19m45.78642s"
    // },

    std::string delim1 = "d";
    std::string delim2 = "'";
    std::string delim3 = "h";
    std::string delim4 = "m";

    std::string tmp;
    std::vector< std::string > tokens;
    fTokenizer.SetUseMulticharacterDelimiterFalse();
    fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
    fTokenizer.SetIncludeEmptyTokensFalse();

    fTokenizer.SetDelimiter(delim1);
    fTokenizer.SetString(&dec);
    fTokenizer.GetTokens(&tokens);
    if(tokens.size() != 2){ msg_error("mk4interface", "error parsing dec string: " << dec << " with delimiter" << delim1 << eom); return 1;}
    
    tmp = tokens[0];
    dec_degs = std::atoi(tmp.c_str());
    tmp = tokens[1];

    fTokenizer.SetDelimiter(delim2);
    fTokenizer.SetString(&tmp);
    fTokenizer.GetTokens(&tokens);
    if(tokens.size() != 2){ msg_error("mk4interface", "error parsing dec string: " << dec << " with delimiter" << delim2 << eom); return 1;}
    
    tmp = tokens[0];
    dec_mins = std::atoi(tmp.c_str());
    tmp = tokens[1];

    std::size_t last = tokens[1].find_first_not_of("0123456789.e+-");
    tmp = tokens[1].substr(0, last);
    dec_secs = std::atof(tmp.c_str());

    fTokenizer.SetDelimiter(delim3);
    fTokenizer.SetString(&ra);
    fTokenizer.GetTokens(&tokens);
    if(tokens.size() != 2){ msg_error("mk4interface", "error parsing ra string: " << ra << " with delimiter" << delim3 << eom); return 1;}
    
    tmp = tokens[0];
    ra_hrs = std::atoi(tmp.c_str());
    tmp = tokens[1];

    fTokenizer.SetDelimiter(delim4);
    fTokenizer.SetString(&tmp);
    fTokenizer.GetTokens(&tokens);
    if(tokens.size() != 2){ msg_error("mk4interface", "error parsing ra string: " << ra << " with delimiter" << delim4 << eom); return 1;}
    
    tmp = tokens[0];
    ra_mins = std::atoi(tmp.c_str());
    tmp = tokens[1];

    std::size_t last2 = tokens[1].find_first_not_of("0123456789.e+-");
    tmp = tokens[1].substr(0, last2);
    ra_secs = std::atof(tmp.c_str());

    coords.ra_hrs = ra_hrs;
    coords.ra_mins = ra_mins;
    coords.ra_secs = ra_secs;
    coords.dec_degs = dec_degs;
    coords.dec_mins = dec_mins;
    coords.dec_secs = dec_secs;

    return 0;
}

void 
MHO_MK4FringeExport::FillString(char* destination, std::string param_path, int max_length, std::string default_value)
{
    char_clear(destination, max_length);
    std::string tmp; 
    bool ok = fPStore->Get(param_path, tmp);
    if(!ok){tmp = default_value;}
    strncpy(destination, tmp.c_str(), std::min( max_length, (int) tmp.size() ) );
}

void 
MHO_MK4FringeExport::FillInt(int& destination, std::string param_path, int default_value)
{
    int value;
    bool ok = fPStore->Get(param_path, value);
    if(!ok){value = default_value;}
    destination = value;
}

void 
MHO_MK4FringeExport::FillDouble(double& destination, std::string param_path, double default_value)
{
    double value;
    bool ok = fPStore->Get(param_path, value);
    if(!ok){value = default_value;}
    destination = value;
}

void 
MHO_MK4FringeExport::FillFloat(float& destination, std::string param_path, float default_value)
{
    double value;
    bool ok = fPStore->Get(param_path, value); //all params are stored as doubles
    if(!ok){value = default_value;}
    destination = value;
}

void 
MHO_MK4FringeExport::FillDate(struct date* destination, std::string param_path)
{
    legacy_hops_date a_date;
    std::string date_vex_string;
    bool ok = fPStore->Get(param_path, date_vex_string);
    if(ok){a_date = MHO_LegacyDateConverter::ConvertFromVexFormat(date_vex_string);}
    else{ a_date = MHO_LegacyDateConverter::HopsEpoch(); } //dummy date
    destination->year = a_date.year;
    destination->day = a_date.day;
    destination->hour = a_date.hour;
    destination->minute = a_date.minute;
    destination->second = a_date.second;
}

void 
MHO_MK4FringeExport::FillDate(struct date* destination, struct legacy_hops_date& a_date)
{
    destination->year = a_date.year;
    destination->day = a_date.day;
    destination->hour = a_date.hour;
    destination->minute = a_date.minute;
    destination->second = a_date.second;
}

void MHO_MK4FringeExport::FillChannels(struct ch_struct* chan_array, std::size_t nchannels)
{
    // visibility_type* vis_data = fCStore->GetObject<visibility_type>(std::string("vis"));
    // if( vis_data == nullptr || wt_data == nullptr )
    // {
    //     msg_fatal("fringe", "could not find visibility object with name: vis." << eom);
    //     std::exit(1);
    // }
    // auto chan_ax = &( std::get<CHANNEL_AXIS>(*vis_data) );

    //limit to supported number of channels
    std::size_t nchan = 32;// chan_ax->GetSize();
    nchan = std::min(nchan, nchannels);
    for(std::size_t ch=0; ch < nchan; ch++)
    {
        int findex = ch;
        double bandwidth = 0;
        short index = 0;
        unsigned short int sample_rate = 0;
        std::string refsb = "";
        std::string remsb = "";
        std::string refpol = "";
        std::string rempol  = "";
        double ref_freq = 0;
        double rem_freq = 0;
        std::string ref_chan_id = "";
        std::string rem_chan_id = "";

        // chan_ax->RetrieveIndexLabelKeyValue(ch, "index", findex);
        // chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", refsb);
        // chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", remsb);
        // chan_ax->RetrieveIndexLabelKeyValue(ch, "sky_freq", ref_freq);
        // chan_ax->RetrieveIndexLabelKeyValue(ch, "sky_freq", rem_freq);
        // chan_ax->RetrieveIndexLabelKeyValue(ch, "bandwidth", bandwidth);
        // chan_ax->RetrieveIndexLabelKeyValue(ch, "chan_id", rem_chan_id);
        // chan_ax->RetrieveIndexLabelKeyValue(ch, "chan_id", ref_chan_id);

        index = (short)findex;
        sample_rate = (unsigned short int)  (2.0*bandwidth*1000.0); //sample rate = 2 x bandwidth (MHz) x (1000KHz/MHz)

        chan_array[ch].index = index;
        chan_array[ch].sample_rate = sample_rate;
        chan_array[ch].refsb = ' '; //refsb[0];
        chan_array[ch].remsb = ' '; //remsb[0];
        chan_array[ch].refpol = ' '; //refpol[0];
        chan_array[ch].rempol = ' '; //rempol[0];
        chan_array[ch].ref_freq = ref_freq;
        chan_array[ch].rem_freq = rem_freq;
        char_clear(&(chan_array[ch].ref_chan_id[0]),8);
        char_clear(&(chan_array[ch].rem_chan_id[0]),8);
        strncpy( &(chan_array[ch].ref_chan_id[0]), ref_chan_id.c_str(), std::min(7, (int) ref_chan_id.size() ) );
        strncpy( &(chan_array[ch].rem_chan_id[0]), rem_chan_id.c_str(), std::min(7, (int) rem_chan_id.size() ) );
    }

}


}//end of namespace