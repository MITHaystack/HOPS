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
MHO_NormFX::InitializeImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out)
{
    fInitialized = false;
    if(in1 != nullptr && in2 != nullptr && out != nullptr)
    {

        bool status = true;
        //figure out if we have USB or LSB data (or a mixture)
        auto* channel_axis = &(std::get<CH_CHANNEL_AXIS>( *(in1) ) );
        std::size_t n_usb_chan = channel_axis->GetNIntervalsWithKeyValue(std::string("net_sideband"), 'U');
        std::size_t n_lsb_chan = channel_axis->GetNIntervalsWithKeyValue(std::string("net_sideband"), 'L');
        if(n_usb_chan != 0){fIsUSB = true;}
        if(n_lsb_chan != 0){fIsUSB = false;}
        if(n_usb_chan != 0 && n_lsb_chan != 0)
        {
            msg_error("operators", "Could not initialize MHO_NormFX, mixed USB/LSB data not yet supported." << eom);
            return false;
        }

        in1->GetDimensions(fInDims);
        out->GetDimensions(fOutDims);

        //check that the output dimensions are correct
        if(fInDims[CH_POLPROD_AXIS] != fOutDims[CH_POLPROD_AXIS]){status = false;}
        if(fInDims[CH_CHANNEL_AXIS] != fOutDims[CH_CHANNEL_AXIS]){status = false;}
        if(fInDims[CH_TIME_AXIS] != fOutDims[CH_TIME_AXIS]){status = false;}
        if(4*fInDims[CH_FREQ_AXIS] != fOutDims[CH_FREQ_AXIS]){status = false;}
        if(!status){msg_error("operators", "Could not initialize MHO_NormFX, in/out dimension mis-match." << eom); return false;}

        std::size_t nlags = fInDims[CH_FREQ_AXIS]; //in the original norm_fx, nlags is 2x this number

        //temp fWorkspace
        out->GetDimensions(fWorkDims);
        fWorkDims[CH_FREQ_AXIS] *= 2;
        fWorkspace.Resize(fWorkDims);
        fWorkspace.SetArray(std::complex<double>(0.0,0.0));

        #pragma message("TODO FIXME, the following line casts away const-ness:")
        fNaNBroadcaster.SetArgs( const_cast<XArgType1*>(in1) );
        status = fNaNBroadcaster.Initialize();
        if(!status){msg_error("operators", "Could not initialize NaN mask broadcast in MHO_NormFX." << eom); return false;}

        #pragma message("TODO FIXME, the following line casts away const-ness:")
        fConjBroadcaster.SetArgs( const_cast<XArgType1*>(in1) );
        status = fConjBroadcaster.Initialize();
        if(!status){msg_error("operators", "Could not initialize complex conjugation broadcast in MHO_NormFX." << eom); return false;}

        fPaddedFFTEngine.SetArgs(in1, &fWorkspace);
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

        fSubSampler.SetDimensionAndStride(CH_FREQ_AXIS, 2);
        fSubSampler.SetArgs(&fWorkspace, out);
        status = fSubSampler.Initialize();
        if(!status){msg_error("operators", "Could not initialize sub-sampler in MHO_NormFX." << eom); return false;}

        fCyclicRotator.SetOffset(CH_FREQ_AXIS, 2*nlags);
        fCyclicRotator.SetArgs(out);
        status = fCyclicRotator.Initialize();
        if(!status){msg_error("operators", "Could not initialize cyclic rotation in MHO_NormFX." << eom); return false;}

        // fNormBroadcaster.GetFunctor()->SetFactor(norm);
        // fNormBroadcaster.SetInput(out);
        // fNormBroadcaster.SetOutput(out);
        // status = fNormBroadcaster.Initialize();
        // if(!status){msg_error("operators", "Could not initialize MHO_NormFX." << eom); return false;}

        //double it
        nlags *= 2;
        xp_spec.Resize(4*nlags);
        S.Resize(4*nlags);
        xlag.Resize(4*nlags);

        fFFTEngine.SetArgs(&S, &xlag);
        fFFTEngine.SetForward();
        fFFTEngine.Initialize();

        fInitialized = true;
    }

    return fInitialized;

}




bool
MHO_NormFX::ExecuteImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out)
{

    if(fInitialized)
    {


        std::cout<<"in norm fx"<<std::endl;
        //apply phase-cal corrections to each channel (manuals only)
        //for now do nothing

        //apply differential delay corrections (phase ramp) to each channel (manuals only)
        //for now do nothing

        //sum over all relevant polarization-products and side-bands
        //for now only select the first polarization

        bool status;

        //first thing we do is filter out any NaNs
        //(ADHOC flagging would likely also be implemented in a similar fashion)
        status = fNaNBroadcaster.Execute();
        if(!status){msg_error("operators", "Could not execute NaN masker MHO_NormFX." << eom); return false;}

        //for lower sideband we complex conjugate the data
        if(!fIsUSB)
        {
            status = fConjBroadcaster.Execute();
            if(!status){msg_error("operators", "Could not execute complex conjugation in MHO_NormFX." << eom); return false;}
        }

    #ifdef USE_OLD
        run_old_normfx_core(in1, in2, out);
    #else

        status = fPaddedFFTEngine.Execute();
        if(!status){msg_error("operators", "Could not execute paddded FFT in MHO_NormFX." << eom); return false;}

        status = fSubSampler.Execute();
        if(!status){msg_error("operators", "Could not execute sub-sampler in MHO_NormFX." << eom); return false;}

        status = fCyclicRotator.Execute();
        if(!status){msg_error("operators", "Could not execute cyclic-rotation MHO_NormFX." << eom); return false;}

    #endif

        //normalize the array
        double norm =  1.0/(double)fInDims[CH_FREQ_AXIS];
        *(out) *= norm;

        // status = fNormBroadcaster.Execute();
        // if(!status){msg_error("operators", "Could not execute normalization in MHO_NormFX." << eom); return false;}

        std::cout<<"done"<<std::endl;

        return true;
    }

    return false;
};




void MHO_NormFX::run_old_normfx_core(const XArgType1* in1, const XArgType2* in2, XArgType3* out)
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
                    z = (*(in1))(pp,fr,ap,i);
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

            fFFTEngine.Execute();

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
                (*(out))(0,fr,ap,i) = xlag[j] ; // (double) (nlags / 2);
                //out->at(0,fr,ap,i) = xlag[j] / (double) (nlags / 2);
            }

        }
    }
}



}//end of namespace
