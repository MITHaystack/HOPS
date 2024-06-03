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


#include "MHO_Constants.hh"

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
    std::string pol = "?";

    std::string cf_line = "if station " + station_id + "\n" + "pc_phases_" + pol + " ";

    std::map<std::string, double> chan2resid;
    std::map<std::string, double> chan2freq;
    std::vector< std::pair<std::string, double> > ch_freq_pairs;
    std::string chan_label_key = "channel_label";
    std::string sidebandlabelkey = "net_sideband";

    //calculate the average fPhasors
    std::size_t nchan = (&std::get<0>(*fPhasors))->GetSize(); //"what about 'All' channel?"...we just skip it
    std::size_t naps = (&std::get<1>(*fPhasors))->GetSize();
    for(std::size_t i=0; i<nchan; i++) //loop over channels
    {
        //grab the channel label 
        std::string ch_label;
        (&std::get<0>(*fPhasors))->RetrieveIndexLabelKeyValue(i, chan_label_key, ch_label);
        
        if(ch_label != "All")
        {
            double freq = std::get<0>(*fPhasors).at(i);//sky freq of this channel
            std::complex<double> phsum = 0;
            double wght = 0;
            for(std::size_t j=0; j<naps; j++) //sum over APs
            {
                phsum += (*fPhasors)(i,j);
                wght += 1.0; //TODO use real weights, but for now just assume 1
            }
            phsum *= 1.0/(double)wght;
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

    std::cout<<" concatenated channels = "<<concat_ch<<std::endl;



    //for (buf[nd = ss = 0] = 0, ch = first; ch <= final; ch++)
//
//     //loop over pol axis
//
//     //loop over channel axis:
//
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
                double ref_pc;
                //get the rem station manual pcal (if applied)
                double rem_pc;
                
                double est_phase = rem_pc - rem_pc; //should already be in degrees
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
                if(rr){ inp_phase = ref_pc;} //status.pc_phase[ch][1][stnpol[1][pass->pol]];
                else{ inp_phase = -1.0*rem_pc;} //status.pc_phase[ch][0][stnpol[0][pass->pol]];

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
                if(ch_idx != 0 && ch_idx % 8 == 0){output_string +=  "\n";}
            }
            
        }
    }

    std::cout<<cf_line<<" "<<concat_ch<<" "<<output_string<<std::endl;

//     // if (buf[0]) msg(buf, 3);
//     // msg("*est: phases %s (%d)", 2, nd ? "converging" : "converged", nd);

}







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
    if(dophs){ est_phases(doref, domrp); }

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
    est_pc_manual(0);
    return true;
}





}//end of namespace
