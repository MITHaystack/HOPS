#include "MHO_DelayRate.hh"

#include <math.h>

namespace hops
{

MHO_DelayRate::MHO_DelayRate():
    fInitialized(false)
{
    fRefFreq = 1.0;
};

MHO_DelayRate::~MHO_DelayRate(){};

bool
MHO_DelayRate::InitializeImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out)
{

    fInitialized = false;
    if(in1 != nullptr && in2 != nullptr && out != nullptr)
    {
        bool ok = true;

        in1->GetDimensions(fInDims);

        //copy the input data into the workspace
        fWorkspace.Copy(*in1);

        //borrow this stupid routine from search_windows.c /////////////////////
        #pragma message("Fix the DRSP size calculation to remove upper limit of 8192.")
        fDRSPSize = 8192;
        while ( (fDRSPSize / 4) > fInDims[TIME_AXIS] ) {fDRSPSize /= 2;};
        msg_debug("calibration", "delay rate search space size = "<< fDRSPSize << eom );
        ////////////////////////////////////////////////////////////////////////

        std::size_t np = fDRSPSize*4;
        ConditionallyResizeOutput(&(fInDims[0]), np, &fWorkspace2);

        #ifdef TOGGLE_SWITCH

        fZeroPadder.SetArgs(&fWorkspace, &fWorkspace2);
        fZeroPadder.DeselectAllAxes();
        fZeroPadder.SelectAxis(TIME_AXIS); //only pad on the frequency (to lag) axis
        fZeroPadder.SetPaddedSize(np);
        fZeroPadder.SetEndPadded();

        fFFTEngine.SetArgs(&fWorkspace2);
        fFFTEngine.DeselectAllAxes();
        fFFTEngine.SelectAxis(TIME_AXIS); //only perform padded fft on frequency (to lag) axis
        fFFTEngine.SetForward();//forward DFT

        #else

        fPaddedFFTEngine.SetArgs(&fWorkspace, &fWorkspace2);
        fPaddedFFTEngine.DeselectAllAxes();
        fPaddedFFTEngine.SelectAxis(TIME_AXIS); //only perform padded fft on frequency (to lag) axis
        fPaddedFFTEngine.SetForward();//forward DFT
        fPaddedFFTEngine.SetPaddedSize(np);
        fPaddedFFTEngine.SetEndPadded(); //for both LSB and USB (what about DSB?)

        #endif

        // fPaddedFFTEngine.SetArgs(&fWorkspace, &fWorkspace2);
        // fPaddedFFTEngine.DeselectAllAxes();
        // fPaddedFFTEngine.SelectAxis(TIME_AXIS);
        // fPaddedFFTEngine.SetForward();//forward DFT
        // fPaddedFFTEngine.SetPaddedSize(np);
        // fPaddedFFTEngine.SetEndPadded();//pretty sure this is the default from delay_rate.c

        #ifdef TOGGLE_SWITCH

        ok = fZeroPadder.Initialize();
        if(!ok){msg_error("operators", "Could not initialize zero padder in MHO_DelayRate" << eom); return false;}

        ok = fFFTEngine.Initialize();
        if(!ok){msg_error("operators", "Could not initialize FFT in MHO_DelayRate" << eom); return false;}

        #else

        ok = fPaddedFFTEngine.Initialize();
        if(!ok){msg_error("operators", "Could not initialize padded FFT in MHO_DelayRate" << eom); return false;}

        #endif

        // ok = fPaddedFFTEngine.Initialize();
        // if(!ok){msg_error("operators", "Could not initialize padded FFT in MHO_DelayRate." << eom); return false;}

        fCyclicRotator.SetOffset(TIME_AXIS, np/2);
        fCyclicRotator.SetArgs(&fWorkspace2);
        ok = fCyclicRotator.Initialize();
        if(!ok){msg_error("operators", "Could not initialize cyclic rotation in MHO_DelayRate." << eom); return false;}

        fInitialized = true;
    }

    return fInitialized;

}




