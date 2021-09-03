#include "MHO_NormFX.hh"

namespace hops
{

//#define USE_OLD


MHO_NormFX::MHO_NormFX():
    fInitialized(false),
    fIsUSB(true)
{};

MHO_NormFX::~MHO_NormFX(){};


bool
MHO_NormFX::Initialize()
{
    fInitialized = false;
    if(this->fInput1 != nullptr && this->fInput2 != nullptr && this->fOutput != nullptr)
    {

        bool status = true;
        //figure out if we have USB or LSB data (or a mixture)
        auto* channel_axis = &(std::get<CH_CHANNEL_AXIS>( *(this->fInput1) ) );
        std::size_t n_usb_chan = channel_axis->GetNIntervalsWithKeyValue(std::string("net_sideband"), 'U');
        std::size_t n_lsb_chan = channel_axis->GetNIntervalsWithKeyValue(std::string("net_sideband"), 'L');
        if(n_usb_chan != 0){fIsUSB = true;}
        if(n_lsb_chan != 0){fIsUSB = false;}
        if(n_usb_chan != 0 && n_lsb_chan != 0)
        {
            msg_error("operators", "Could not initialize MHO_NormFX, mixed USB/LSB data not yet supported." << eom);
            return false;
        }

        this->fInput1->GetDimensions(fInDims);
        this->fOutput->GetDimensions(fOutDims);

        //check that the output dimensions are correct
        if(fInDims[CH_POLPROD_AXIS] != fOutDims[CH_POLPROD_AXIS]){status = false;}
        if(fInDims[CH_CHANNEL_AXIS] != fOutDims[CH_CHANNEL_AXIS]){status = false;}
        if(fInDims[CH_TIME_AXIS] != fOutDims[CH_TIME_AXIS]){status = false;}
        if(4*fInDims[CH_FREQ_AXIS] != fOutDims[CH_FREQ_AXIS]){status = false;}
        if(!status){msg_error("operators", "Could not initialize MHO_NormFX, in/out dimension mis-match." << eom); return false;}

        std::size_t nlags = fInDims[CH_FREQ_AXIS]; //in the original norm_fx, nlags is 2x this number

        //temp fWorkspace
        this->fOutput->GetDimensions(fWorkDims);
        fWorkDims[CH_FREQ_AXIS] *= 2;
        fWorkspace.Resize(fWorkDims);
        fWorkspace.SetArray(std::complex<double>(0.0,0.0));

        fNaNBroadcaster.SetFunctor(&fNaNMasker);
        fNaNBroadcaster.SetInput(this->fInput1);
        fNaNBroadcaster.SetOutput(this->fInput1);
        status = fNaNBroadcaster.Initialize();
        if(!status){msg_error("operators", "Could not initialize NaN mask broadcast in MHO_NormFX." << eom); return false;}

        fConjBroadcaster.SetFunctor(&fConjugator);
        fConjBroadcaster.SetInput(this->fInput1);
        fConjBroadcaster.SetOutput(this->fInput1);
        status = fConjBroadcaster.Initialize();
        if(!status){msg_error("operators", "Could not initialize complex conjugation broadcast in MHO_NormFX." << eom); return false;}

        fPaddedFFTEngine.SetInput(this->fInput1);
        fPaddedFFTEngine.SetOutput(&fWorkspace);
        fPaddedFFTEngine.DeselectAllAxes();
        fPaddedFFTEngine.SelectAxis(CH_FREQ_AXIS); //only perform padded fft on frequency (to lag) axis
        fPaddedFFTEngine.SetForward();//forward DFT
        fPaddedFFTEngine.SetPaddingFactor(8);

        //TODO FIXME...currently this treats all channels as USB or LSB (but what if we have a mixed case?)
        //for LSB data we flip as well as pad
        if(fIsUSB){fPaddedFFTEngine.SetEndPadded();}
        else{fPaddedFFTEngine.SetReverseEndPadded();}

        status = fPaddedFFTEngine.Initialize();
        if(!status){msg_error("operators", "Could not initialize padded FFT in MHO_NormFX." << eom); return false;}

        fSubSampler.SetDimensionStride(CH_FREQ_AXIS, 2);
        fSubSampler.SetInput(&fWorkspace);
        fSubSampler.SetOutput(this->fOutput);
        status = fSubSampler.Initialize();
        if(!status){msg_error("operators", "Could not initialize sub-sampler in MHO_NormFX." << eom); return false;}

        fCyclicRotator.SetOffset(CH_FREQ_AXIS, 2*nlags);
        fCyclicRotator.SetInput(this->fOutput);
        fCyclicRotator.SetOutput(this->fOutput);
        status = fCyclicRotator.Initialize();
        if(!status){msg_error("operators", "Could not initialize cyclic rotation in MHO_NormFX." << eom); return false;}

        //normalize the array
        double norm =  1.0/(double)fInDims[CH_FREQ_AXIS];
        fScalarMultiplier.SetFactor(norm);
        fNormBroadcaster.SetFunctor(&fScalarMultiplier);
        fNormBroadcaster.SetInput(this->fOutput);
        fNormBroadcaster.SetOutput(this->fOutput);
        status = fNormBroadcaster.Initialize();
        if(!status){msg_error("operators", "Could not initialize MHO_NormFX." << eom); return false;}

        fInitialized = true;
    }

    return false;

}




bool
MHO_NormFX::ExecuteOperation()
{
    if(fInitialized)
    {

        //apply phase-cal corrections to each channel (manuals only)
        //for now do nothing

        //apply differential delay corrections (phase ramp) to each channel (manuals only)
        //for now do nothing

        //sum over all relevant polarization-products and side-bands
        //for now only select the first polarization

        bool status;

        //first thing we do is filter out any NaNs
        //(ADHOC flagging would likely also be implemented in a similar fashion)
        status = fNaNBroadcaster.ExecuteOperation();
        if(!status){msg_error("operators", "Could not execute NaN masker MHO_NormFX." << eom); return false;}

        //for lower sideband we complex conjugate the data
        if(!fIsUSB)
        {
            status = fConjBroadcaster.ExecuteOperation();
            if(!status){msg_error("operators", "Could not execute complex conjugation in MHO_NormFX." << eom); return false;}
        }

    #ifdef USE_OLD
        run_old_normfx_core();
    #else

        status = fPaddedFFTEngine.ExecuteOperation();
        if(!status){msg_error("operators", "Could not execute paddded FFT in MHO_NormFX." << eom); return false;}

        status = fSubSampler.ExecuteOperation();
        if(!status){msg_error("operators", "Could not execute sub-sampler in MHO_NormFX." << eom); return false;}

        status = fCyclicRotator.ExecuteOperation();
        if(!status){msg_error("operators", "Could not execute cyclic-rotation MHO_NormFX." << eom); return false;}

    #endif

        status = fNormBroadcaster.ExecuteOperation();
        if(!status){msg_error("operators", "Could not execute normalization in MHO_NormFX." << eom); return false;}


        return true;
    }

    return false;
};




void MHO_NormFX::run_old_normfx_core()
{
    std::size_t npp = fInDims[CH_POLPROD_AXIS];
    std::size_t nchan = fInDims[CH_CHANNEL_AXIS];
    std::size_t naps = fInDims[CH_TIME_AXIS];
    std::size_t nlags = fInDims[CH_FREQ_AXIS];


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

    msg_debug("operators", "Using old norm_fx basic code."<<eom);

    for(std::size_t fr=0; fr<nchan; fr++)
    {
        for(std::size_t ap=0; ap<naps; ap++)
        {
            for (int i=0; i<4*nlags; i++){xp_spec[i] = 0.0;}
            for (int i=0; i<4*nlags; i++){S[i] = 0.0;}

            for(std::size_t pp=0; pp<1; pp++) //loop over pol-products (select and/or add)
            {
                for (int i=0; i<nlags/2; i++)
                {
                    z = this->fInput1->at(pp,fr,ap,i);
                    z = z * polcof;
                    xp_spec[i] += z;
                }
            }

            //lower-sideband data
            for(int i = 0; i < nlags; i++)
            {
                factor = 1.0;// datum->lsbfrac;

                //LSB
                // DC+highest goes into middle element of the S array
                // int sindex = 4*nlags - i;
                // if(i==0){sindex = 2*nlags;}
                //USB
                int sindex;

                if(fIsUSB)
                {
                    sindex = i;
                }
                else
                {
                    sindex = 4*nlags - i;
                    if(i==0){sindex = 2*nlags;}
                }

                //int sindex = i ? 4 * nlags - i : 2 * nlags;
                //std::complex<double> tmp2 = std::exp (I_complex * (status->lsb_phoff[0] - status->lsb_phoff[1]));
                //S[sindex] += factor * std::conj (xp_spec[i] );// * tmp2 );

//                S[sindex] += factor * std::conj (xp_spec[i] );
                S[sindex] += factor * xp_spec[i];
            }

            //for (int i=0; i<4*nlags; i++){S[i] = S[i] * factor;}

            fFFTEngine.ExecuteOperation();

            // corrections to phase as fn of freq based upon
            // delay calibrations
            // FFT to single-band delay
            //fftw_execute (fftplan);
            // Place SB delay values in data structure
            // FX correlator - use full xlag range
            for (int i = 0; i < 2*nlags; i++)
            {
                // Translate so i=nlags is central lag
                // skip every other (interpolated) lag
                int j = 2 * (i - nlags);
                if (j < 0){j += 4 * nlags;}
                this->fOutput->at(0,fr,ap,i) = xlag[j] ; // (double) (nlags / 2);
                //this->fOutput->at(0,fr,ap,i) = xlag[j] / (double) (nlags / 2);
            }

        }
    }
}







































































































//
//
// bool
// MHO_NormFX::ExecuteOperation()
// {
//     //apply phase-cal corrections to each channel (manuals only)
//     //for now do nothing
//
//     //apply differential delay corrections (phase ramp) to each channel (manuals only)
//     //for now do nothing
//
//     //sum over all relevant polarization-products and side-bands
//     //for now only select the first polarization
//
//
//     bool status;
//     std::size_t fDims[CH_VIS_NDIM];
//     this->fInput1->GetDimensions(fDims);
//
//     std::size_t npp = fDims[CH_POLPROD_AXIS];
//     std::size_t nchan = fDims[CH_CHANNEL_AXIS];
//     std::size_t naps = fDims[CH_TIME_AXIS];
//     std::size_t nlags = fDims[CH_FREQ_AXIS];
//     double polcof = 1.0;
//     double usbfrac = 0.0;
//     double lsbfrac = 1.0;
//     double factor = 1.0;
//
//     //double it
//     nlags *= 2;
//     std::complex<double> z;
//
//     xp_spec.Resize(4*nlags);
//     S.Resize(4*nlags);
//     xlag.Resize(4*nlags);
//
//     fFFTEngine.SetInput(&S);
//     fFFTEngine.SetOutput(&xlag);
//     fFFTEngine.SetForward();
//     fFFTEngine.Initialize();
//
//     //insert a NaN for testing
//     //this->fInput1->at(0,0,0,0) = std::complex<double>(1.0, 0.0/0.0);
//
//     //first thing we do is filter out any NaNs
//     //(ADHOC flagging would likely also be implemented in a similar fashion)
//     MHO_NaNMasker<ch_baseline_data_type, ch_baseline_data_type> fNaNMasker;
//     MHO_FunctorBroadcaster<ch_baseline_data_type, ch_baseline_data_type> fNaNBroadcaster;
//     fNaNBroadcaster.SetFunctor(&fNaNMasker);
//     fNaNBroadcaster.SetInput(this->fInput1);
//     fNaNBroadcaster.SetOutput(this->fInput1);
//     status = fNaNBroadcaster.Initialize();
//     status = fNaNBroadcaster.ExecuteOperation();
//
//
//     for(std::size_t fr=0; fr<nchan; fr++)
//     {
//         for(std::size_t ap=0; ap<naps; ap++)
//         {
//
//             for (int i=0; i<4*nlags; i++){xp_spec[i] = 0.0;}
//             for (int i=0; i<4*nlags; i++){S[i] = 0.0;}
//
//             for(std::size_t pp=0; pp<1; pp++) //loop over pol-products (select and/or add)
//             {
//                 for (int i=0; i<nlags/2; i++)
//                 {
//                     //Should filter out NaNs at some point
//
//                     // add in iff this is a requested pol product (currently hard coded polprod=0)
//                     //HERE WE ARE TAKING THE VISIBILITIES FROM THE NEW DATA CONTAINERS, previous was t120->ld.spec[i]
//                     z = this->fInput1->at(pp,fr,ap,i);
//
//                     //APPLY pcal here
//
//                     // scale phasor by polarization coefficient
//                     z = z * polcof;
//
//                     //APPLY delay phase ramp here
//
//                     xp_spec[i] += z;
//                 }
//             }
//
//             /* Put sidebands together.  For each sb,
//             the Xpower array, which is the FFT across
//             lags, is symmetrical about DC of the
//             sampled sideband, and thus contains the
//             (filtered out) "other" sideband, which
//             consists primarily of noise.  Thus we only
//             copy in half of the Xpower array
//             Weight each sideband by data fraction */
//
//             // // skip 0th spectral pt if DC channel suppressed
//             // ibegin = (pass->control.dc_block) ? 1 : 0;
//             // if (sb == 0 && datum->usbfrac > 0.0)
//             // {                         // USB: accumulate xp spec, no phase offset
//             //     for (i = 0; i < nlags; i++)
//             //     {
//             //         factor = datum->usbfrac;
//             //         S[i] += factor * xp_spec[i];
//             //     }
//             // }
//             // else if (sb == 1 && datum->lsbfrac > 0.0)
//             //{
//
//             //lower-sideband data
//             for(int i = 0; i < nlags; i++)
//             {
//                 factor = 1.0;// datum->lsbfrac;
//                 // DC+highest goes into middle element of the S array
//                 int sindex = i ? 4 * nlags - i : 2 * nlags;
//                 //sstd::complex<double> tmp2 = std::exp (I_complex * (status->lsb_phoff[0] - status->lsb_phoff[1]));
//                 S[sindex] += factor * std::conj (xp_spec[i] );// * tmp2 );
//             }
//             //}
//
//
//
//
//
//
//
//             // /* Normalize data fractions
//             // The resulting sbdelay functions which
//             // are attached to each AP from this point
//             // on reflect twice as much power in the
//             // double sideband case as in single sideband.
//             // The usbfrac and lsbfrac numbers determine
//             // a multiplicative weighting factor to be
//             // applied.  In the double sideband case, the
//             // factor of two is inherent in the data values
//             // and additional weighting should be done
//             // using the mean of usbfrac and lsbfrac */
//             // factor = 0.0;
//             // if (datum->usbfrac >= 0.0){factor += datum->usbfrac;}
//             // if (datum->lsbfrac >= 0.0){factor += datum->lsbfrac;}
//             // if ((datum->usbfrac >= 0.0) && (datum->lsbfrac >= 0.0)){factor /= 4.0;}             // x2 factor for sb and for polcof
//             // // correct for multiple pols being added in
//             //
//             // //For linear pol IXY fourfitting, make sure that we normalize for the two pols
//             // if( param->pol == POL_IXY)
//             // {
//             //     factor *= 2.0;
//             // }
//             // else
//             // {
//             //     factor *= polcof_sum; //should be 1.0 in all other cases, so this isn't really necessary
//             // }
//             //
//             // //Question:
//             // //why do we do this check? factor should never be negative (see above)
//             // //and if factor == 0, is this an error that should be flagged?
//             // if (factor > 0.0){factor = 1.0 / factor;}
//             // //Answer:
//             // //if neither of usbfrac or lsbfrac was set above the default (-1), then
//             // //no data was seen and thus the spectral array S is here set to zero.
//             // //That should result in zero values for datum->sbdelay, but why take chances.
//             //
//             // //msg ("usbfrac %f lsbfrac %f polcof_sum %f factor %1f flag %x", -2,
//             // //        datum->usbfrac, datum->lsbfrac, polcof_sum, factor, datum->flag);
//             // /* Collect the results */
//             // if(datum->flag != 0 && factor > 0.0)
//             // {
//
//
//             for (int i=0; i<4*nlags; i++){S[i] = S[i] * factor;}
//
//             fFFTEngine.ExecuteOperation();
//
//             // corrections to phase as fn of freq based upon
//             // delay calibrations
//             /* FFT to single-band delay */
//             //fftw_execute (fftplan);
//             /* Place SB delay values in data structure */
//             // FX correlator - use full xlag range
//             for (int i = 0; i < 2*nlags; i++)
//             {
//                 /* Translate so i=nlags is central lag */
//                 // skip every other (interpolated) lag
//                 int j = 2 * (i - nlags);
//                 if (j < 0){j += 4 * nlags;}
//                 /* re-normalize back to single lag */
//                 /* (property of FFTs) */
//                 // nlags-1 norm. iff zeroed-out DC
//                 // factor of 2 for skipped lags
//                 // if (pass->control.dc_block)
//                 // {
//                 //     datum->sbdelay[i][0] = xlag[j].real() / (double) (nlags / 2 - 1.0);
//                 //     datum->sbdelay[i][1] = xlag[j].imag() / (double) (nlags / 2 - 1.0);
//                 // }
//                 // else
//
//                 this->fOutput->at(0,fr,ap,i) = xlag[j] / (double) (nlags / 2);
//                 // {
//                 //     datum->sbdelay[i][0] = xlag[j].real() / (double) (nlags / 2);
//                 //     datum->sbdelay[i][1] = xlag[j].imag() / (double) (nlags / 2);
//                 // }
//             }
//
//         }
//     }
//
//     return true;
// };
//
//
//
//
//
//
//











}//end of namespace














    //weight results by lsb/usb frac and copy into x-form array

    //execute FFT, normalize
