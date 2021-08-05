#include <iostream>
#include <string>
#include <vector>
#include <getopt.h>

#include "MHO_Message.hh"
#include "MHO_MK4VexInterface.hh"
#include "MHO_MK4CorelInterface.hh"
#include "MHO_MK4StationInterface.hh"

#include "MHO_Visibilities.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_VisibilityChannelizer.hh"
#include "MHO_WeightChannelizer.hh"
#include "MHO_StationCoordinates.hh"

#include "MHO_DirectoryInterface.hh"

//#include "MHO_NormFX.hh"


#define signum(a) (a>=0 ? 1.0 : -1.0)

#define CIRC_MODE 0
#define LIN_MODE 1
#define MIXED_MODE 2

int fcode (char c, char *codes)
	{
	int i;

	for (i = 0; i < MAXFREQ; i++)
		if (c == codes[i])
            return i;
    return -1;
	}





extern "C"
{
    #include "mk4_data.h"
    #include "vex.h"
    #include "pass_struct.h"
    #include "param_struct.h"
    #include "write_lock_mechanism.h"

    /* External structure declarations */
    struct mk4_corel cdata;
    struct type_param param;
    struct type_status status;
    struct mk4_fringe fringe;
    struct type_plot plot;
    struct c_block *cb_head;
    struct mk4_sdata sdata[MAXSTATIONS];

    int
    default_cblock (struct c_block *cb_ptr);

    int
    set_defaults();

    int
    organize_data (
    struct mk4_corel *cdata,
    struct scan_struct *ovex,
    struct ivex_struct *ivex,
    struct mk4_sdata *sdata,
    struct freq_corel *corel,
    struct type_param* param);

    void
    norm_fx (
    struct type_pass* pass,
    struct type_param* param,
    struct type_status* status,
    int fr,
    int ap);

    int
    make_passes (
    struct scan_struct *ovex,
    struct freq_corel *corel,
    struct type_param *param,
    struct type_pass **pass,
    int *npass);


    int baseline, base, ncorel_rec, lo_offset, max_seq_no;
    int do_only_new = FALSE;
    int test_mode = FALSE;
    int write_xpower = FALSE;
    int do_accounting = FALSE;
    int do_estimation = FALSE;
    int refringe = FALSE;
    int ap_per_seg = 0;
    int reftime_offset = 0;
    int version_no = 0;

    //global variables provided for signal handler clean up of lock files
    lockfile_data_struct global_lockfile_data;

    int msglev = -2;
    char progname[] = "test";

}




#include "MHO_FFTWTypes.hh"
#include "MHO_TableContainer.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_BinaryNDArrayOperator.hh"

