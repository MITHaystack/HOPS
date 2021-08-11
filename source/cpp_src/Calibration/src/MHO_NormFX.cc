#include "MHO_NormFX.hh"

#define signum(a) (a>=0 ? 1.0 : -1.0)

#define CIRC_MODE 0
#define LIN_MODE 1
#define MIXED_MODE 2
#define MAXFREQ 64

int fcode (char c, char *codes)
	{
	int i;

	for (i = 0; i < MAXFREQ; i++)
		if (c == codes[i])
            return i;
    return -1;
	}



namespace hops
{


MHO_NormFX::MHO_NormFX(){};

MHO_NormFX::~MHO_NormFX(){};

bool
MHO_NormFX::Initialize()
{
    //check dimensions on the input arrays match
    return true;

}

bool
MHO_NormFX::ExecuteOperation()
{
    //apply phase-cal corrections to each channel (manuals only)
    //for now do nothing

    //apply differential delay corrections (phase ramp) to each channel (manuals only)
    //for now do nothing

    //sum over all relevant polarization-products and side-bands 
    //for now only select the first polarization

    MHO_NDArrayWrapper< std::complex<double>, 1 > xp_spec;
    MHO_NDArrayWrapper< std::complex<double>, 1 > S;
    MHO_NDArrayWrapper< std::complex<double>, 1 > xlag;

    std::size_t dims[CH_VIS_NDIM];
    this->fInput1->GetDimensions(dims);

    std::size_t npp = dims[CH_POLPROD_AXIS];
    std::size_t nchan = dims[CH_CHANNEL_AXIS];
    std::size_t naps = dims[CH_TIME_AXIS];
    std::size_t nlags = dims[CH_FREQ_AXIS];
    double polcof = 1.0;
    double usbfrac = 0.0;
    double lsbfrac = 1.0;
    double factor = 1.0;

    //double it
    nlags *= 2;
    std::complex<double> z;

    xp_spec.Resize(4*nlags);
    S.Resize(4*nlags);
    xlag.Resize(4*nlags);



    fFFTEngine.SetInput(&S);
    fFFTEngine.SetOutput(&xlag);
    fFFTEngine.SetForward();
    fFFTEngine.Initialize();

    for(std::size_t fr=0; fr<nchan; fr++)
    {
        for(std::size_t ap=0; ap<naps; ap++)
        {
            for(std::size_t pp=0; pp<1; pp++) //loop over pol-products (select and/or add)
            {
                for (int i=0; i<nlags/2; i++)
                {
                    //Should filter out NaNs at some point

                    // add in iff this is a requested pol product (currently hard coded polprod=0)
                    //HERE WE ARE TAKING THE VISIBILITIES FROM THE NEW DATA CONTAINERS, previous was t120->ld.spec[i]
                    z = this->fInput1->at(pp,fr,ap,i);

                    //APPLY pcal here

                    // scale phasor by polarization coefficient
                    z = z * polcof;

                    //APPLY delay phase ramp here

                    xp_spec[i] += z;
                }
            }

            /* Put sidebands together.  For each sb,
            the Xpower array, which is the FFT across
            lags, is symmetrical about DC of the
            sampled sideband, and thus contains the
            (filtered out) "other" sideband, which
            consists primarily of noise.  Thus we only
            copy in half of the Xpower array
            Weight each sideband by data fraction */

            // // skip 0th spectral pt if DC channel suppressed
            // ibegin = (pass->control.dc_block) ? 1 : 0;
            // if (sb == 0 && datum->usbfrac > 0.0)
            // {                         // USB: accumulate xp spec, no phase offset
            //     for (i = 0; i < nlags; i++)
            //     {
            //         factor = datum->usbfrac;
            //         S[i] += factor * xp_spec[i];
            //     }
            // }
            // else if (sb == 1 && datum->lsbfrac > 0.0)
            //{

            //lower-sideband data
            for(int i = 0; i < nlags; i++)
            {
                factor = 1.0;// datum->lsbfrac;
                // DC+highest goes into middle element of the S array
                int sindex = i ? 4 * nlags - i : 2 * nlags;
                //sstd::complex<double> tmp2 = std::exp (I_complex * (status->lsb_phoff[0] - status->lsb_phoff[1]));
                S[sindex] += factor * std::conj (xp_spec[i] );// * tmp2 );
            }
            //}





        

            // /* Normalize data fractions
            // The resulting sbdelay functions which
            // are attached to each AP from this point
            // on reflect twice as much power in the
            // double sideband case as in single sideband.
            // The usbfrac and lsbfrac numbers determine
            // a multiplicative weighting factor to be
            // applied.  In the double sideband case, the
            // factor of two is inherent in the data values
            // and additional weighting should be done
            // using the mean of usbfrac and lsbfrac */
            // factor = 0.0;
            // if (datum->usbfrac >= 0.0){factor += datum->usbfrac;}
            // if (datum->lsbfrac >= 0.0){factor += datum->lsbfrac;}
            // if ((datum->usbfrac >= 0.0) && (datum->lsbfrac >= 0.0)){factor /= 4.0;}             // x2 factor for sb and for polcof
            // // correct for multiple pols being added in
            // 
            // //For linear pol IXY fourfitting, make sure that we normalize for the two pols
            // if( param->pol == POL_IXY)
            // {
            //     factor *= 2.0;
            // }
            // else
            // {
            //     factor *= polcof_sum; //should be 1.0 in all other cases, so this isn't really necessary
            // }
            // 
            // //Question:
            // //why do we do this check? factor should never be negative (see above)
            // //and if factor == 0, is this an error that should be flagged?
            // if (factor > 0.0){factor = 1.0 / factor;}
            // //Answer:
            // //if neither of usbfrac or lsbfrac was set above the default (-1), then
            // //no data was seen and thus the spectral array S is here set to zero.
            // //That should result in zero values for datum->sbdelay, but why take chances.
            // 
            // //msg ("usbfrac %f lsbfrac %f polcof_sum %f factor %1f flag %x", -2,
            // //        datum->usbfrac, datum->lsbfrac, polcof_sum, factor, datum->flag);
            // /* Collect the results */
            // if(datum->flag != 0 && factor > 0.0)
            // {


            for (int i=0; i<4*nlags; i++){S[i] = S[i] * factor;}

            fFFTEngine.ExecuteOperation();

            // corrections to phase as fn of freq based upon
            // delay calibrations
            /* FFT to single-band delay */
            //fftw_execute (fftplan);
            /* Place SB delay values in data structure */
            // FX correlator - use full xlag range
            for (int i = 0; i < 2*nlags; i++)
            {
                /* Translate so i=nlags is central lag */
                // skip every other (interpolated) lag
                int j = 2 * (i - nlags);
                if (j < 0){j += 4 * nlags;}
                /* re-normalize back to single lag */
                /* (property of FFTs) */
                // nlags-1 norm. iff zeroed-out DC
                // factor of 2 for skipped lags
                // if (pass->control.dc_block)
                // {
                //     datum->sbdelay[i][0] = xlag[j].real() / (double) (nlags / 2 - 1.0);
                //     datum->sbdelay[i][1] = xlag[j].imag() / (double) (nlags / 2 - 1.0);
                // }
                // else

                this->fOutput->at(0,fr,ap,i) = xlag[j] / (double) (nlags / 2);
                // {
                //     datum->sbdelay[i][0] = xlag[j].real() / (double) (nlags / 2);
                //     datum->sbdelay[i][1] = xlag[j].imag() / (double) (nlags / 2);
                // }
            }

        }
    }

    return true;
};



}//end of namespace














    //weight results by lsb/usb frac and copy into x-form array 

    //execute FFT, normalize
























