#include "MHO_MK4FringeExport.hh"

#include "hops_version.hh"

//mk4 IO library
#ifndef HOPS3_USE_CXX
extern "C"
{
#endif
#include "mk4_data.h"
#include "mk4_dfio.h"
#ifndef HOPS3_USE_CXX
}
#endif

#include "MHO_LegacyDateConverter.hh"
#include "MHO_LockFileHandler.hh"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include <algorithm>
#include <array>

#define LOCK_STATUS_OK 0

namespace hops
{

unsigned int adler32_checksum(unsigned char* buf, int len)
{
#define MOD_ADLER 65521

    unsigned int s1 = 1;
    unsigned int s2 = 0;
    int n;

    for(n = 0; n < len; n++)
    {
        s1 = (s1 + buf[n]) % MOD_ADLER;
        s2 = (s2 + s1) % MOD_ADLER;
    }
    return (s2 << 16) + s1;
}

MHO_MK4FringeExport::MHO_MK4FringeExport()
{}

MHO_MK4FringeExport::~MHO_MK4FringeExport()
{}

int MHO_MK4FringeExport::fill_200(struct type_200* t200)
{

    bool ok;
    clear_200(t200);

    //set to zero for now
    t200->software_rev[0] = 0; //HOPS_SVN_REV;

    std::string exper_num;
    ok = fPStore->Get("/vex/experiment_number", exper_num);
    if(!ok)
    {
        exper_num = "9999";
    }
    t200->expt_no = std::atoi(exper_num.c_str()); // root->exper_num;

    FillString(&(t200->exper_name[0]), "/vex/experiment_name", 32);
    FillString(&(t200->scan_name[0]), "/vex/scan/name", 32);
    FillString(&(t200->correlator[0]), "/correlator/name", 32, "difx");
    FillDate(&(t200->scantime), "/vex/scan/start");

    //we need to retrieve these values from the vex (it is the start/stop of the scan in the schedule, not data)

    FillInt(t200->start_offset, "/start_offset", 0);
    FillInt(t200->stop_offset, "/stop_offset", 0);

    //pull the processing date from the parameter store
    std::string procdate_vex = fPStore->GetAs< std::string >("/fringe/procdate");
    legacy_hops_date procdate = MHO_LegacyDateConverter::ConvertFromVexFormat(procdate_vex);
    FillDate(&(t200->fourfit_date), procdate);

    //the correlation processing date
    FillDate(&(t200->corr_date), "/config/correlation_date");

    //write out the fourfit reference time
    FillDate(&(t200->frt), "/vex/scan/fourfit_reftime");

    return 0;
}

int MHO_MK4FringeExport::fill_201(struct type_201* t201)
{

    bool ok;
    clear_201(t201);

    FillString(&(t201->source[0]), "/vex/scan/source/name", 32);

    std::string source_ra;
    ok = fPStore->Get("/vex/scan/source/ra", source_ra);
    if(!ok)
    {
        source_ra = "00h00m00.0s";
    }
    std::string source_dec;
    ok = fPStore->Get("/vex/scan/source/dec", source_dec);
    if(!ok)
    {
        source_dec = "00d00'00.0\"";
    }

    struct sky_coord src_coords;
    int check = convert_sky_coords(src_coords, source_ra, source_dec);
    if(check != 0)
    {
        msg_error("mk4interface", "error converting source coordinates" << eom);
        return -1;
    }
    t201->coord.ra_hrs = src_coords.ra_hrs;
    t201->coord.ra_mins = src_coords.ra_mins;
    t201->coord.ra_secs = src_coords.ra_secs;
    t201->coord.dec_degs = src_coords.dec_degs;
    t201->coord.dec_mins = src_coords.dec_mins;
    t201->coord.dec_secs = src_coords.dec_secs;

    //default to year 2000
    t201->epoch = 2000;
    std::string ref_coord_frame;
    FillString(ref_coord_frame, "/vex/scan/source/ref_coord_frame");
    if(ref_coord_frame.find("1950") != std::string::npos)
    {
        t201->epoch = 1950;
    }
    if(ref_coord_frame.find("2000") != std::string::npos)
    {
        t201->epoch = 2000;
    }

    //TODO FIXME, this optional parameter (src.position_epoch) may or may not be present in the vex/root file
    FillDate(&(t201->coord_date), "/vex/scan/source/source_position_epoch");

    //TODO FIXME, these optional parameters may or may not be present in the vex/root file
    FillDouble(t201->ra_rate, "/vex/scan/source/ra_rate");
    FillDouble(t201->dec_rate, "/vex/scan/source/dec_rate");

    // NOTE!!!! Differential TEC is accidentally
    // in the sense of reference_tec - remote_tec
    // This differs from all other diff. quantities
    FillDouble(t201->dispersion, "/fringe/ion_diff");

    return 0;
}

int MHO_MK4FringeExport::fill_202(struct type_202* t202)
{
    bool ok;
    clear_202(t202);

    FillString(&(t202->baseline[0]), "/config/baseline", 2);
    FillString(&(t202->ref_intl_id[0]), "/ref_station/site_id", 2);
    FillString(&(t202->rem_intl_id[0]), "/rem_station/site_id", 2);
    FillString(&(t202->ref_name[0]), "/ref_station/site_name", 8);
    FillString(&(t202->rem_name[0]), "/rem_station/site_name", 8);
    FillString(&(t202->ref_tape[0]), "/ref_station/tape_name", 8); //obsolete
    FillString(&(t202->rem_tape[0]), "/rem_station/tape_name", 8); //obsolete

    short nlags = fPStore->GetAs< int >("/config/nlags");
    t202->nlags = (short)nlags;

    FillDouble(t202->ref_xpos, "/ref_station/position/x/value");
    FillDouble(t202->ref_ypos, "/ref_station/position/y/value");
    FillDouble(t202->ref_zpos, "/ref_station/position/z/value");
    FillDouble(t202->rem_xpos, "/rem_station/position/x/value");
    FillDouble(t202->rem_ypos, "/rem_station/position/y/value");
    FillDouble(t202->rem_zpos, "/rem_station/position/z/value");

    FillFloat(t202->ref_clockrate, "/ref_station/clock_rate");
    FillFloat(t202->rem_clockrate, "/rem_station/clock_rate");
    FillFloat(t202->ref_clock, "/ref_station/clock_early_offset");
    FillFloat(t202->rem_clock, "/rem_station/clock_early_offset");

    //note that in HOPS4 these (az,el, u,v) are evaluated at the FRT,
    //so they will differ from the HOPS3 values (evaluated at scan start)

    FillFloat(t202->ref_elev, "/ref_station/elevation");
    FillFloat(t202->rem_elev, "/rem_station/elevation");
    FillFloat(t202->ref_az, "/ref_station/azimuth");
    FillFloat(t202->rem_az, "/rem_station/azimuth");

    // double ref_freq;
    // FillDouble(ref_freq, "/control/config/ref_freq");

    // double speed_of_light_Mm = 299.792458; // in mega-meters
    // double radians_to_arcsec = 4.848137e-6;
    // double lambda = speed_of_light_Mm / ref_freq; // wavelength (m)
    //
    // double ref_u, ref_v;
    // double rem_u, rem_v;
    // FillDouble(ref_u, "/ref_station/u");
    // FillDouble(ref_v, "/ref_station/v");
    // FillDouble(rem_u, "/rem_station/u");
    // FillDouble(rem_v, "/rem_station/v");
    //
    // double du = radians_to_arcsec * (rem_u - ref_u) / lambda;
    // double dv = radians_to_arcsec * (rem_v - ref_v) / lambda;
    // t202->u = (float) du;
    // t202->v = (float) dv;

    FillDouble(t202->u, "/fringe/du");
    FillDouble(t202->v, "/fringe/dv");
    // paramStore->Set("/fringe/du", du);
    // paramStore->Set("/fringe/dv", dv);

    return 0;
}

int MHO_MK4FringeExport::fill_203(struct type_203* t203)
{
    clear_203(t203);
    FillChannels(&(t203->channels[0]));
    return 0;
}

int MHO_MK4FringeExport::fill_204(struct type_204* t204)
{
    clear_204(t204);

    //use major and minor version (no patch version)
    t204->ff_version[0] = HOPS_VERSION_MAJOR;
    t204->ff_version[1] = HOPS_VERSION_MINOR;

    std::string tmp = "";
    char* env = secure_getenv("HOPS_ARCH");
    if(env != nullptr)
    {
        tmp = env;
    }
    char_clear(&(t204->platform[0]), 8);
    strncpy(&(t204->platform[0]), tmp.c_str(), std::min(8, (int)tmp.size()));

    FillString(&(t204->control_file[0]), "/files/control_file", 96);

    /* Look up modification date */
    struct stat buf;
    if(stat(t204->control_file, &buf) == 0)
    {
        struct tm* mod_time;
        mod_time = gmtime(&(buf.st_mtime));
        t204->ffcf_date.year = mod_time->tm_year + 1900;
        t204->ffcf_date.day = mod_time->tm_yday + 1;
        t204->ffcf_date.hour = mod_time->tm_hour;
        t204->ffcf_date.minute = mod_time->tm_min % 100;
        t204->ffcf_date.second = mod_time->tm_sec % 100;
    }

    //this is the text after  the (optional) 'set' keyword on the command line
    std::string set_string;
    bool ok = fPStore->Get("/cmdline/set_string", set_string);
    if(!ok)
    {
        set_string = "";
    }
    char_clear(&(t204->override[0]), 128);
    strncpy(&(t204->override[0]), set_string.c_str(), std::min(128, (int)set_string.size()));

    return 0;
}

int MHO_MK4FringeExport::fill_205(struct type_203* t203, struct type_205* t205)
{
    bool ok;
    clear_205(t205);

    /* For now, UTC central is same as FRT */
    FillDate(&(t205->utc_central), "/vex/scan/fourfit_reftime");
    t205->offset = 0.0;

    //ffmode and filter are not used/populated, so we skip them too

    //fill out the search windows used
    std::vector< double > sb_win;
    std::vector< double > dr_win;
    std::vector< double > mb_win;

    ok = fPStore->Get("/fringe/sb_win", sb_win);
    if(!ok)
    {
        sb_win.resize(2, 0.0);
    }
    ok = fPStore->Get("/fringe/dr_win", dr_win);
    if(!ok)
    {
        dr_win.resize(2, 0.0);
    }
    ok = fPStore->Get("/fringe/mb_win", mb_win);
    if(!ok)
    {
        mb_win.resize(2, 0.0);
    }

    t205->search[0] = sb_win[0];
    t205->search[1] = sb_win[1];
    t205->search[2] = dr_win[0];
    t205->search[3] = dr_win[1];
    t205->search[4] = mb_win[0];
    t205->search[5] = mb_win[1];

    FillDate(&(t205->start), "/fringe/start_date");
    FillDate(&(t205->stop), "/fringe/stop_date");

    //fill the ref freq
    FillDouble(t205->ref_freq, "/control/config/ref_freq");

    int nchan;
    FillInt(nchan, "/config/nchannels", 0);
    nchan = std::min(MAX_CHAN, nchan);
    std::vector< std::string > ch_labels;
    ok = fPlotData.Get("/PLOT_INFO/#Ch", ch_labels);
    if(ok && nchan > 0 && nchan < ch_labels.size())
    {
        for(int i = 0; i < nchan; i++)
        {
            t205->ffit_chan[i].ffit_chan_id = ch_labels[i][0];
            //this element (array 0-3) is effectively useless (we have no type_101 records to reference)
            t205->ffit_chan[i].channels[0] = (short)i;
        }
    }
    
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
    //             msg ("Could not find index number %d in type 203 record", 
    //                                             2, fc->index[ind]);
    //             return (-1);
    //             }
    //         if (nch >= 4)
    //             {
    //             msg ("Error - more than 4 correlator indices in ffit chan '%c'", 
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

int MHO_MK4FringeExport::fill_206(struct type_206* t206)
{
    clear_206(t206);

    //this is the same as the type 205 start parameter as far as I can tell...
    FillDate(&(t206->start), "/fringe/start_date");

    double first_ap;
    double last_ap;
    double ap_period;
    FillDouble(ap_period, "/config/ap_period");
    FillDouble(first_ap, "/start_offset");
    FillDouble(last_ap, "/stop_offset");
    short first = first_ap / ap_period;
    short last = last_ap / ap_period;
    t206->first_ap = first;
    t206->last_ap = last;

    FillFloat(t206->intg_time, "/fringe/integration_time");
    FillShort(t206->ratesize, "/fringe/n_drsp_points");
    FillShort(t206->mbdsize, "/fringe/n_mbd_points");

    //do not use: "/fringe/n_sbd_points", instead follow recipe from fill_206
    //this is due to fact that we drop every-other-point directly after the FFT
    //while HOPS3 removes them at a later stage
    int nlags = fPStore->GetAs< int >("/config/nlags");
    t206->sbdsize = (short)4 * nlags;

    visibility_type* vis_data = fCStore->GetObject< visibility_type >(std::string("vis"));
    if(vis_data == nullptr)
    {
        msg_error("fringe", "could not find visibility object with name: vis." << eom);
        return 1;
    }

    auto chan_ax = &(std::get< CHANNEL_AXIS >(*vis_data));
    std::size_t nchannels = chan_ax->GetSize();

    double acc_period;
    double samp_period;
    bool ok = fPStore->Get("/config/ap_period", acc_period);
    ok = fPStore->Get("/vex/scan/sample_period/value", samp_period);

    //TODO FIXME ...THESE ARE DUMMY values
    // we need to check these values, for now we are treating these values
    // as if there are no per-channel/per-ap data edits!
    // struct sidebands    accepted[64];           /* APs accepted by chan/sband */
    // struct sbweights    weights[64];            /* Samples per channel/sideband */
    // float               accept_ratio;           /* % ratio min/max data accepted */
    // float               discard;                /* % data discarded */
    double samp_per_ap = acc_period / samp_period;
    for(std::size_t fr = 0; fr < nchannels; fr++)
    {
        std::string sb;
        chan_ax->RetrieveIndexLabelKeyValue(fr, "net_sideband", sb);
        double usb = 0.;
        double lsb = 0.;
        if(sb == "U")
        {
            usb = 1.0;
            lsb = 0.0;
        }
        if(sb == "L")
        {
            usb = 0.0;
            lsb = 1.0;
        }
        t206->accepted[fr].usb = usb * (last - first);
        t206->accepted[fr].lsb = lsb * (last - first);
        //NOTE: the use of integration time here ignores individual channel edits!
        t206->weights[fr].usb = t206->intg_time * (usb * samp_per_ap);
        t206->weights[fr].lsb = t206->intg_time * (lsb * samp_per_ap);
    }
    //ignore cuts ...fake/dummy values
    t206->accept_ratio = 100;
    t206->discard = 0.0;

    return 0;
}

int MHO_MK4FringeExport::fill_207(struct type_207* t207)
{
    //TODO FIXME implement this
    clear_207(t207);
    return 0;
}

int MHO_MK4FringeExport::fill_208(struct type_202* t202, struct type_208* t208)
{
    bool ok;
    clear_208(t208);

    std::string qcode;
    ok = fPStore->Get("/fringe/quality_code", qcode);
    if(!ok)
    {
        qcode = "0";
    }
    t208->quality = qcode[0];

    std::string errcode;
    ok = fPStore->Get("/fringe/error_code", errcode);
    if(!ok)
    {
        errcode = " ";
    };
    t208->errcode = errcode[0];

//don't forget the "provisional" pol product indicator
#define POLCHAR_OFFSET 64
// polarization values in pass array structure
#define POL_LL 0
#define POL_RR 1
#define POL_LR 2
#define POL_RL 3
// polarization values in param array structure
#define POL_ALL 0
#define POLMASK_LL 1
#define POLMASK_RR 2
#define POLMASK_LR 4
#define POLMASK_RL 8
#define POL_IXY 31

    char passpol;
    char parampol;
    std::string polprod = fPStore->GetAs< std::string >("/config/polprod");
    if(polprod == "I")
    {
        parampol = POL_IXY;
    }
    if(polprod == "XX")
    {
        parampol = POLMASK_LL;
        passpol = POL_LL;
    }
    if(polprod == "XY")
    {
        parampol = POLMASK_LR;
        passpol = POL_LR;
    }
    if(polprod == "YX")
    {
        parampol = POLMASK_RL;
        passpol = POL_RL;
    }
    if(polprod == "YY")
    {
        parampol = POLMASK_RR;
        passpol = POL_RR;
    }
    if(polprod == "LL")
    {
        parampol = POLMASK_LL;
        passpol = POL_LL;
    }
    if(polprod == "LR")
    {
        parampol = POLMASK_LR;
        passpol = POL_LR;
    }
    if(polprod == "RL")
    {
        parampol = POLMASK_RL;
        passpol = POL_RL;
    }
    if(polprod == "RR")
    {
        parampol = POLMASK_RR;
        passpol = POL_RR;
    }

    t208->unused1[0] = passpol + POLCHAR_OFFSET;
    t208->unused1[1] = parampol + POLCHAR_OFFSET;
    t208->unused1[2] = '\0';

    //not used
    strncpy(t208->tape_qcode, "99999?", 6);

    FillDouble(t208->adelay, "/model/adelay");
    FillDouble(t208->arate, "/model/arate");
    FillDouble(t208->aaccel, "/model/aaccel");

    FillDouble(t208->tot_mbd, "/fringe/total_mbdelay");
    FillDouble(t208->tot_sbd, "/fringe/total_sbdelay");
    FillDouble(t208->tot_rate, "/fringe/total_drate");

    TODO_FIXME_MSG("TODO FIXME -- the totals for the reference station are not yet calculated")
    FillDouble(t208->tot_mbd_ref, "/fringe/total_mbdelay_ref");
    FillDouble(t208->tot_sbd_ref, "/fringe/total_sbdelay_ref");
    FillDouble(t208->tot_rate_ref, "/fringe/total_rate_ref");
    FillFloat(t208->totphase_ref, "/fringe/tot_phase_ref"); //DOES NOT EXIST YET

    FillFloat(t208->resid_mbd, "/fringe/mbdelay");
    FillFloat(t208->resid_sbd, "/fringe/sbdelay");
    FillFloat(t208->resid_rate, "/fringe/drate");
    FillFloat(t208->mbd_error, "/fringe/mbd_error");
    FillFloat(t208->sbd_error, "/fringe/sbd_error");

    FillFloat(t208->rate_error, "/fringe/drate_error");
    FillFloat(t208->ambiguity, "/fringe/ambiguity");
    FillFloat(t208->amplitude, "/fringe/famp");
    t208->amplitude /= 10000.0; //remove Whitneys prefactor

    double inc_avg_amp;
    double inc_avg_amp_freq;
    ok = fPlotData.Get("/extra/inc_avg_amp", inc_avg_amp);
    if(!ok)
    {
        inc_avg_amp = 0.0;
    }
    ok = fPlotData.Get("extra/inc_avg_amp_freq", inc_avg_amp_freq);
    if(!ok)
    {
        inc_avg_amp_freq = 0.0;
    }
    t208->inc_seg_ampl = inc_avg_amp;
    t208->inc_chan_ampl = inc_avg_amp_freq;

    FillFloat(t208->snr, "/fringe/snr");
    FillFloat(t208->prob_false, "/fringe/pfd");
    FillFloat(t208->totphase, "/fringe/tot_phase");
    FillFloat(t208->resphase, "/fringe/resid_phase");
    FillFloat(t208->tec_error, "/fringe/dtec_error");

    return 0;
}

int MHO_MK4FringeExport::fill_210(struct type_210* t210)
{
    clear_210(t210);
    bool ok1, ok2;
    std::vector< double > ch_amp;
    std::vector< double > ch_phase;
    int nchan;

    FillInt(nchan, "/config/nchannels", 0);
    ok1 = fPlotData.Get("/PLOT_INFO/Ampl", ch_amp);
    ok2 = fPlotData.Get("/PLOT_INFO/Phase", ch_phase);
    if(ok1 && ok2 && nchan > 0)
    {
        nchan = std::min(MAX_CHAN, nchan);
        for(int i = 0; i < nchan; i++)
        {
            t210->amp_phas[i].ampl = ch_amp[i] / 10000.0; //remove Whitneys prefactor
            t210->amp_phas[i].phase = ch_phase[i];        //already in degrees
        }
    }

    return 0;
}

int MHO_MK4FringeExport::fill_212(int fr, struct type_212* t212)
{
    clear_212(t212);

    int nap = fPStore->GetAs< int >("/config/total_naps");
    t212->nap = nap;
    t212->first_ap = 0; //pass->ap_off;
    t212->channel = fr;
    t212->sbd_chan = fPStore->GetAs< int >("/fringe/max_sbd_bin"); //status->max_delchan;

    //retrieve the 'phasor' object from the container store
    auto phasor_data = fCStore->GetObject< phasor_type >(std::string("phasors"));
    auto wt_data = fCStore->GetObject< weight_type >(std::string("weight"));
    if(phasor_data != nullptr && wt_data != nullptr)
    {
        std::size_t p_nchan = phasor_data->GetDimension(0);
        std::size_t p_nap = phasor_data->GetDimension(1);
        std::size_t w_nchan = wt_data->GetDimension(CHANNEL_AXIS);
        std::size_t w_nap = wt_data->GetDimension(TIME_AXIS);

        for(int ap = 0; ap < nap; ap++)
        {
            std::complex< double > pvalue = 0;
            double wvalue = 0.0;
            if(fr < p_nchan && ap < p_nap && fr < w_nchan && ap < w_nap)
            {
                pvalue = phasor_data->at(fr, ap);
                wvalue = wt_data->at(0, fr, ap, 0);
                t212->data[ap].amp = std::abs(pvalue);
                t212->data[ap].phase = std::arg(pvalue);
                t212->data[ap].weight = wvalue;
            }
            else
            {
                t212->data[ap].amp = -1.0;
                t212->data[ap].phase = 0.0;
                t212->data[ap].weight = 0.0;
            }
        }
    }
    else
    {
        msg_warn("mk4_interface", "could not retrieve phasor data for channel "
                                      << fr << ", type_212's will be populated with dummy data" << eom);
        for(int ap = 0; ap < nap; ap++)
        {
            t212->data[ap].amp = -1.0;
            t212->data[ap].phase = 0.0;
            t212->data[ap].weight = 0.0;
        }
    }

    return 0;
}

int MHO_MK4FringeExport::fill_222(struct type_222** t222)
{
    std::string control_contents;
    bool ok = fPStore->Get("/control/control_file_contents", control_contents);
    if(!ok)
    {
        control_contents = "";
    }

    unsigned char set_string_buff[1] = {' '};
    int setstr_len, cf_len, setstr_pad, cf_pad, full_size, i;
    unsigned int setstr_hash = 0;
    unsigned int cf_hash = 0;

    //now allocate the necessary amount of memory
    setstr_len = 0; //nothing, we've packed this into the control contents
    cf_len = control_contents.size();

    //find next largest multiple of 8 bytes
    setstr_pad = ((setstr_len + 7) & ~7) + 8;
    cf_pad = ((cf_len + 7) & ~7) + 8;
    full_size = sizeof(struct type_222) + setstr_pad + cf_pad;

    char* temp_buf = new char[cf_pad];
    for(std::size_t j = 0; j < cf_pad; j++)
    {
        if(j < cf_len)
        {
            temp_buf[j] = control_contents[j];
        }
        else
        {
            temp_buf[j] = '\0';
        }
    }

    /* Allocate space for output record */
    *t222 = (struct type_222*)malloc(full_size);
    if(*t222 == NULL)
    {
        msg_error("mk4interface", "memory allocation failure in fill_222" << eom);
        return -1;
    }

    //figure out the stop/stop of the cf leading/trailing white space:
    //this is probably uncessary since the new parser strips this already
    //but legacy type_222 sometimes has an extra space at front or back
    int j;
    int cf_start = 0;
    int cf_stop = cf_len;
    for(j = 0; j < cf_len; j++)
    {
        cf_start = j;
        if((temp_buf[j] != ' ') && (temp_buf[j] != '\t') && (temp_buf[j] != '\n'))
        {
            break;
        }
    }
    for(j = cf_len - 1; j >= 0; j--)
    {
        cf_stop = j;
        if((temp_buf[j] != ' ') && (temp_buf[j] != '\t') && (temp_buf[j] != '\n'))
        {
            break;
        }
    }

    //now do the hashing
    setstr_hash = adler32_checksum((unsigned char*)&(set_string_buff[0]), setstr_len);
    cf_hash = adler32_checksum((unsigned char*)&(temp_buf[cf_start]), cf_stop - cf_start);

    strncpy((*t222)->record_id, "222", 3);
    strncpy((*t222)->version_no, "00", 2);
    (*t222)->unused1 = ' ';
    (*t222)->padded = 0;
    (*t222)->setstring_hash = setstr_hash;
    (*t222)->control_hash = cf_hash;
    (*t222)->setstring_length = setstr_len;
    (*t222)->cf_length = cf_len;

    //don't bother with set string stuff
    for(i = 0; i < setstr_pad; i++)
    {
        ((*t222)->control_contents)[i] = ' ';
    }

    for(std::size_t j = 0; j < cf_len; j++)
    {
        ((*t222)->control_contents)[j + setstr_pad] = temp_buf[j];
    }
    for(i = setstr_pad + cf_len; i < setstr_pad + cf_pad; i++)
    {
        ((*t222)->control_contents)[i] = '\0';
    }

    delete[] temp_buf;

    return 0;
}

int MHO_MK4FringeExport::fill_230(int fr, int ap, struct type_230* t230)
{
    //TODO FIXME implement this
    clear_230(t230);
    return 0;
}

//dummy, just clears the structure
int MHO_MK4FringeExport::fill_221(struct type_221** t221)
{
    struct stat file_status;
    int fd;
    size_t nb, size, filesize;
    FILE* fp;
    char *pplot, *end;

    double tickinc;
    //load the dummy ps file
    std::string ps_file;
    ps_file += HOPS_MK4AUX_DIR;
    ps_file += "/mk4aux/blank.ps";
    fp = fopen(ps_file.c_str(), "r");

    if((fd = fileno(fp)) < 0)
    {
        return -1;
    }
    if(fstat(fd, &file_status) != 0)
    {
        return -2;
    }

    filesize = file_status.st_size;

    size = filesize + (size_t)512; //don't really need this much extra space
    if((*t221 = (struct type_221*)malloc(size)) == NULL)
    {
        return -3;
    }

    clear_221(*t221);
    rewind(fp);
    pplot = (*t221)->pplot;

    nb = fread(pplot, sizeof(char), filesize, fp);
    pplot[filesize] = '\0';
    if(nb != filesize)
    {
        return -4;
    }
    fclose(fp);

    if((end = strstr(pplot, "EOF\n")) != NULL)
    {
        *(end + 4) = '\0';
    }
    (*t221)->ps_length = strlen(pplot);

    return 0;
}

int MHO_MK4FringeExport::output()
{
    //set up the write lock mechanism
    //TODO -- allow for a different directory for output than the input directory
    std::string directory = fPStore->GetAs< std::string >("/files/directory");
    directory = MHO_DirectoryInterface::GetDirectoryFullPath(directory);

    // for locking
    int lock_retval = LOCK_PROCESS_NO_PRIORITY;
    char lockfile_name[512] = {'\0'};
    char fringe_name[256];
    char_clear(fringe_name, 256);
    int the_seq_no; //the fringe file sequence number

    //declare the fringe structure and items we are going to fill on the stack
    struct mk4_fringe fringe;
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

    //wait until we are the next process allowed to write an output file
    MHO_LockFileHandler::GetInstance().EnableLegacyMode();
    lock_retval = MHO_LockFileHandler::GetInstance().WaitForWriteLock(directory, the_seq_no);

    if(lock_retval == LOCK_STATUS_OK && the_seq_no > 0)
    {
        //construct the fringe filename
        std::string filename = CreateFringeFileName(directory, the_seq_no);

        if(filename.size() > 256)
        {
            msg_error("mk4interface", "filename exceeds max length of 256, fringe_name will be truncated" << eom);
        }
        strncpy(fringe_name, filename.c_str(), std::min(255, (int)filename.size()));

        //some of these are unused...leave as for now
        int error, nap, xpow_len, fr, ap, size_of_t212, size_of_t230, recno;
        char buf[256];
        char_clear(buf, 256);
        char *t212_array, *t230_array, *address;

        fringe.nalloc = 0;
        clear_mk4fringe(&fringe);

        strncpy(buf, filename.c_str(), std::min(255, (int)filename.size()));
        int val = init_000(&t2_id, fringe_name);
        if(val != 0)
        {
            msg_fatal("mk4interface", "failed to init type 000, error due to filename: " << filename << " ?" << eom);
            return (-1);
        }

        //fill the data structures
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

        //point the fringe to the data structures
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

        // Type 212 (ap-by-ap data) records
        // Allocate memory as a block
        nap = fPStore->GetAs< int >("/config/total_naps");
        int nfreq = fPStore->GetAs< int >("/config/nchannels");

        size_of_t212 = sizeof(struct type_212) + 12 * (nap - 1);
        if((nap % 2) == 1)
            size_of_t212 += 12;
        t212_array = (char*)malloc(nfreq * size_of_t212);
        if(t212_array == NULL)
        {
            msg_error("mk4interface", "error allocating memory for type_212 record." << eom);
            return 0;
        }

        //record the allocation
        fringe.allocated[fringe.nalloc] = t212_array;
        fringe.nalloc += 1;

        //Fill in records and pointers
        fringe.n212 = nfreq;
        for(fr = 0; fr < nfreq; fr++)
        {
            address = t212_array + (fr * size_of_t212);
            fringe.t212[fr] = (struct type_212*)address;
            error += fill_212(fr, fringe.t212[fr]);
        }

        //TODO FIXME implement type_230
        fringe.n230 = 0;

        struct type_221* t221;
        if(fill_221(&t221) != 0)
        {
            msg_error("mk4interface", "error filling dummy postscript record" << eom);
            return 1;
        }
        fringe.t221 = t221;
        fringe.t221->ps_length = strlen(fringe.t221->pplot);
        fringe.allocated[fringe.nalloc] = fringe.t221;
        fringe.nalloc += 1;

        /* Fill in the control file record */
        fringe.t222 = NULL;
        struct type_222* t222;
        if(fill_222(&t222) != 0)
        {
            msg_error("mk4interface", "error filling control file record" << eom);
            return 1;
        }
        fringe.t222 = t222;
        fringe.allocated[fringe.nalloc] = fringe.t222;
        fringe.nalloc += 1;

        //kludge to get fourfit to feed the generated fringe file name
        //(but nothing else) as a return value to a
        //a python calling script (requires passing option "-m 4"); see
        //e.g. chops/source/python_src/hopstest_module/hopstestb/hopstestb.py
        //around line 74 in the FourFitThread class.
        auto msglev = MHO_Message::GetInstance().GetMessageLevel();
        if(msglev == eSpecial)
        {
            fprintf(stderr, "fourfit: %s \n", fringe_name);
        }

        int write_nbytes = write_mk4fringe(&fringe, fringe_name);
        //pause 5ms, if a lock file was created, delete it now
        usleep(5000);
        MHO_LockFileHandler::GetInstance().RemoveWriteLock();
        if(write_nbytes <= 0)
        {
            msg_error("mk4interface", "error writing fringe file, mk4 code: " << write_nbytes << "." << eom);
            return 1;
        }
    }
    else
    {
        msg_error("mk4interface", "could not obtain write lock for directory: " << directory << eom);
    }

    return 0;
}

int MHO_MK4FringeExport::convert_sky_coords(struct sky_coord& coords, std::string ra, std::string dec)
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
    if(tokens.size() != 2)
    {
        msg_error("mk4interface", "error parsing dec string: " << dec << " with delimiter" << delim1 << eom);
        return 1;
    }

    tmp = tokens[0];
    dec_degs = std::atoi(tmp.c_str());
    tmp = tokens[1];

    fTokenizer.SetDelimiter(delim2);
    fTokenizer.SetString(&tmp);
    fTokenizer.GetTokens(&tokens);
    if(tokens.size() != 2)
    {
        msg_error("mk4interface", "error parsing dec string: " << dec << " with delimiter" << delim2 << eom);
        return 1;
    }

    tmp = tokens[0];
    dec_mins = std::atoi(tmp.c_str());
    tmp = tokens[1];

    std::size_t last = tokens[1].find_first_not_of("0123456789.e+-");
    tmp = tokens[1].substr(0, last);
    dec_secs = std::atof(tmp.c_str());

    fTokenizer.SetDelimiter(delim3);
    fTokenizer.SetString(&ra);
    fTokenizer.GetTokens(&tokens);
    if(tokens.size() != 2)
    {
        msg_error("mk4interface", "error parsing ra string: " << ra << " with delimiter" << delim3 << eom);
        return 1;
    }

    tmp = tokens[0];
    ra_hrs = std::atoi(tmp.c_str());
    tmp = tokens[1];

    fTokenizer.SetDelimiter(delim4);
    fTokenizer.SetString(&tmp);
    fTokenizer.GetTokens(&tokens);
    if(tokens.size() != 2)
    {
        msg_error("mk4interface", "error parsing ra string: " << ra << " with delimiter" << delim4 << eom);
        return 1;
    }

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

void MHO_MK4FringeExport::FillString(char* destination, std::string param_path, int max_length, std::string default_value)
{
    char_clear(destination, max_length);
    std::string tmp;
    bool ok = fPStore->Get(param_path, tmp);
    if(!ok)
    {
        tmp = default_value;
    }
    strncpy(destination, tmp.c_str(), std::min(max_length, (int)tmp.size()));
}

void MHO_MK4FringeExport::FillString(std::string& destination, std::string param_path, std::string default_value)
{
    std::string tmp;
    bool ok = fPStore->Get(param_path, tmp);
    if(!ok)
    {
        tmp = default_value;
    }
    destination = tmp;
}

void MHO_MK4FringeExport::FillInt(int& destination, std::string param_path, int default_value)
{
    int value;
    bool ok = fPStore->Get(param_path, value);
    if(!ok)
    {
        value = default_value;
    }
    destination = value;
}

void MHO_MK4FringeExport::FillShort(short& destination, std::string param_path, int default_value)
{
    int value;
    bool ok = fPStore->Get(param_path, value);
    if(!ok)
    {
        value = default_value;
    }
    destination = (short)value;
}

void MHO_MK4FringeExport::FillDouble(double& destination, std::string param_path, double default_value)
{
    double value;
    bool ok = fPStore->Get(param_path, value);
    if(!ok)
    {
        value = default_value;
    }
    destination = value;
}

void MHO_MK4FringeExport::FillFloat(float& destination, std::string param_path, float default_value)
{
    double value;
    bool ok = fPStore->Get(param_path, value); //all params are stored as doubles
    if(!ok)
    {
        value = default_value;
    }
    destination = value;
}

void MHO_MK4FringeExport::FillDate(struct date* destination, std::string param_path)
{
    legacy_hops_date a_date;
    std::string date_vex_string;
    bool ok = fPStore->Get(param_path, date_vex_string);
    if(ok)
    {
        a_date = MHO_LegacyDateConverter::ConvertFromVexFormat(date_vex_string);
    }
    else
    {
        a_date = MHO_LegacyDateConverter::HopsEpoch();
    } //dummy date
    destination->year = a_date.year;
    destination->day = a_date.day;
    destination->hour = a_date.hour;
    destination->minute = a_date.minute;
    destination->second = a_date.second;
}

void MHO_MK4FringeExport::FillDate(struct date* destination, struct legacy_hops_date& a_date)
{
    destination->year = a_date.year;
    destination->day = a_date.day;
    destination->hour = a_date.hour;
    destination->minute = a_date.minute;
    destination->second = a_date.second;
}

void MHO_MK4FringeExport::FillChannels(struct ch_struct* chan_array)
{
    visibility_type* vis_data = fCStore->GetObject< visibility_type >(std::string("vis"));
    if(vis_data == nullptr)
    {
        msg_fatal("fringe", "could not find visibility object with name: vis, will not fill channel info" << eom);
        return; //bail out
    }

    auto chan_ax = &(std::get< CHANNEL_AXIS >(*vis_data));
    std::size_t nchannels = chan_ax->GetSize();

    std::vector< std::string > polprod_set;
    bool ok = fPStore->Get("/config/polprod_set", polprod_set);

    //limit to supported number of channels
    std::size_t max_chan_records = 8 * 64; //8*MAXFREQ
    std::size_t counter = 0;

    for(std::size_t ppi = 0; ppi < polprod_set.size(); ppi++)
    {
        std::string polprod = polprod_set.at(ppi);

        char refpol = ' ';
        char rempol = ' ';
        if(ok && polprod.size() == 2)
        {
            refpol = polprod[0];
            rempol = polprod[1];
        }

        if(ok && polprod.size() == 1)
        {
            refpol = polprod[0];
            rempol = polprod[0];
        }

        for(std::size_t ch = 0; ch < nchannels; ch++)
        {
            int findex = counter;
            double bandwidth = 0;
            short index = 0;
            unsigned short int sample_rate = 0;
            std::string refsb = "";
            std::string remsb = "";

            double ref_freq = 0;
            double rem_freq = 0;
            std::string ref_chan_id = "";
            std::string rem_chan_id = "";
            std::string temp_chan_id = "";

            chan_ax->RetrieveIndexLabelKeyValue(ch, "index", findex);
            chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", refsb);
            chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", remsb);
            chan_ax->RetrieveIndexLabelKeyValue(ch, "sky_freq", ref_freq);
            chan_ax->RetrieveIndexLabelKeyValue(ch, "sky_freq", rem_freq);
            chan_ax->RetrieveIndexLabelKeyValue(ch, "bandwidth", bandwidth);
            chan_ax->RetrieveIndexLabelKeyValue(ch, "mk4_channel_id", temp_chan_id);
            //chan_ax->RetrieveIndexLabelKeyValue(ch, "chan_id", ref_chan_id);

            //split the temporary-channel id on the ":" character
            std::vector< std::string > tokens;
            fTokenizer.SetUseMulticharacterDelimiterFalse();
            fTokenizer.SetRemoveLeadingTrailingWhitespaceTrue();
            fTokenizer.SetIncludeEmptyTokensFalse();

            fTokenizer.SetDelimiter(":");
            fTokenizer.SetString(&temp_chan_id);
            fTokenizer.GetTokens(&tokens);

            //these are dummy channel ids
            if(tokens.size() == 2)
            {
                ref_chan_id = tokens[0];
                rem_chan_id = tokens[1];
                //replace the last character with the pol-label from the polprod
                ref_chan_id.back() = refpol;
                rem_chan_id.back() = rempol;
            }

            index = (short)findex;
            sample_rate = (unsigned short int)(2.0 * bandwidth * 1000.0); //sample rate = 2 x bandwidth (MHz) x (1000KHz/MHz)

            chan_array[ch].index = index;
            chan_array[ch].sample_rate = sample_rate;
            chan_array[ch].refsb = refsb[0];
            chan_array[ch].remsb = remsb[0];
            chan_array[ch].refpol = refpol;
            chan_array[ch].rempol = rempol;
            chan_array[ch].ref_freq = ref_freq * 1e6; //convert to Hz
            chan_array[ch].rem_freq = rem_freq * 1e6; //convert to Hz
            char_clear(&(chan_array[ch].ref_chan_id[0]), 8);
            char_clear(&(chan_array[ch].rem_chan_id[0]), 8);
            strncpy(&(chan_array[ch].ref_chan_id[0]), ref_chan_id.c_str(), std::min(7, (int)ref_chan_id.size()));
            strncpy(&(chan_array[ch].rem_chan_id[0]), rem_chan_id.c_str(), std::min(7, (int)rem_chan_id.size()));

            counter++;

            if(counter >= max_chan_records)
            {
                msg_warn("mk4interface", "too many channel records, (" << counter << "), to export to type_203, truncating to " << max_chan_records << eom);
                break;
            }
        }
    }
}

std::string MHO_MK4FringeExport::CreateFringeFileName(std::string directory, int seq_no)
{
    //grab the name info
    std::string baseline;
    std::string root_code;
    std::string frequency_group;

    FillString(baseline, "/config/baseline", "??");
    FillString(root_code, "/config/root_code", "XXXXXX");
    FillString(frequency_group, "/config/frequency_group", "X");

    std::stringstream ss;
    ss << directory << "/" << baseline << "." << frequency_group << "." << seq_no << "." << root_code;
    return ss.str();
}

} // namespace hops