namespace hops
{


class MHO_NormFX: public MHO_BinaryNDArrayOperator<
    ch_baseline_data_type,
    ch_baseline_weight_type,
    ch_baseline_sbd_type >
{
    public:
        MHO_NormFX(){}
        virtual ~MHO_NormFX(){};

        virtual bool Initialize() override {}
        virtual bool ExecuteOperation() override {};

    private:


        // //this version of the function will gradually get modified
        // //until we can move functionality out of it entirely and
        // //make it more modular
        void new_norm_fx(struct type_pass *pass,
                         struct type_param* param,
                         struct type_status* status,
                         int fr, int ap)
        {


            struct type_120 *t120;
            struct freq_corel *fdata;
            struct data_corel *datum;
            int sb, st, i, rev_i, j, l, m;
            int nlags = 0;
            int ip, ips;

            hops_complex z;
            double factor, mean;
            double diff_delay, deltaf, polcof, polcof_sum, phase_shift, dpar;
            int freq_no,
                ibegin,
                sindex,
                pol,
                pols,                       // bit-mapped pols to be processed in this pass
                usb_present, lsb_present,
                usb_bypol[4],lsb_bypol[4],
                lastpol[2];                 // last pol index with data present, by sideband
            int datum_uflag, datum_lflag;
            int stnpol[2][4] = {0, 1, 0, 1, 0, 1, 1, 0}; // [stn][pol] = 0:L/X/H, 1:R/Y/V




                //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                //From the input data and 'pass' data (read: user config), determine which pol-product
                //we are going to transform, and whether or not it is linear, circular or mixed
                //and also if we are computing a linear combination of polarizations
                //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!




            //determine if the ref and rem stations are using circular or linear
            //feeds, or it is some combination
            int station_pol_mode = CIRC_MODE;
            if( pass->linpol[0] == 0 && pass->linpol[1] == 0){station_pol_mode = CIRC_MODE;}
            if( pass->linpol[0] == 1 && pass->linpol[1] == 1){station_pol_mode = LIN_MODE;}
            if( pass->linpol[0] != pass->linpol[1] ){station_pol_mode = MIXED_MODE;};

            if (pass->npols == 1)
                {
                pol = pass->pol;            // single pol being done per pass
                ips = pol;
                pols = 1 << pol;
                }
            else                            // linear combination of polarizations
                {
                ips = 0;
                pols = param->pol;
                }



                //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                //For the number of 'lags' (e.g. spectral channels) of this pass create an
                //FFT plan that can transform the data of this size
                //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


                                            // do fft plan only iff nlags changes
            if (param->nlags != nlags)
                {
                nlags = param->nlags;
                fftplan = fftw_plan_dft_1d (4 * nlags,
                    reinterpret_cast<typename MHO_FFTWTypes<double>::fftw_complex_type_ptr>(S),
                    reinterpret_cast<typename MHO_FFTWTypes<double>::fftw_complex_type_ptr>(xlag),
                    FFTW_FORWARD, FFTW_MEASURE);
                }

            freq_no = fcode(pass->pass_data[fr].freq_code, pass->control.chid);

                                            /* Point to current frequency */
            fdata = pass->pass_data + fr;
                                            /* Convenience pointer */
            datum = fdata->data + ap;


                //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                //Calculate the differential parallactic angle (only important for lin-pol)
                //to be used when combining lin-pol producs
                //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


                                            // differenced parallactic angle
            dpar = param->par_angle[1] - param->par_angle[0];
                                                /* Initialize */
            for (i = 0; i < nlags*4; i++)
                S[i] = 0.0;

            datum->sband = 0;
                                            /* -1.0 means no data, not zero weight */
            datum->usbfrac = -1.0;
            datum->lsbfrac = -1.0;

            polcof_sum = 0.0;

            usb_present = FALSE;
            lsb_present = FALSE;
            lastpol[0] = ips;
            lastpol[1] = ips;



        //Disable ad-hoc flagging //////////////////////////////////////////////////////
            //ADHOC_FLAG(&param, datum->flag, fr, ap, &datum_uflag, &datum_lflag);



        // check sidebands for each pol. for data
            for (ip=ips; ip<pass->pol+1; ip++)
                {
                usb_bypol[ip] = ((datum_uflag & (USB_FLAG << 2*ip)) != 0)
                             && ((pols & (1 << ip)) != 0);
                lsb_bypol[ip] = ((datum_lflag & (LSB_FLAG << 2*ip)) != 0)
                             && ((pols & (1 << ip)) != 0);
                pass->pprods_present[ip] |= usb_bypol[ip] || lsb_bypol[ip];

                if (usb_bypol[ip])
                    lastpol[0] = ip;
                if (lsb_bypol[ip])
                    lastpol[1] = ip;

                usb_present |= usb_bypol[ip];
                lsb_present |= lsb_bypol[ip];
                }
            datum->sband = usb_present - lsb_present;


            //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            //The following monstrosity is looping over both side bands and all pol-products
            //and determining which spectral data to sum into the 'xp_spec' array (to be x-formed)
            //Since this is not complicated enough, it is also responsible for:
            //(0) Filtering out any NaNs
            //(1) Performing a coherent sum over the desired pol-products (and applying dpar corrections for linear-polprod data).
            //(2) Determining the data-weights for each sideband
            //(3) Apply phase-cal correction, either for multitone or manuals
            //(4) Applying delay-diff correction either from multitone or manuals
            //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!






                                            /*  sideband # -->  0=upper , 1= lower */
            for (sb = 0; sb < 2; sb++)
              {
              for (i = 0; i < nlags*4; i++) // clear xcor & xp_spec for pol sum into them
                  {
                  xcor[i] = 0.0;
                  xp_spec[i] = 0.0;
                  }
                                            // loop over polarization products
              for (ip=ips; ip<pass->pol+1; ip++)
                {
                if (param->pol)
                    pol = ip;
                                            // If no data for this sb/pol, go on to next
                if ((sb == 0 && usb_bypol[ip] == 0)
                 || (sb == 1 && lsb_bypol[ip] == 0))
                    continue;
                                            // Pluck out the requested polarization
                switch (pol)
                    {
                    case POL_LL: t120 = datum->apdata_ll[sb];
                    if(station_pol_mode == LIN_MODE)  //TODO: check if this correction should also be applied in mixed-mode case
                    {
                                 polcof = (pass->npols > 1) ?
                                     cos (dpar) :
                                     signum (cos (dpar));
                    }
                    else
                    {
                        polcof = 1;
                    }
                                 break;
                    case POL_RR: t120 = datum->apdata_rr[sb];
                    if(station_pol_mode == LIN_MODE)
                    {
                                 polcof = (pass->npols > 1) ?
                                     cos (dpar) :
                                     signum (cos (dpar));
                    }
                    else
                    {
                        polcof = 1;
                    }
                                 break;
                    case POL_LR: t120 = datum->apdata_lr[sb];
                    if(station_pol_mode == LIN_MODE)
                    {
                                 polcof = (pass->npols > 1) ?
                                     sin (-dpar) :
                                     signum (sin (-dpar));
                    }
                    else
                    {
                        polcof = 1;
                    }
                                 break;
                    case POL_RL: t120 = datum->apdata_rl[sb];
                    if(station_pol_mode == LIN_MODE)
                    {
                                 polcof = (pass->npols > 1) ?
                                     sin (dpar) :
                                     signum (sin (dpar));
                    }
                    else
                    {
                        polcof = 1;
                    }
                                 break;
                    }
                polcof_sum += fabs (polcof);
                                            // sanity test
                if (t120 -> type != SPECTRAL)
                    {
                    //msg ("Conflicting correlation type %d", 2, t120->type);
                    return;
                    }

                // note datum->lsbfrac or datum->usbfrac remains at -1.0
                if (pass->control.min_weight > 0.0 &&
                    pass->control.min_weight > t120->fw.weight) continue;

                                            // determine data weights by sideband
                if (ip == lastpol[sb])
                    {                       // last included polarization, do totals
                    status->ap_num[sb][fr]++;
                    status->total_ap++;
                                            // sum to micro-edited totals
                    if (sb)                 // lower sideband
                        {                   // 0 weight encoded by negative 0
                        if (*((unsigned int *)(&(t120->fw.weight))) == 0)
                                            // +0 is backward-compatibility for no weight
                            datum->lsbfrac = 1.0;
                        else
                            datum->lsbfrac = t120->fw.weight;
                        status->ap_frac[sb][fr] += datum->lsbfrac;
                        status->total_ap_frac   += datum->lsbfrac;
                        status->total_lsb_frac  += datum->lsbfrac;
                        }
                    else                    // upper sideband
                        {
                        if (*((unsigned int *)(&(t120->fw.weight))) == 0)
                            datum->usbfrac = 1.0;
                        else
                            datum->usbfrac = t120->fw.weight;
                        status->ap_frac[sb][fr] += datum->usbfrac;
                        status->total_ap_frac   += datum->usbfrac;
                        status->total_usb_frac  += datum->usbfrac;
                        }
                    }

                                            // add in phase effects if multitone delays
                                            // were extracted
                if (pass->control.nsamplers && param->pc_mode[0] == MULTITONE
                                            && param->pc_mode[1] == MULTITONE)
                    diff_delay = +1e9 * (datum->rem_sdata.mt_delay[stnpol[1][pol]]
                                       - datum->ref_sdata.mt_delay[stnpol[0][pol]]);
                                            // ##DELAY_OFFS## otherwise assume user has
                                            // used delay_offs or delay_offs_? but not both.
                else
                    diff_delay = pass->control.delay_offs_pol[freq_no][stnpol[1][pol]].rem
                               + pass->control.delay_offs[freq_no].rem  // ##DELAY_OFFS##
                               - pass->control.delay_offs[freq_no].ref  // ##DELAY_OFFS##
                               - pass->control.delay_offs_pol[freq_no][stnpol[0][pol]].ref;
                //msg ("ap %d fr %d pol %d diff_delay %f", -2, ap, fr, pol, diff_delay);

                                            // loop over spectral points
                for (i=0; i<nlags/2; i++)
                    {                       // filter out any nan's, if present
                    if (isnan (t120->ld.spec[i].re) || isnan (t120->ld.spec[i].im))
                        {
                        //msg ("omitting nan's in visibility for ap %d fr %d lag %i",
                        //      2, ap, fr, i);
                        }
                                            // add in iff this is a requested pol product
                    else if (param->pol & 1<<ip || param->pol == 0)
                        {
                        z = t120->ld.spec[i].re + I * t120->ld.spec[i].im;
                                            // rotate each pol prod by pcal prior to adding in
                        if (sb==0)
                            z = z * datum->pc_phasor[ip];
                        else                // use conjugate of usb pcal tone for lsb
                            z = z * conj (datum->pc_phasor[ip]);
                                            // scale phasor by polarization coefficient
                        z = z * polcof;
                                            // corrections to phase as fn of freq based upon
                                            // delay calibrations

                                            // calculate offset frequency in GHz
                                            // from DC edge for this spectral point
                        deltaf = -2e-3 * i / (2e6 * param->samp_period * nlags);

                        // One size may not fit all.  The code below is a compromise
                        // between current geodetic practice and current EHT needs.
                        if (param->pc_mode[0] == MANUAL && param->pc_mode[1] == MANUAL)
                            {
                            // the correction had the wrong sign and minor O(1/nlags) error
                            // if one is trying to keep the mean phase of this channel fixed
                            phase_shift = - 1e-3 * diff_delay / (4e6 * param->samp_period);
                            phase_shift *= - (double)(nlags - 2) / (double)(nlags);
                            // per-channel phase should now be stable against delay adjustments
                            }
                        else
                            {
                                            // correction to mean phase depends on sideband
                            phase_shift = - 1e-3 * diff_delay / (4e6 * param->samp_period);
                            if (sb)
                                phase_shift = -phase_shift;
                            }
                                            // apply phase ramp to spectral points
                        std::complex<double> tmp = cexp (-2.0 * M_PI * I * (diff_delay * deltaf + phase_shift));
                        z = z * tmp;
                        xp_spec[i] += z;
                        }
                    }                       // bottom of lags loop
                }                           // bottom of polarization loop

                                            // also skip over this next section, if no data
              if ((sb == 0 && usb_present == 0) || (sb == 1 && lsb_present == 0))
                  continue;
                                            // yet another way of saying "no data"
              if ((sb == 0 && datum->usbfrac < 0) || (sb == 1 && datum->lsbfrac < 0))
                  continue;

        ////////////////////////////////////////////////////////////////////////////////
        //entirely disable these features

                                            /* apply spectral filter as needed */
             // apply_passband (sb, ap, fdata, xp_spec, nlags*2, datum);
             // apply_notches (sb, ap, fdata, xp_spec, nlags*2, datum);

                                            // apply video bandpass correction (if so instructed)
        //      if (pass->control.vbp_correct)
        //          apply_video_bp (xp_spec, nlags/2, pass);

        ////////////////////////////////////////////////////////////////////////////////

                                            // if data was filtered away...
              if ((sb == 0 && datum->usbfrac <= 0) || (sb == 1 && datum->lsbfrac <= 0))
                  continue;

                                            /* Put sidebands together.  For each sb,
                                               the Xpower array, which is the FFT across
                                               lags, is symmetrical about DC of the
                                               sampled sideband, and thus contains the
                                               (filtered out) "other" sideband, which
                                               consists primarily of noise.  Thus we only
                                               copy in half of the Xpower array
                                               Weight each sideband by data fraction */

                                            // skip 0th spectral pt if DC channel suppressed
              ibegin = (pass->control.dc_block) ? 1 : 0;
              if (sb == 0 && datum->usbfrac > 0.0)
                  {                         // USB: accumulate xp spec, no phase offset
                  for (i = ibegin; i < nlags; i++)
                      {
                      factor = datum->usbfrac;
                      S[i] += factor * xp_spec[i];
                      }
                  }
              else if (sb == 1 && datum->lsbfrac > 0.0)
                  {                         // LSB: accumulate conj(xp spec) with phase offset
                  for (i = ibegin; i < nlags; i++)
                      {
                      factor = datum->lsbfrac;
                                            // DC+highest goes into middle element of the S array
                      sindex = i ? 4 * nlags - i : 2 * nlags;
                      std::complex<double> tmp2 = cexp (I * (status->lsb_phoff[0] - status->lsb_phoff[1]));
                      S[sindex] += factor * conj (xp_spec[i] * tmp2 );
                      }
                  }
              }                             // bottom of sideband loop

                                            /* Normalize data fractions
                                               The resulting sbdelay functions which
                                               are attached to each AP from this point
                                               on reflect twice as much power in the
                                               double sideband case as in single sideband.
                                               The usbfrac and lsbfrac numbers determine
                                               a multiplicative weighting factor to be
                                               applied.  In the double sideband case, the
                                               factor of two is inherent in the data values
                                               and additional weighting should be done
                                               using the mean of usbfrac and lsbfrac */
            factor = 0.0;
            if (datum->usbfrac >= 0.0)
                factor += datum->usbfrac;
            if (datum->lsbfrac >= 0.0)
                factor += datum->lsbfrac;
            if ((datum->usbfrac >= 0.0) && (datum->lsbfrac >= 0.0))
                factor /= 4.0;              // x2 factor for sb and for polcof
                                            // correct for multiple pols being added in

            //For linear pol IXY fourfitting, make sure that we normalize for the two pols
            if( param->pol == POL_IXY)
                {
                factor *= 2.0;
                }
            else
                {
                factor *= polcof_sum; //should be 1.0 in all other cases, so this isn't really necessary
                }

            //Question:
            //why do we do this check? factor should never be negative (see above)
            //and if factor == 0, is this an error that should be flagged?
            if (factor > 0.0)
                factor = 1.0 / factor;
            //Answer:
            //if neither of usbfrac or lsbfrac was set above the default (-1), then
            //no data was seen and thus the spectral array S is here set to zero.
            //That should result in zero values for datum->sbdelay, but why take chances.

            //msg ("usbfrac %f lsbfrac %f polcof_sum %f factor %1f flag %x", -2,
            //        datum->usbfrac, datum->lsbfrac, polcof_sum, factor, datum->flag);
                                            /* Collect the results */
            if(datum->flag != 0 && factor > 0.0)
                {
                for (i=0; i<4*nlags; i++)
                    S[i] = S[i] * factor;
                                            // corrections to phase as fn of freq based upon
                                            // delay calibrations
                                            /* FFT to single-band delay */
                fftw_execute (fftplan);
                                            /* Place SB delay values in data structure */
                                            // FX correlator - use full xlag range
                for (i = 0; i < 2*nlags; i++)
                    {
                                            /* Translate so i=nlags is central lag */
                                            // skip every other (interpolated) lag
                    j = 2 * (i - nlags);
                    if (j < 0)
                        j += 4 * nlags;
                                            /* re-normalize back to single lag */
                                            /* (property of FFTs) */
                                            // nlags-1 norm. iff zeroed-out DC
                                            // factor of 2 for skipped lags
                    if (pass->control.dc_block)
                        datum->sbdelay[i] = xlag[j] / (double) (nlags / 2 - 1.0);
                    else
                        datum->sbdelay[i] = xlag[j] / (double) (nlags / 2);
                    }
                }
            else                            /* No data */
                {
                for (i = 0; i < nlags*2; i++)
                    datum->sbdelay[i] = 0.0;
                }




        };

        //private data structures to store what were 'extern'/globals
        //in the old code
        fftw_plan fftplan;
        hops_complex xp_spec[4*MAXLAG];
        hops_complex xcor[4*MAXLAG], S[4*MAXLAG], xlag[4*MAXLAG];
    }

};


























































































