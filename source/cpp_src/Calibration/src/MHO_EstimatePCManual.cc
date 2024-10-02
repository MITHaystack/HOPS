#include "MHO_EstimatePCManual.hh"

/*
 * If invoked, estimate ph phase and
 * delay values to use in manual mode.
 * pc_phase_? and delay_offs_? values report.
 *
 * This is adapted from fearfit test code. gbc 5/8/2017
 * Ported to hops4 jpb 6/6/2024
 */

#include <algorithm>
#include <cctype>
#include <cmath>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <stdlib.h>

#include "MHO_Constants.hh"

#define MAXFREQ 64

/* a comparison routine for use by qsort() */
static int sbd_cmp(const void* a, const void* b)
{
    double da = *(double*)a, db = *(double*)b;
    if(da < db)
        return (-1);
    if(da > db)
        return (1);
    return (0);
}

/* move phase to principal branch */
static double pbranch(double phase)
{
    phase = (fmod(phase, 360.0));
    if(phase < -180.0)
    {
        phase += 360.0;
    }
    if(phase > 180.0)
    {
        phase -= 360.0;
    }
    return phase;
}

namespace hops
{

//the ordering operator for channel labels to sort by frequency;
class freq_predicate
{
    public:
        freq_predicate(){};
        virtual ~freq_predicate(){};

