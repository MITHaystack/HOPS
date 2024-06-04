#include "MHO_EstimatePCManual.hh"

/*
 * If invoked, estimate ph phase and
 * delay values to use in manual mode.
 * pc_phase_? and delay_offs_? values report.
 *
 * This is adapted from fearfit test code. gbc 5/8/2017
 */

#include <cmath>
#include <string>
#include <map>
#include <vector>
#include <utility>
#include <algorithm>
#include <cctype>


#include "MHO_Constants.hh"

#define MAXFREQ 64

namespace hops
{


//the ordering operator for channel labels to sort by frequency;
class freq_predicate
{
    public:
        freq_predicate(){};
        virtual ~freq_predicate(){};

    virtual bool operator()(const std::pair<std::string, double>& a, const std::pair<std::string, double>& b)
    {
        return a.second < b.second;
    }
};

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

//
/* move phase to principal branch */
static double pbranch(double phase)
{
    phase = ( fmod(phase, 360.0) );
    if (phase < -180.0){ phase += 360.0; }
    if (phase >  180.0){ phase -= 360.0; }
    return phase;
}

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
// static
void
MHO_EstimatePCManual::est_phases(int rr, int keep)
{

    //static char buf[720], tmp[80], *pb;
    // int ch, ss, pol, nd;
    int nd;
    // double inp_phase, est_phase, sbmult, delta_delay, phase_bias;
    char *epb = getenv("HOPS_EST_PC_BIAS");
    char *epd = getenv("HOPS_EST_PC_DLYM");
    //
    // // *progname = 0;
    // // msg("*est: phases on %s station", 1, rr ? "ref" : "rem");
    //
    /* support for bias operation */
    double phase_bias;
    if(keep)
    {
        phase_bias = (epb) ? atof(epb) : 0.0;
        //msg("*est: phase bias %f (mod res phase is %f)", 3, phase_bias, status.coh_avg_phase * (180.0 / M_PI));
    }
    // if (epb || epd)
    //     msg("*est: HOPS_EST_PC_BIAS %s ..._DLYM %s", 3, epb, epd);
    //

    // // /* header for the section */
    // pol = pol_letter(pass->pol, !rr);
    // sprintf(buf, "if station %c\n pc_phases_%c ",fringe.t202->baseline[!rr], pol);
    // for (ch = first, pb = buf + strlen(buf); ch <= final; ch++, pb++)
    // {
    //     *pb = pass->pass_data[ch].freq_code;
    // }
    // *pb = 0;
    // msg(buf, 3);

    double ref_freq = fParameterStore->GetAs<double>(std::string("/control/config/ref_freq"));

    //construct the control file line prefix
    std::string station_id = "?";
    if(rr){station_id = fRefStationMk4ID;}
    else{station_id = fRemStationMk4ID;}

    std::string pol = "?";
    if(rr){pol = fRefStationPol;}
    else{pol = fRemStationPol;}

    std::string cf_line = "if station " + station_id + "\n" + "pc_phases_" + pol + " ";

    std::map<std::string, double> chan2resid;
    std::map<std::string, double> chan2freq;
    std::vector< std::pair<std::string, double> > ch_freq_pairs;
    std::string chan_label_key = "channel_label";
    std::string sidebandlabelkey = "net_sideband";

    //calculate the average fPhasors
    std::size_t nchan = (&std::get<0>(*fPhasors))->GetSize(); //"what about 'All' channel?"...we just skip it
    std::size_t naps = (&std::get<1>(*fPhasors))->GetSize();

    std::size_t wchan = fWeights->GetDimension(CHANNEL_AXIS);
    std::size_t wnaps = fWeights->GetDimension(TIME_AXIS);
    std::size_t POLPROD_INDEX = 0;

    for(std::size_t i=0; i<nchan; i++) //loop over channels
    {
        //grab the channel label
        std::string ch_label;
        (&std::get<0>(*fPhasors))->RetrieveIndexLabelKeyValue(i, chan_label_key, ch_label);

        if(ch_label != "All")
        {
            double freq = std::get<0>(*fPhasors).at(i);//sky freq of this channel
            std::complex<double> phsum = 0;
            double wsum = 0;
            double w;
            for(std::size_t j=0; j<naps; j++) //sum over APs
            {
                phsum += (*fPhasors)(i,j);
                w = 1.0;
                if(i<wchan && j<wnaps)
                {
                    w = (*fWeights)(POLPROD_INDEX,i,j,0);
                }
                wsum += w;
            }
            phsum *= 1.0/(double)wsum;
            double ch_arg = std::arg(phsum);
            chan2resid[ch_label] = ch_arg;
            chan2freq[ch_label] = freq;
            ch_freq_pairs.push_back( std::make_pair(ch_label, freq) );
        }
    }

    //construct the frequency ordered list of channels
    freq_predicate fpred;
    std::sort(ch_freq_pairs.begin(), ch_freq_pairs.end(), fpred);
    std::string concat_ch;
    for(std::size_t i=0; i<ch_freq_pairs.size();i++)
    {
        concat_ch += ch_freq_pairs[i].first;
    }

    //loop over the channels
    std::string output_string = "";
    for(auto it = ch_freq_pairs.begin(); it != ch_freq_pairs.end(); it++)
    {
        std::string ch = it->first;
        double ch_freq = it->second;
        double ch_resid_phase = chan2resid[ch];

        //figure out the channel index and grab the sideband info
        //as well as what manual pcal has been applied
        std::size_t ch_idx;
        std::vector< std::size_t > ch_idx_vec = std::get<CHANNEL_AXIS>(*fVisibilities).GetMatchingIndexes(chan_label_key, ch);
        if(ch_idx_vec.size() == 1)
        {
            ch_idx = ch_idx_vec[0];
            std::string net_sideband = "?";
            bool key_present = std::get<CHANNEL_AXIS>(*fVisibilities).RetrieveIndexLabelKeyValue(ch_idx, sidebandlabelkey, net_sideband);
            if(!key_present){msg_error("calibration", "missing net_sideband label for channel "<< ch << "." << eom);}
            else
            {
                double sbmult = 0.0;
                if(net_sideband == "L"){sbmult = -1.0;}
                if(net_sideband == "U"){sbmult = 1.0;}

                //get the ref station manual pcal (if applied)
                double ref_pc = get_manual_phasecal(0, ch_idx, fRefStationPol); //0 indicates ref station
                //get the rem station manual pcal (if applied)
                double rem_pc = get_manual_phasecal(1, ch_idx, fRemStationPol); //1 indicates rem station

                double est_phase = ref_pc - rem_pc; //should already be in degrees
                double inp_phase = pbranch(est_phase);

                //
                // /* assume it is all usb or lsb for this estimate */
                // sbmult = (status.total_usb_frac > 0) ? 1.0 : -1.0;
                // est_phase = status.pc_phase[ch][0][stnpol[0][pass->pol]]
                //           - status.pc_phase[ch][1][stnpol[1][pass->pol]];
                // est_phase *= 180.0 / M_PI;  /* radians to degrees */
                // inp_phase = pbranch(est_phase);

                //get the residual delays
                 double resid_mbd = fParameterStore->GetAs<double>("/fringe/mbdelay");
                 double resid_sbd = fParameterStore->GetAs<double>("/fringe/sbdelay");

                //get the mbd_anchor method
                std::string mbd_anchor = fParameterStore->GetAs<std::string>("/control/config/mbd_anchor");
                double delta_delay = 0.0;
                if(mbd_anchor == "model")
                {
                    delta_delay = resid_mbd;
                }
                else if(mbd_anchor == "sbd")
                {
                    delta_delay = resid_mbd - resid_sbd;
                }

                if(epd){delta_delay *= std::atof(epd);}

                est_phase += sbmult*(ch_resid_phase*MHO_Constants::rad_to_deg) + 360.0*delta_delay*(ch_freq - ref_freq);

                // /* what we need to do to remove the multiband delay */
                // delta_delay = (param.mbd_anchor == MODEL)
                //             ? fringe.t208->resid_mbd
                //             : fringe.t208->resid_mbd - fringe.t208->resid_sbd;
                // /* allow this factor to be adjusted */
                // delta_delay *= (epd) ? atof(epd) : 1.0;
                // est_phase += sbmult * (arg_complex(status.fringe[ch]) * 180.0 / M_PI
                //           + 360.0 * delta_delay *
                //             (pass->pass_data[ch].frequency - fringe.t205->ref_freq));

                /* bias the phase calculation to preserve existing resid phase */
                if (keep) est_phase += phase_bias;

                /* canonicalize for comparision */
                est_phase = pbranch(est_phase);
                if (fabs(inp_phase - est_phase) > 0.01) nd ++;

                /* remove input phase values */
                if(rr){ inp_phase = rem_pc;} //status.pc_phase[ch][1][stnpol[1][pass->pol]];
                else{ inp_phase = -1.0*ref_pc;} //status.pc_phase[ch][0][stnpol[0][pass->pol]];

                est_phase += inp_phase;// * 180.0 / M_PI;

                if(rr){ est_phase = pbranch(est_phase); }
                else{ est_phase = pbranch(-est_phase); }

                //snprintf(tmp, sizeof(tmp), " %+8.3f", est_phase);
                // strncat(buf, tmp, sizeof(buf)-1);
                //
                // /* eight phases per line for a line length of 73 */
                // if (++ss == 8)
                // {
                //     // msg(buf, 3);
                //     buf[ss = 0] = 0;
                // }

                char tmp[80] = {0};
                snprintf(tmp, sizeof(tmp), " %+8.3f", est_phase);
                std::stringstream ss;
                ss << tmp;
                output_string += ss.str();
                if(ch_idx % 8 == 7){output_string +=  "\n";}
            }

        }
    }

    std::cout<<cf_line<<" "<<concat_ch<<"\n"<<output_string<<std::endl;

//     // if (buf[0]) msg(buf, 3);
//     // msg("*est: phases %s (%d)", 2, nd ? "converging" : "converged", nd);

}







void
MHO_EstimatePCManual::est_pc_manual(int mode)
{
    // int first_ch, final_ch;
    // int doref, dophs, dodly, dooff, domrp;
    std::cout<<" in est_pc_manual"<<std::endl;

    fParameterStore->Dump();

    std::string rootfile = fParameterStore->GetAs<std::string>("/files/root_file");

    std::string ref_id = fParameterStore->GetAs<std::string>("/ref_station/mk4id");
    std::string rem_id = fParameterStore->GetAs<std::string>("/rem_station/mk4id");
    fRefStationMk4ID = ref_id;
    fRemStationMk4ID = rem_id;

    std::string polprod = fParameterStore->GetAs<std::string>("/config/polprod");
    fRefStationPol = tolower(polprod[0]);
    fRefStationPol = tolower(polprod[1]);

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
    if(dophs){ est_phases(doref, domrp); }
    if(dodly){ est_delays(doref, dodly); }
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

    fVisibilities = in;
    est_pc_manual(1);
    return true;
}


double
MHO_EstimatePCManual::get_manual_phasecal(int is_remote, int channel_idx, std::string pol)
{
    std::string key = "ref_";
    if(is_remote){key = "rem_";}
    key += "pcphase_";
    char upper_pol = toupper(pol[0]);
    key += upper_pol;

    double phase = 0.0;
    bool present = std::get<CHANNEL_AXIS>(*fVisibilities).RetrieveIndexLabelKeyValue(channel_idx, key, phase);
    phase *= MHO_Constants::rad_to_deg;
    if(present){return phase;}
    return 0.0;
}







////////////////////////////////////////////////////////////////////////////////


/*
 * This routine adjusts delays according to the method specified in 'how':
 *    0x02: use median channel SBD
 *    0x04: average channel SBD
 *    0x08: use total SBD channel
 *    0x10: use original SBD values
 *    0x20: use heuristics to discard outliers
 */
void
MHO_EstimatePCManual::adj_delays(double sbd_max, double sbd[], double esd[], double delta_delay, int first, int final, int rr, int how)
{
    double cpy[MAXFREQ];
    double tol, medly, ave, tot = sbd_max * 1e3;
    int ch, med;

    /* start with a clean slate */
    for (ch = first; ch <= final; ch++) esd[ch] = 0.0;

    /* for methods requiring a median value */
    if ( how & 0x026 )
    {
        for (ch = first, ave=0.0; ch <= final; ch++)
        {
            cpy[ch] = sbd[ch];
            ave += sbd[ch];
        }
        ave /= (final - first + 1);
        //qsort(cpy + first, final - first + 1, sizeof(double), &sbd_cmp);
        med = (first + final) / 2;
        medly = cpy[med];
        //msg("*est: median,average,total delays are %.3f,%.3f,%.3f",3,
        //    medly,ave,tot);
    }

    /* heuristic is to replace outliers with the median delay */
    if (how & 0x20)
    {
        tol = fabs(cpy[med] - tot);
        //msg("*est: tolerance %.3f, retaining %.3f+/-%.3f",3,tol,medly,3*tol);
        for (ch = first; tol > 0 && ch <= final; ch++)
            if (fabs( (sbd[ch] - medly) / tol ) > 3) sbd[ch] = medly;
        /* recompute average */
        for (ch = first, ave=0.0; ch <= final; ch++) ave += sbd[ch];
        ave /= (final - first + 1);
        //msg("*est: revised average delay is %.3f",3,ave);
    }

    if (how & 0x02)
    {            /* use the median value */
        //msg("*est: using median delay (mode %x)",3,how);
        for (ch = first; ch <= final; ch++) esd[ch] = medly - delta_delay;
    }
    else if (how & 0x04)
    {            /* compute and use average */
        //msg("*est: using ave delay (mode %x)",3,how);
        for (ch = first; ch <= final; ch++) esd[ch] = ave - delta_delay;
    }
    else if (how & 0x08)
    {            /* use total SBD value */
        //msg("*est: using total SBD delay (mode %x)",3,how);
        for (ch = first; ch <= final; ch++) esd[ch] = tot - delta_delay;
    }
    else if (how & 0x10)
    {            /* use the measured values */
        //msg("*est: using measured SBD delay (mode %x)",3,how);
        for (ch = first; ch <= final; ch++) esd[ch] = sbd[ch] - delta_delay;
    }

    if (!rr) for (ch = first; ch <= final; ch++) esd[ch] = -esd[ch];
}

/*
 *  This routine calculates adjustments to channel delays
 *  designed to remove the sbd.  Several methods and options
 *  are available via the 'how' argument.  See adj_delays().
 */
//static void est_delays(struct type_pass *pass, int first, int final, int rr, int how)
void MHO_EstimatePCManual::est_delays(int rr, int how)
{
    static char buf[720], tmp[80];
    static double sbd[MAXFREQ], rdy[MAXFREQ], esd[MAXFREQ];
    double delta_delay;
    int ch, ss, pol, nd;
    char *pb, *epd = getenv("HOPS_EST_PC_MDLY");

    int first = 0;
    int final = MAXFREQ;


    //Quantities we need
    double resid_mbd = 0.0;
    double sbd_max = 0.0;
    double sbd_sep = 0.0;
    int sbdbox[MAXFREQ];
    char baseline[2];
    int nlags = 0;


    //*progname = 0;
    //msg("*est: delays on %s station", 1, rr ? "ref" : "rem");
    //if (epd) msg("*est: HOPS_EST_PC_MDLY %s", 3, epd);

    /* restrict operation to only one delay calculation */
    if ((((how & 0x02)>>1) + ((how & 0x04)>>2) +
         ((how & 0x08)>>3) + ((how & 0x10)>>4)) > 1) {
        //msg("*est: too many delay modes selected: 0x%02x",3,how);
        return;
    }

    /* consider a delay correction due to mbd */
    if (how & 0x100)
    {
        //delta_delay = (param.mbd_anchor == MODEL)
        //            ? fringe.t208->resid_mbd
        //            : fringe.t208->resid_mbd - fringe.t208->resid_sbd;
        delta_delay = resid_mbd;
        delta_delay *= ((epd) ? atof(epd) : 1.0) * 1000.0;
        //msg("*est: post-MDLY sbd adjustment %f ns", 3, delta_delay);
    } else {
        delta_delay = 0.0;
    }

    /* build an array of per-channel sbd values */
    for (ch = first; ch <= final; ch++) {
        /* Cf. status.sbdbox[MAXFREQ] <=> status.sbd_max */
        sbd[ch] = (sbdbox[ch] - nlags - 1) * sbd_sep;
        sbd[ch] *= 1000.0;  /* us to ns */
        if (!rr) sbd[ch] = - sbd[ch];

        /* calculate original delays */
        //TODO PORT THIS!!!
        // rdy[ch] = (rr)
        //         ? pass->control.delay_offs[ch].ref
        //         + pass->control.delay_offs_pol[ch][stnpol[0][pass->pol]].ref
        //         : pass->control.delay_offs[ch].rem
        //         + pass->control.delay_offs_pol[ch][stnpol[1][pass->pol]].rem;
    }

    /* make sense of it */
    adj_delays(sbd_max, sbd, esd, delta_delay, first, final, rr, how);

    /* header for the section */
    //TODO FIX THE POL LETTER
    // pol = pol_letter(pass->pol, !rr);


    //TODO FIXE THE ONTROL FILE CONSTRUCTION
    sprintf(buf, "if station %c\n delay_offs_%c ",baseline[!rr], pol);
    // for (ch = first, pb = buf + strlen(buf); ch <= final; ch++, pb++)
    //     *pb = pass->pass_data[ch].freq_code;
    // *pb = 0;
    //msg(buf, 3);

    for (buf[nd = ss = 0] = 0, ch = first; ch <= final; ch++) {
        esd[ch] += rdy[ch];     /* work relative to input value */
        if (fabs(esd[ch] - rdy[ch]) > 0.01) nd ++;
        snprintf(tmp, sizeof(tmp), " %+8.3f", esd[ch]);
        strncat(buf, tmp, sizeof(buf)-1);

        /* eight delays per line for a line length of 73 */
        // if (++ss == 8) {
        //     msg(buf, 3);
        //     buf[ss = 0] = 0;
        // }
    }
    // if (buf[0]) msg(buf, 3);
    //msg("*est: delays %s (%d)", 2, nd ? "converging" : "converged", nd);
}


////////////////////////////////////////////////////////////////////////////////



















}//end of namespace