using namespace hops;


//read and fill-in the vex data as json and vex struct objects
bool GetVex(MHO_DirectoryInterface& dirInterface,
            MHO_MK4VexInterface& vexInterface,
            json& json_vex,
            struct vex*& root)
{
    //get list of all the files (and directories) in directory
    std::vector< std::string > allFiles;
    dirInterface.GetFileList(allFiles);
    std::string root_file;
    dirInterface.GetRootFile(allFiles, root_file);

    //convert root file ovex data to JSON, and export root/ovex ptr
    vexInterface.OpenVexFile(root_file);
    bool ovex_ok = vexInterface.ExportVexFileToJSON(json_vex);
    root = vexInterface.GetVex();
    return ovex_ok;
}
////////////////////////////////////////////////////////////////////////////////



// read a corel file and fill in old style and new style data containers
bool GetCorel(MHO_DirectoryInterface& dirInterface,
              MHO_MK4CorelInterface& corelInterface,
              const std::string& baseline,
              struct mk4_corel*& pcdata,
              ch_baseline_data_type*& ch_bl_data,
              ch_baseline_weight_type*& ch_bl_wdata
)
{
    bool corel_ok = false;
    // ch_baseline_data_type* ch_bl_data = nullptr;
    // ch_baseline_weight_type* ch_bl_wdata = nullptr;
    std::string root_file;
    std::vector< std::string > allFiles, corelFiles;
    dirInterface.GetFileList(allFiles);
    dirInterface.GetRootFile(allFiles, root_file);
    dirInterface.GetCorelFiles(allFiles, corelFiles);

    for(auto it = corelFiles.begin(); it != corelFiles.end(); it++)
    {
        std::string st_pair, root_code;
        std::string input_basename = dirInterface.GetBasename(*it);
        dirInterface.SplitCorelFileBasename(input_basename, st_pair, root_code);
        if(st_pair == baseline)
        {
            std::cout<<"found corel file: "<< *it <<std::endl;
            corelInterface.SetCorelFile(*it);
            corelInterface.SetVexFile(root_file);
            corelInterface.ExtractCorelFile();
            baseline_data_type* bl_data = corelInterface.GetExtractedVisibilities();
            baseline_weight_type* bl_wdata = corelInterface.GetExtractedWeights();

            MHO_VisibilityChannelizer channelizer;
            channelizer.SetInput(bl_data);
            ch_bl_data = new ch_baseline_data_type();
            channelizer.SetOutput(ch_bl_data);
            bool init = channelizer.Initialize();
            bool exe = false;
            if(init)
            {
                std::cout<<"initialization done"<<std::endl;
                exe = channelizer.ExecuteOperation();
                if(exe){std::cout<<"vis channelizer done"<<std::endl;}
            }

            MHO_WeightChannelizer wchannelizer;
            wchannelizer.SetInput(bl_wdata);
            ch_bl_wdata = new ch_baseline_weight_type();
            wchannelizer.SetOutput(ch_bl_wdata);
            bool winit = wchannelizer.Initialize();
            bool wexe = false;
            if(winit)
            {
                std::cout<<"initialization done"<<std::endl;
                wexe = wchannelizer.ExecuteOperation();
                if(wexe){std::cout<<"weight channelizer done"<<std::endl;}
            }

            //get the raw mk4 type data
            pcdata = corelInterface.GetCorelData();

            if(exe && wexe){corel_ok = true;}
            break;
        }
    }
    return corel_ok;
};
////////////////////////////////////////////////////////////////////////////////



