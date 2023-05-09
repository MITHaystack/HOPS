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
        auto* channel_axis = &(std::get<CHANNEL_AXIS>( *(in1) ) );
        std::size_t n_usb_chan = channel_axis->GetNIntervalsWithKeyValue(std::string("net_sideband"), std::string("U"));
        std::size_t n_lsb_chan = channel_axis->GetNIntervalsWithKeyValue(std::string("net_sideband"), std::string("L"));
        if(n_usb_chan != 0){fIsUSB = true;}
        if(n_lsb_chan != 0){fIsUSB = false;}


        if(!fIsUSB){msg_debug("operators", "MHO_NormFX operating on LSB data, N LSB channels: " << n_lsb_chan <<eom );}
        else{msg_debug("operators", "MHO_NormFX operating on USB data, N USB channels: " << n_usb_chan <<eom );}

        if(n_usb_chan != 0 && n_lsb_chan != 0)
        {
            msg_error("operators", "Could not initialize MHO_NormFX, mixed USB/LSB data not yet supported." << eom);
            return false;
        }

        in1->GetDimensions(fInDims);
        out->GetDimensions(fOutDims);

        //check that the output dimensions are correct
        if(fInDims[POLPROD_AXIS] != fOutDims[POLPROD_AXIS]){status = false;}
        if(fInDims[CHANNEL_AXIS] != fOutDims[CHANNEL_AXIS]){status = false;}
        if(fInDims[TIME_AXIS] != fOutDims[TIME_AXIS]){status = false;}
        if(4*fInDims[FREQ_AXIS] != fOutDims[FREQ_AXIS]){status = false;}
        if(!status){msg_error("operators", "Could not initialize MHO_NormFX, in/out dimension mis-match." << eom); return false;}

        std::size_t nlags = fInDims[FREQ_AXIS]; //in the original norm_fx, nlags is 2x this number

        //temp fWorkspace
        out->GetDimensions(fWorkDims);
        fWorkDims[FREQ_AXIS] *= 2;
        fWorkspace.Resize(fWorkDims);
        fWorkspace.SetArray(std::complex<double>(0.0,0.0));

        #pragma message("TODO FIXME, the following line casts away const-ness:")
        fNaNBroadcaster.SetArgs( const_cast<XArgType1*>(in1) );
        status = fNaNBroadcaster.Initialize();
        if(!status){msg_error("operators", "Could not initialize NaN mask broadcast in MHO_NormFX." << eom); return false;}

        fPaddedFFTEngine.SetArgs(in1, &fWorkspace);
        fPaddedFFTEngine.DeselectAllAxes();
        fPaddedFFTEngine.SelectAxis(FREQ_AXIS); //only perform padded fft on frequency (to lag) axis
        fPaddedFFTEngine.SetForward();//forward DFT
        fPaddedFFTEngine.SetPaddingFactor(8);

        //TODO FIXME...currently this treats all channels as USB or LSB (but what if we have a mixed case?)
        //for LSB data we flip as well as pad
        // if(!fIsUSB){fPaddedFFTEngine.SetEndPadded();}
        // else{fPaddedFFTEngine.SetReverseEndPadded();}

        fPaddedFFTEngine.SetEndPadded();


        status = fPaddedFFTEngine.Initialize();
        if(!status){msg_error("operators", "Could not initialize padded FFT in MHO_NormFX." << eom); return false;}

        fSubSampler.SetDimensionAndStride(FREQ_AXIS, 2);
        fSubSampler.SetArgs(&fWorkspace, out);
        status = fSubSampler.Initialize();
        if(!status){msg_error("operators", "Could not initialize sub-sampler in MHO_NormFX." << eom); return false;}

        fCyclicRotator.SetOffset(FREQ_AXIS, 2*nlags);
        fCyclicRotator.SetArgs(out);
        status = fCyclicRotator.Initialize();
        if(!status){msg_error("operators", "Could not initialize cyclic rotation in MHO_NormFX." << eom); return false;}

        // double norm = 1.0/(nlags*8);
        // fNormBroadcaster.GetFunctor()->SetFactor(norm);
        // fNormBroadcaster.SetInput(out);
        // fNormBroadcaster.SetOutput(out);
        // status = fNormBroadcaster.Initialize();
        // if(!status){msg_error("operators", "Could not initialize MHO_NormFX." << eom); return false;}

        //#pragma message("TODO FIXME, the following line casts away const-ness:")
        fConjBroadcaster.SetArgs( out );
        status = fConjBroadcaster.Initialize();
        if(!status){msg_error("operators", "Could not initialize complex conjugation broadcast in MHO_NormFX." << eom); return false;}

        //double it
        nlags *= 2;

        fInitialized = true;
    }

    return fInitialized;

}




bool
MHO_NormFX::ExecuteImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out)
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
        status = fNaNBroadcaster.Execute();
        if(!status){msg_error("operators", "Could not execute NaN masker MHO_NormFX." << eom); return false;}



    #ifdef USE_OLD
        run_old_normfx_core(in1, in2, out);
    #else

        status = fPaddedFFTEngine.Execute();
        if(!status){msg_error("operators", "Could not execute paddded FFT in MHO_NormFX." << eom); return false;}

        // auto ax2 = &(std::get<TIME_AXIS>(fWorkspace));
        // std::cout<<"workspace ax2(1) = "<<(*ax2)(1)<<std::endl;

        status = fSubSampler.Execute();
        if(!status){msg_error("operators", "Could not execute sub-sampler in MHO_NormFX." << eom); return false;}

        // auto ax2b = &(std::get<TIME_AXIS>(*out));
        // std::cout<<"workspace ax2b(1) = "<<(*ax2b)(1)<<std::endl;

        status = fCyclicRotator.Execute();
        if(!status){msg_error("operators", "Could not execute cyclic-rotation MHO_NormFX." << eom); return false;}

        // auto ax2c = &(std::get<TIME_AXIS>(*out));
        // std::cout<<"workspace ax2c(1) = "<<(*ax2c)(1)<<std::endl;

        //for lower sideband we complex conjugate the data
        if(!fIsUSB)
        {
            status = fConjBroadcaster.Execute();
            if(!status){msg_error("operators", "Could not execute complex conjugation in MHO_NormFX." << eom); return false;}
        }

    #endif

        //normalize the array
        double norm =  1.0/(double)fInDims[FREQ_AXIS];
        //double norm =  1.0/(double) out->GetDimension(FREQ_AXIS);
        // std::cout<<"the norm = "<< fInDims[FREQ_AXIS]<<std::endl;
        *(out) *= norm;

        // status = fNormBroadcaster.Execute();
        // if(!status){msg_error("operators", "Could not execute normalization in MHO_NormFX." << eom); return false;}

        // fWorkspace.Resize(0,0,0,0);

        return true;
    }

    return false;
};


}//end of namespace