bool
MHO_DelayRate::ExecuteImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out)
{

    if(fInitialized)
    {
        //apply the data weights to the data in fWorkspace
        std::size_t pprod = fWorkspace.GetDimension(POLPROD_AXIS);
        std::size_t nch = fWorkspace.GetDimension(CHANNEL_AXIS);
        std::size_t nap = fWorkspace.GetDimension(TIME_AXIS);

        double time_delta = std::get<TIME_AXIS>(*in1)(1) -  std::get<TIME_AXIS>(*in1)(0);

        ApplyDataWeights(in2);

        //std::size_t nap = fInDims[TIME_AXIS];
        out->ZeroArray();
        bool ok;
        #ifdef TOGGLE_SWITCH

        ok = fZeroPadder.Execute();
        if(!ok){msg_error("operators", "Could not execute zero padder in MHO_DelayRate" << eom); return false;}

        ok = fFFTEngine.Execute();
        if(!ok){msg_error("operators", "Could not execute FFT in MHO_DelayRate" << eom); return false;}

        #else

        ok = fPaddedFFTEngine.Execute();
        if(!ok){msg_error("operators", "Could not execute FFT in MHO_DelayRate" << eom); return false;}

        #endif
        // bool ok = fPaddedFFTEngine.Execute();
        // check_step_fatal(ok, "calibration", "fft engine execution." << eom );

        ok = fCyclicRotator.Execute();
        check_step_fatal(ok, "calibration", "cyclic rotation execution." << eom );

        //linear interpolation, and conversion from fringe rate to delay rate step
        int sz = 4*fDRSPSize;
        std::size_t nsbd = fWorkspace2.GetDimension(FREQ_AXIS);
        out->Copy(fWorkspace2);
        out->Resize(pprod, nch, fDRSPSize, nsbd);
        out->ZeroArray();

        for(std::size_t pp=0; pp<pprod; pp++)
        {
            for(std::size_t ch=0; ch<nch; ch++)
            {
                double chan_freq = (std::get<CHANNEL_AXIS>(*in1) )(ch);
                double b = ( (chan_freq / fRefFreq) * sz) / fDRSPSize;

                for(std::size_t sbd=0; sbd<nsbd; sbd++)
                {
                    for(std::size_t dr=0; dr<fDRSPSize; dr++)
                    {
                        double num = ( (double)dr - (double)(fDRSPSize/2) ) * b + ( (double)sz * 1.5);
                        double l_fp = fmod(  num , (double) sz) ;
                        int l_int = (int)l_fp;
                        if (l_int < 0){ l_int = 0; }
                        int l_int2 = l_int+1;
                        //std::cout<<sz<<":"<<l_int<<": "<<l_int2<<std::endl;
                        if (l_int2 > (sz-1)){ l_int2 = sz - 1;}
                        //std::cout<<sz<<":"<<l_int<<": "<<l_int2<<std::endl;
                        sbd_type::value_type interp_val = fWorkspace2(pp, ch, l_int, sbd) * (1.0 - l_fp + l_int) + fWorkspace2(pp, ch, l_int2, sbd) * (l_fp - l_int);
                        (*out)(pp, ch, dr, sbd) = interp_val;

                        std::get<TIME_AXIS>(*out)(dr) = ( (double)dr - (double)(fDRSPSize/2) )*(1.0/(time_delta*(double)fDRSPSize) );
                        //assign the axis value along dr axis
                        //std::get<TIME_AXIS>(*out)(dr) = std::get<TIME_AXIS>(fWorkspace2)(l_int) * (1.0 - l_fp + l_int) + std::get<TIME_AXIS>(fWorkspace2)(l_int2) * (l_fp - l_int);
                    }
                }
            }
        }

        return true;
    }

    return false;
};

void
MHO_DelayRate::ApplyDataWeights(const XArgType2* in2)
{
    //apply the data weights to the data in fWorkspace
    std::size_t pprod = fWorkspace.GetDimension(POLPROD_AXIS);
    std::size_t nch = fWorkspace.GetDimension(CHANNEL_AXIS);
    std::size_t nap = fWorkspace.GetDimension(TIME_AXIS);

    for(std::size_t pp=0; pp<pprod; pp++)
    {
        for(std::size_t ch=0; ch<nch; ch++)
        {
            for(std::size_t ap=0; ap<nap; ap++)
            {
                fWorkspace.SliceView(pp, ch, ap, ":") *= (*in2)(pp, ch, ap, 0); //apply the data weights
            }
        }
    }

}


void
MHO_DelayRate::ConditionallyResizeOutput(const std::size_t* dims,
                               std::size_t size,
                               XArgType3* out)
{
    auto out_dim = out->GetDimensionArray();
    bool have_to_resize = false;
    for(std::size_t i=0; i<XArgType3::rank::value; i++)
    {
        if(i == TIME_AXIS)
        {
            if(out_dim[i] != size)
            {
                have_to_resize = true;
                out_dim[i] = size;
            }
        }
        else
        {
            if(dims[i] != out_dim[i])
            {
                have_to_resize = true;
                out_dim[i] = dims[i];
            }
        }
    }
    if(have_to_resize){ out->Resize( &(out_dim[0]) );}
}



}//end of namespace