        virtual bool operator()(const std::pair< std::string, double >& a, const std::pair< std::string, double >& b)
        {
            return a.second < b.second;
        }
};

MHO_EstimatePCManual::MHO_EstimatePCManual(){};
MHO_EstimatePCManual::~MHO_EstimatePCManual(){};

bool MHO_EstimatePCManual::ExecuteImpl(const visibility_type* in)
{
    fVisibilities = in;
    int mode = 0;
    bool ok = fParameterStore->Get("/control/config/est_pc_manual", mode);
    est_pc_manual(mode);
    return true;
}

void MHO_EstimatePCManual::est_pc_manual(int mode)
{
    //fParameterStore->Dump();

    std::string rootfile = fParameterStore->GetAs< std::string >("/files/root_file");

    std::string ref_id = fParameterStore->GetAs< std::string >("/ref_station/mk4id");
    std::string rem_id = fParameterStore->GetAs< std::string >("/rem_station/mk4id");
    fRefStationMk4ID = ref_id;
    fRemStationMk4ID = rem_id;

    std::string polprod = fParameterStore->GetAs< std::string >("/config/polprod");
    fRefStationPol = tolower(polprod[0]);
    fRemStationPol = tolower(polprod[1]);

    std::string key = "/control/station/pc_mode";
    std::string default_pcmode;
    bool have_default = fParameterStore->Get(key, default_pcmode);
    if(!have_default)
    {
        default_pcmode = "manual";
    }

    std::string ref_pcmode;
    key = "/control/station/" + ref_id + "/pc_mode";
    bool have_refmode = fParameterStore->Get(key, ref_pcmode);
    if(!have_refmode)
    {
        ref_pcmode = default_pcmode;
    }

    std::string rem_pcmode;
    key = "/control/station/" + rem_id + "/pc_mode";
    bool have_remmode = fParameterStore->Get(key, rem_pcmode);
    if(!have_remmode)
    {
        rem_pcmode = default_pcmode;
    }

    //this is only enabled for pc_mode manual...why? TODO FIXME
    if(ref_pcmode != "manual" || rem_pcmode != "manual")
    {
        return;
    }

    //map channel labels to indices
    fChannelLabel2Index.clear();
    fIndex2ChannelLabel.clear();
    std::string ch_label_key = "channel_label";
    auto chan_ax = &(std::get< CHANNEL_AXIS >(*fVisibilities));
    for(int i = 0; i < chan_ax->GetSize(); i++)
    {
        std::string label;
        bool ok = chan_ax->RetrieveIndexLabelKeyValue(i, ch_label_key, label);
        if(ok)
        {
            fChannelLabel2Index[label] = i;
            fIndex2ChannelLabel[i] = label;
        }
    }

    //determine mode logic
    int doref = (mode > 0) ? 1 : 0;
    if(!doref)
        mode *= -1;           /* so that mode is now positive   */
    int dophs = mode & 0x001; /* per-channel phase correction   */
    int dodly = mode & 0x13e; /* 0x02 0x04 0x08 0x10 0x20 0x100 */
    int dooff = mode & 0x040; /* estimate phase offset value    */
    int domrp = mode & 0x080; /* phase bias HOPS_EST_PC_BIAS    */

    // first_ch = (param.first_plot == 0) ? 0 : param.first_plot;
    // final_ch = (param.nplot_chans == 0) ? pass->nfreq : param.nplot_chans;
    // final_ch += first_ch - 1;
    // masthead(mode, rootfile, pass, first_ch, final_ch);

    //compute pc_phases for all channels of the visibility array
    if(dophs)
    {
        est_phases(doref, domrp);
    }
    if(dodly)
    {
        est_delays(doref, dodly);
    }
    if(dooff)
    {
        est_offset(doref);
    }
    // msg("*-----------------------------------"
    //     "------------------------------------",3);

    //rest_pn();
    // msg("done with  pc phases and delays", 1);
}

double MHO_EstimatePCManual::get_manual_phasecal(int is_remote, int channel_idx, std::string pol)
{
    std::string key = "ref_";
    if(is_remote)
    {
        key = "rem_";
    }
    std::string pol_key = key + "pcphase_offset_";
    key += "pcphase_";
    char upper_pol = toupper(pol[0]);
    key += upper_pol;
    pol_key += upper_pol;

    double phase = 0.0;
    bool present = false;

    double pol_phase = 0.0;
    present = std::get< POLPROD_AXIS >(*fVisibilities).RetrieveIndexLabelKeyValue(0, pol_key, pol_phase);
    pol_phase *= MHO_Constants::rad_to_deg;
    if(present)
    {
        phase += pol_phase;
    }

    double ch_phase = 0.0;
    present = std::get< CHANNEL_AXIS >(*fVisibilities).RetrieveIndexLabelKeyValue(channel_idx, key, ch_phase);
    ch_phase *= MHO_Constants::rad_to_deg;
    if(present)
    {
        phase += ch_phase;
    }

    return phase;
}

double MHO_EstimatePCManual::get_manual_delayoff(int is_remote, int channel_idx, std::string pol)
{
    std::string key = "ref_";
    if(is_remote)
    {
        key = "rem_";
    }
    key += "delayoff_";
    char upper_pol = toupper(pol[0]);
    key += upper_pol;

    //test...flip order of channels to check on bug in c code
    //channel_idx = 31 - channel_idx;

    double delay = 0.0;

    bool present = false;
    //grab the pol-based delay
    double pol_delay = 0.0;
    present = std::get< POLPROD_AXIS >(*fVisibilities).RetrieveIndexLabelKeyValue(0, key, pol_delay);
    if(present)
    {
        delay += pol_delay;
    }

    //grab the individual channel delay
    double ch_delay = 0.0;
    present = std::get< CHANNEL_AXIS >(*fVisibilities).RetrieveIndexLabelKeyValue(channel_idx, key, ch_delay);
    if(present)
    {
        delay += ch_delay;
    }

    return delay;
}

void MHO_EstimatePCManual::fill_sbd(std::vector< std::string >& ch_labels, std::vector< double >& sbd)
{
    bool ok;
    ch_labels.clear();
    sbd.clear();

    mho_json ch_obj;
    ok = fPlotData.Get("/PLOT_INFO/#Ch", ch_obj);
    if(ok)
    {
        ch_labels = ch_obj.get< std::vector< std::string > >();
    }

    mho_json obj;
    ok = fPlotData.Get("/PLOT_INFO/SbdBox", obj);
    if(ok)
    {
        sbd = obj.get< std::vector< double > >();
    }

    //trim the last element 'All' from the list
    if(ch_labels.back() == "All" && ch_labels.size() == sbd.size())
    {
        ch_labels.pop_back();
        sbd.pop_back();
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void MHO_EstimatePCManual::est_phases(int is_ref, int keep)
{
    int nd = 0;
    char* epb = getenv("HOPS_EST_PC_BIAS");
    char* epd = getenv("HOPS_EST_PC_DLYM");

    if(is_ref)
    {
        msg_debug("calibration", "*est: phases on ref station" << eom);
    }
    else
    {
        msg_debug("calibration", "*est: phases on rem station" << eom);
    }

    /* support for bias operation */
    double phase_bias;
    double coh_avg_phase = 0; //TODO FIXME
    if(keep)
    {
        phase_bias = (epb) ? atof(epb) : 0.0;
        msg_debug("calibration", "*est: phase bias " << phase_bias << eom);
    }

    if(epb || epd)
    {
        msg_debug("calibration", "*est: HOPS_EST_PC_BIAS " << epb << " ..._DLYM " << epd << eom);
    }

    double ref_freq = fParameterStore->GetAs< double >(std::string("/control/config/ref_freq"));

    //construct the control file line prefix
    std::string station_id = "?";
    if(is_ref)
    {
        station_id = fRefStationMk4ID;
    }
    else
    {
        station_id = fRemStationMk4ID;
    }

    std::string pol = "?";
    if(is_ref)
    {
        pol = fRefStationPol;
    }
    else
    {
        pol = fRemStationPol;
    }

    std::string cf_line = "if station " + station_id + "\n" + "pc_phases_" + pol + " ";

    std::map< std::string, double > chan2resid;
    std::map< std::string, double > chan2freq;
    std::vector< std::pair< std::string, double > > ch_freq_pairs;
    std::string chan_label_key = "channel_label";
    std::string sidebandlabelkey = "net_sideband";

    //calculate the average fPhasors
    std::size_t nchan = (&std::get< 0 >(*fPhasors))->GetSize(); //"what about 'All' channel?"...we just skip it
    std::size_t naps = (&std::get< 1 >(*fPhasors))->GetSize();

    std::size_t wchan = fWeights->GetDimension(CHANNEL_AXIS);
    std::size_t wnaps = fWeights->GetDimension(TIME_AXIS);
    std::size_t POLPROD_INDEX = 0;

    for(std::size_t i = 0; i < nchan; i++) //loop over channels
    {
        //grab the channel label
        std::string ch_label;
        (&std::get< 0 >(*fPhasors))->RetrieveIndexLabelKeyValue(i, chan_label_key, ch_label);

        if(ch_label != "All")
        {
            double freq = std::get< 0 >(*fPhasors).at(i); //sky freq of this channel
            std::complex< double > phsum = 0;
            double wsum = 0;
            double w;
            for(std::size_t j = 0; j < naps; j++) //sum over APs
            {
                phsum += (*fPhasors)(i, j);
                w = 1.0;
                if(i < wchan && j < wnaps)
                {
                    w = (*fWeights)(POLPROD_INDEX, i, j, 0);
                }
                wsum += w;
            }
            phsum *= 1.0 / (double)wsum;
            double ch_arg = std::arg(phsum);
            chan2resid[ch_label] = ch_arg;
            chan2freq[ch_label] = freq;
            ch_freq_pairs.push_back(std::make_pair(ch_label, freq));
        }
    }

    //construct the frequency ordered list of channels
    freq_predicate fpred;
    std::sort(ch_freq_pairs.begin(), ch_freq_pairs.end(), fpred);
    std::string concat_ch;
    for(std::size_t i = 0; i < ch_freq_pairs.size(); i++)
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
        std::vector< std::size_t > ch_idx_vec = std::get< CHANNEL_AXIS >(*fVisibilities).GetMatchingIndexes(chan_label_key, ch);
        if(ch_idx_vec.size() == 1)
        {
            ch_idx = ch_idx_vec[0];
            std::string net_sideband = "?";
            bool key_present =
                std::get< CHANNEL_AXIS >(*fVisibilities).RetrieveIndexLabelKeyValue(ch_idx, sidebandlabelkey, net_sideband);
            if(!key_present)
            {
                msg_error("calibration", "missing net_sideband label for channel " << ch << "." << eom);
            }
            else
            {
                double sbmult = 0.0;
                if(net_sideband == "L")
                {
                    sbmult = -1.0;
                }
                if(net_sideband == "U")
                {
                    sbmult = 1.0;
                }

                //get the ref station manual pcal (if applied)
                double ref_pc = get_manual_phasecal(0, ch_idx, fRefStationPol); //0 indicates ref station
                //get the rem station manual pcal (if applied)
                double rem_pc = get_manual_phasecal(1, ch_idx, fRemStationPol); //1 indicates rem station

                double est_phase = ref_pc - rem_pc; //should already be in degrees
                double inp_phase = pbranch(est_phase);

                //get the residual delays
                double resid_mbd = fParameterStore->GetAs< double >("/fringe/mbdelay");
                double resid_sbd = fParameterStore->GetAs< double >("/fringe/sbdelay");

                //get the mbd_anchor method
                std::string mbd_anchor = fParameterStore->GetAs< std::string >("/control/config/mbd_anchor");
                double delta_delay = 0.0;
                if(mbd_anchor == "model")
                {
                    delta_delay = resid_mbd;
                }
                else if(mbd_anchor == "sbd")
                {
                    delta_delay = resid_mbd - resid_sbd;
                }

                if(epd)
                {
                    delta_delay *= std::atof(epd);
                }

                est_phase += sbmult * (ch_resid_phase * MHO_Constants::rad_to_deg) + 360.0 * delta_delay * (ch_freq - ref_freq);

                /* bias the phase calculation to preserve existing resid phase */
                if(keep)
                    est_phase += phase_bias;

                /* canonicalize for comparision */
                est_phase = pbranch(est_phase);
                if(fabs(inp_phase - est_phase) > 0.01)
                    nd++;

                /* remove input phase values */
                if(is_ref)
                {
                    inp_phase = get_manual_phasecal(1, ch_idx, fRemStationPol);
                } //status.pc_phase[ch][1][stnpol[1][pass->pol]];
                else
                {
                    inp_phase = -1.0 * get_manual_phasecal(0, ch_idx, fRefStationPol);
                }                       //status.pc_phase[ch][0][stnpol[0][pass->pol]];
                est_phase += inp_phase; //already in degrees

                if(is_ref)
                {
                    est_phase = pbranch(est_phase);
                }
                else
                {
                    est_phase = pbranch(-est_phase);
                }

                char tmp[80] = {0};
                snprintf(tmp, sizeof(tmp), " %+8.3f", est_phase);
                std::stringstream ss;
                ss << tmp;
                output_string += ss.str();
                if(ch_idx % 8 == 7)
                {
                    output_string += "\n";
                }
            }
        }
    }

    msg_info("calibration", "*est: control file info: " << eol);
    msg_info("calibration", "\n" << cf_line << " " << concat_ch << "\n" << output_string << eom);

    msg_debug("calibration", "*est: phases converged (" << nd << ")" << eom);
}

/*
 * This routine adjusts delays according to the method specified in 'how':
 *    0x02: use median channel SBD
 *    0x04: average channel SBD
 *    0x08: use total SBD channel
 *    0x10: use original SBD values
 *    0x20: use heuristics to discard outliers
 */
void MHO_EstimatePCManual::adj_delays(double sbd_max, double* sbd, double* esd, double delta_delay, int first, int final,
                                      int is_ref, int how)
{
    double cpy[MAXFREQ];
    double tol, medly, ave;
    double tot = sbd_max * 1e3;
    int ch, med;

    /* start with a clean slate */
    for(ch = first; ch < final; ch++)
    {
        esd[ch] = 0.0;
    }

    /* for methods requiring a median value */
    if(how & 0x026)
    {
        for(ch = first, ave = 0.0; ch < final; ch++)
        {
            cpy[ch] = sbd[ch];
            ave += sbd[ch];
        }
        ave /= (final - first); // + 1);
        qsort(cpy + first, final, sizeof(double), &sbd_cmp);
        med = (first + final) / 2;
        medly = cpy[med];
        msg_debug("calibration", "*est: median,average,total delays are: " << medly << ", " << ave << ", " << tot << eom);
    }

    /* heuristic is to replace outliers with the median delay */
    if(how & 0x20)
    {
        tol = fabs(cpy[med] - tot);
        msg_debug("calibration", "*est: tolerance " << tol << ", retaining " << medly << "+/-" << 3 * tol << eom);

        for(ch = first; tol > 0 && ch < final; ch++)
        {
            if(fabs((sbd[ch] - medly) / tol) > 3)
            {
                sbd[ch] = medly;
            }
        }
        /* recompute average */
        for(ch = first, ave = 0.0; ch < final; ch++)
        {
            ave += sbd[ch];
        }
        ave /= (final - first + 1);
        msg_debug("calibration", "*est: revised average delay is " << ave << eom);
    }

    if(how & 0x02)
    { /* use the median value */
        msg_debug("calibration", "*est: using median delay (mode " << std::hex << how << std::dec << ")" << eom);
        for(ch = first; ch < final; ch++)
        {
            esd[ch] = medly - delta_delay;
        }
    }
    else if(how & 0x04)
    { /* compute and use average */
        msg_debug("calibration", "*est: using ave delay (mode " << std::hex << how << std::dec << ")" << eom);
        for(ch = first; ch < final; ch++)
        {
            esd[ch] = ave - delta_delay;
        }
    }
    else if(how & 0x08)
    { /* use total SBD value */
        msg_debug("calibration", "*est: using total SBD delay (mode " << std::hex << how << std::dec << ")" << eom);
        for(ch = first; ch < final; ch++)
        {
            esd[ch] = tot - delta_delay;
        }
    }
    else if(how & 0x10)
    { /* use the measured values */
        msg_debug("calibration", "*est: using measured SBD delay (mode " << std::hex << how << std::dec << ")" << eom);
        for(ch = first; ch < final; ch++)
        {
            esd[ch] = sbd[ch] - delta_delay;
        }
    }

    if(!is_ref)
    {
        for(ch = first; ch < final; ch++)
        {
            esd[ch] = -esd[ch];
        }
    }
}

/*
 *  This routine calculates adjustments to channel delays
 *  designed to remove the sbd.  Several methods and options
 *  are available via the 'how' argument.  See adj_delays().
 */
//static void est_delays(struct type_pass *pass, int first, int final, int is_ref, int how)
void MHO_EstimatePCManual::est_delays(int is_ref, int how)
{
    std::vector< std::string > ch_label;
    std::vector< double > sbd_vec;
    std::vector< double > rdy_vec;
    std::vector< double > esd_vec;

    int ch, ss, nd = 0;
    char *pb, *epd = getenv("HOPS_EST_PC_MDLY");

    //quantities we need
    double sbd_sep = fParameterStore->GetAs< double >("/fringe/sbd_separation");
    int nlags = fParameterStore->GetAs< int >("/config/nlags");
    double resid_mbd = fParameterStore->GetAs< double >("/fringe/mbdelay");
    double resid_sbd = fParameterStore->GetAs< double >("/fringe/sbdelay");
    double sbd_max = resid_sbd;
    double delta_delay = 0.0;

    fill_sbd(ch_label, sbd_vec);
    rdy_vec.resize(sbd_vec.size());
    esd_vec.resize(sbd_vec.size());

    double* sbd = &(sbd_vec[0]);
    double* rdy = &(rdy_vec[0]);
    double* esd = &(esd_vec[0]);

    //just do all channels
    int first = 0;
    int final = sbd_vec.size();

    if(is_ref)
    {
        msg_debug("calibration", "*est: phases on ref station" << eom);
    }
    else
    {
        msg_debug("calibration", "*est: phases on rem station" << eom);
    }

    if(epd)
    {
        msg_debug("calibration", "*est: HOPS_EST_PC_MDLY " << epd << eom);
    }

    /* restrict operation to only one delay calculation */
    if((((how & 0x02) >> 1) + ((how & 0x04) >> 2) + ((how & 0x08) >> 3) + ((how & 0x10) >> 4)) > 1)
    {
        msg_debug("calibration", "*est: too many delay modes selected: " << std::hex << how << std::dec << eom);
        return;
    }

    /* consider a delay correction due to mbd */
    if(how & 0x100)
    {
        //get the mbd_anchor method
        std::string mbd_anchor = fParameterStore->GetAs< std::string >("/control/config/mbd_anchor");
        if(mbd_anchor == "model")
        {
            delta_delay = resid_mbd;
        }
        else if(mbd_anchor == "sbd")
        {
            delta_delay = resid_mbd - resid_sbd;
        }
        delta_delay *= ((epd) ? atof(epd) : 1.0) * 1000.0;
        msg_debug("calibration", "*est: post-MDLY sbd adjustment " << delta_delay << " ns" << eom);
    }

    //info needed to construct the control file line prefix
    std::string station_id = "?";
    if(is_ref)
    {
        station_id = fRefStationMk4ID;
    }
    else
    {
        station_id = fRemStationMk4ID;
    }

    /* header for the section */
    std::string pol = "?";
    if(is_ref)
    {
        pol = fRefStationPol;
    }
    else
    {
        pol = fRemStationPol;
    }

    /* build an array of per-channel sbd values */
    for(ch = first; ch < final; ch++)
    {

        sbd[ch] = (sbd[ch] - (double)nlags - 1) * sbd_sep;
        sbd[ch] *= 1000.0; /* us to ns */
        if(!is_ref)
            sbd[ch] = -sbd[ch];

        /* calculate original delays */
        rdy[ch] = get_manual_delayoff(!is_ref, fChannelLabel2Index[ch_label[ch]], pol);
    }

    /* make sense of it */
    adj_delays(sbd_max, sbd, esd, delta_delay, first, final, is_ref, how);

    //control file line prefix
    std::string cf_line = "if station " + station_id + "\n" + "delay_offs_" + pol + " ";

    std::string concat_ch;
    for(std::size_t i = 0; i < ch_label.size(); i++)
    {
        if(ch_label[i] != "All")
        {
            concat_ch += ch_label[i];
        }
    }

    std::string output_string;

    for(int ch = first; ch < final; ch++)
    {
        esd[ch] += rdy[ch]; /* work relative to input value */
        if(fabs(esd[ch] - rdy[ch]) > 0.01)
            nd++;

        char tmp[80] = {0};
        snprintf(tmp, sizeof(tmp), " %+8.3f", esd[ch]);
        std::stringstream ss;
        ss << tmp;
        output_string += ss.str();
        if(ch % 8 == 7)
        {
            output_string += "\n";
        }
    }
    msg_debug("calibration", "*est: delays converged (" << nd << ")" << eom);
    msg_info("calibration", "*est: control file info: " << eol);
    msg_info("calibration", "\n" << cf_line << " " << concat_ch << "\n" << output_string << eom);
}

////////////////////////////////////////////////////////////////////////////////

void MHO_EstimatePCManual::est_offset(int is_ref)
{
    //grab the residual phase
    double resphase = fParameterStore->GetAs< double >("/fringe/resid_phase");
    double pc_phase_offset = 0.0;

    std::string station_id = "?";
    if(is_ref)
    {
        station_id = fRefStationMk4ID;
    }
    else
    {
        station_id = fRemStationMk4ID;
    }

    std::string pol = "";
    std::string polprod = fParameterStore->GetAs< std::string >("/config/polprod");
    if(is_ref)
    {
        pol += polprod[0];
    }
    else
    {
        pol += polprod[1];
    }
    std::string polchar = "";
    polchar += tolower(pol[0]);

    std::string key;
    if(is_ref)
    {
        key = "ref_pcphase_offset_" + pol;
    }
    else
    {
        key = "rem_pcphase_offset_" + pol;
    }

    bool key_present = std::get< POLPROD_AXIS >(*fVisibilities).RetrieveIndexLabelKeyValue(0, key, pc_phase_offset);

    if(!key_present)
    {
        pc_phase_offset = 0.0;
    }
    pc_phase_offset *= MHO_Constants::rad_to_deg;

    double ofs;

    if(is_ref)
    {
        ofs = resphase - pc_phase_offset;
    }
    else
    {
        ofs = -1.0 * resphase + pc_phase_offset;
    }

    std::string cf_line = "if station " + station_id + "\n pc_phase_offset_" + polchar + " ";

    msg_info("calibration", "*est: control file info: " << eol);
    msg_info("calibration", "\n" << cf_line << ofs << "\n" << eom);
}

/* generate information about where the results came from */
void MHO_EstimatePCManual::masthead(int mode, std::string root_file, int first_ch, int final_ch)
{
    // std::string control_filename;
    // std::string ref_name;
    // std::string rem_name;
    // std::string fgroup;
    // std::string low_chan_fcode;
    // std::string high_chan_fcode;
    // std::string pol_string;
    //
    // std::string baseline;
    //
    // double snr;
    // double delres_max;
    // double resphase;
    //
    // double sbd_max;
    // double mbd_max_global;
    // double dr_max_global;
    // double ref_freq;

    // msg("rf:  %s", 3, root_file);
    // msg("cf:  %s", 3, control_filename);
    // msg("on: %.8s - %.8s [%c%c] fq %c pol %s ch %c..%c mode %03X", 3,
    //     ref_name, rem_name,
    //     baseline[0], baseline[1],
    //     fgroup, pol_string,
    //     low_chan_fcode,
    //     high_chan_fcode, mode);
    // msg("snr %.3f amp %.6f phs %.6f", 3,
    //     snr, delres_max, resphase);
    // msg("sbd %.6f mbd %.6f frr %.6f", 3,
    //     sbd_max, mbd_max_global,
    //     dr_max_global * ref_freq);
}

} // namespace hops