//note: we normally wouldn't bother passing around two station interfaces, but
//we need to keep a pointer to the raw mk4 data, so for now keeping the interface
//classes around is the simplest way to do this with the least work
bool GetStationData(MHO_DirectoryInterface& dirInterface,
                    MHO_MK4StationInterface& refInterface,
                    MHO_MK4StationInterface& remInterface,
                    const std::string& baseline,
                    struct mk4_sdata*& ref_sdata,
                    struct mk4_sdata*& rem_sdata,
                    station_coord_data_type*& ref_stdata,
                    station_coord_data_type*& rem_stdata)
{
    std::string ref_st, rem_st, root_file;
    ref_st = baseline.at(0);
    rem_st = baseline.at(1);
    std::vector< std::string > allFiles, stationFiles;
    dirInterface.GetFileList(allFiles);
    dirInterface.GetStationFiles(allFiles, stationFiles);
    dirInterface.GetRootFile(allFiles, root_file);
    bool ref_ok = false;
    bool rem_ok = false;
    for(auto it = stationFiles.begin(); it != stationFiles.end(); it++)
    {

        std::string st, root_code;
        std::string input_basename = dirInterface.GetBasename(*it);
        dirInterface.SplitStationFileBasename(input_basename, st, root_code);

        if(st == ref_st)
        {
            std::cout<<"ref station file: "<< *it <<std::endl;
            refInterface.SetStationFile(*it);
            refInterface.SetVexFile(root_file);
            ref_stdata = refInterface.ExtractStationFile();
            ref_sdata = refInterface.GetStationData();
            ref_ok = true;
        }

        if(st == rem_st)
        {
            std::cout<<"rem station file: "<< *it <<std::endl;
            remInterface.SetStationFile(*it);
            remInterface.SetVexFile(root_file);
            rem_stdata = remInterface.ExtractStationFile();
            rem_sdata = remInterface.GetStationData();
            rem_ok = true;
        }

        if(ref_ok && rem_ok){break;}
    }

    return (ref_ok && rem_ok);

};

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
    std::string usage = "TestNormFX -i <input_directory> -b <baseline>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string input_dir;
    std::string baseline;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"input_directory", required_argument, 0, 'i'},
                                          {"baseline", required_argument, 0, 'b'}};

    static const char* optString = "hi:b:";

    while(true)
    {
        char optId = getopt_long(argc, argv, optString, longOptions, NULL);
        if (optId == -1)
            break;
        switch(optId)
        {
            case ('h'):  // help
                std::cout << usage << std::endl;
                return 0;
            case ('i'):
                input_dir = std::string(optarg);
                break;
            case ('b'):
                baseline = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    //split the baseline into reference/remote station IDs
    if(baseline.size() != 2){msg_fatal("main", "Baseline: "<<baseline<<" is not of length 2."<<eom);}

    //directory interface, load up the directory information
    MHO_DirectoryInterface dirInterface;
    dirInterface.SetCurrentDirectory(input_dir);
    dirInterface.ReadCurrentDirectory();

    //get the root (ovex) file information
    MHO_MK4VexInterface vexInterface;
    json json_vex;
    struct vex* root = nullptr;
    bool ovex_ok = GetVex(dirInterface, vexInterface, json_vex, root);

    //the corel file information for this baseline
    MHO_MK4CorelInterface corelInterface;
    struct mk4_corel* pcdata = nullptr;
    ch_baseline_data_type* ch_bl_data = nullptr;
    ch_baseline_weight_type* ch_bl_wdata = nullptr;
    bool corel_ok = GetCorel(dirInterface, corelInterface, baseline, pcdata, ch_bl_data, ch_bl_wdata);
    std::cout<<"data ptrs = "<<pcdata<<", "<<ch_bl_data<<", "<<ch_bl_wdata<<std::endl;

    //get the station data information for the ref/rem stations of this baseline
    MHO_MK4StationInterface refInterface, remInterface;
    struct mk4_sdata* ref_sdata = nullptr;
    struct mk4_sdata* rem_sdata = nullptr;
    struct mk4_sdata* sdata = new mk4_sdata[MAXSTATIONS];
    station_coord_data_type* ref_stdata = nullptr;
    station_coord_data_type* rem_stdata = nullptr;
    bool sta_ok = GetStationData(dirInterface, refInterface, remInterface, baseline, ref_sdata, rem_sdata, ref_stdata, rem_stdata);
    sdata[0] = *ref_sdata;
    sdata[1] = *rem_sdata;

////////////////////////////////////////////////////////////////////////////////
//now we need to set up the param, pass, and other data org structs

    struct type_pass pass;
    //global params (defined above as extern for linking reasons)
    // struct type_param param;
    // struct c_block *cb_head;
    // struct type_status status;

    // struct mk4_fringe fringe; //not used
    // struct type_plot plot; //not used


    param.acc_period = root->evex->ap_length;
    param.speedup = root->evex->speedup_factor;
    param.pol = POLMASK_LL;// POL_ALL;
    param.first_plot = 0;
    param.nplot_chans = 0;
    param.fmatch_bw_pct = 25.0;
    param.pc_mode[0] = MANUAL;
    param.pc_mode[1] = MANUAL;

    //control block, default
    cb_head = &(pass.control);
    cb_head->fmatch_bw_pct = 25.0;
    default_cblock(cb_head);
    set_defaults();

    struct type_pass* pass_ptr = &pass;
    pass_ptr->npols = 1;
    struct freq_corel* corel = &(pass_ptr->pass_data[0]);
    pcdata->nalloc = 0;
    fringe.nalloc = 0;
    for (int i=0; i<MAXSTATIONS; i++){sdata[i].nalloc = 0;}
    for (int i=0; i<MAXFREQ; i++){ corel[i].data_alloc = FALSE;}

    int npass = 0;
    int retval = organize_data(pcdata, root->ovex, root->ivex, sdata, corel, &param);
    int passretval = make_passes (root->ovex, corel, &param, &pass_ptr, &npass);

    std::cout<<"st1 = "<<pcdata->t100->baseline[0]<<std::endl;
    std::cout<<"st2 = "<<pcdata->t100->baseline[1]<<std::endl;
    std::cout<<"date = "<< std::string(pcdata->id->date,16)<<std::endl;
    std::cout<<"npass = "<<npass<<std::endl;
    std::cout<<"nlags = "<<param.nlags<<std::endl;
    std::cout<<"pass.pol = "<<pass.pol<<std::endl;

    //allocate space for sbdelay
    struct data_corel *datum;
    hops_complex *sbarray, *sbptr;
    int size = 2 * param.nlags * pass_ptr->nfreq * pass_ptr->num_ap;
    sbarray = (hops_complex *)calloc (size, sizeof (hops_scomplex));
    if (sbarray == NULL)
    {
        std::cout<<"mem alloc failure"<<std::endl;
        return (-1);
    }
    sbptr = sbarray;
    for (int fr=0; fr<pass_ptr->nfreq; fr++)
    {
        for (int ap=0; ap<pass_ptr->num_ap; ap++)
        {
            datum = pass_ptr->pass_data[fr].data + ap + pass_ptr->ap_off;
            datum->sbdelay = sbptr;
            sbptr += 2*param.nlags;
            for(int i=0; i<4; i++)
            {
                //we don't want to do anything with p-cal right now,
                //so just set it all to 1.0
                //(otherwise they default to zero, and nothing gets done)
                datum->pc_phasor[i][0] = 1.0;
                datum->pc_phasor[i][1] = 0.0;
            }
        }
    }

    for (int fr=0; fr<pass_ptr->nfreq; fr++)
    {
        for (int ap=0; ap<pass_ptr->num_ap; ap++)
        {
            norm_fx(&pass, &param, &status, fr, ap);
        }
    }

    std::cout<<"param.nlags = "<<param.nlags<<std::endl;
    for (int fr=0; fr<pass_ptr->nfreq; fr++)
    {
        for (int ap=0; ap<pass_ptr->num_ap; ap++)
        {
            datum = pass_ptr->pass_data[fr].data + ap + pass_ptr->ap_off;
            for(int i=0; i < 2*param.nlags; i++)
            {
                std::cout<<"datum @ "<<i<<" = "<<datum->sbdelay[i]<<std::endl;
            }
        }
    }







    return 0;
}
