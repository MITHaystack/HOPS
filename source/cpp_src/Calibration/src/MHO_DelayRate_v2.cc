#include "MHO_DelayRate_v2.hh"

#include <math.h>

namespace hops
{

MHO_DelayRate_v2::MHO_DelayRate_v2():
    fInitialized(false)
{
    fRefFreq = 1.0;
};

MHO_DelayRate_v2::~MHO_DelayRate_v2(){};

bool
MHO_DelayRate_v2::InitializeImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out)
{

    fInitialized = false;
    if(in1 != nullptr && in2 != nullptr && out != nullptr)
    {
        bool status = true;

        in1->GetDimensions(fInDims);
        
        //copy the input data into the workspace 
        fWorkspace.Copy(*in1);

        //borrow this stupid routine from search_windows.c /////////////////////
        fDRSPSize = 8192;
        while ( (fDRSPSize / 4) > fInDims[TIME_AXIS] ) {fDRSPSize /= 2;};
        std::cout<<"DRSP size = "<<fDRSPSize<<std::endl;
        ////////////////////////////////////////////////////////////////////////

        std::size_t np = fDRSPSize*4;
        ConditionallyResizeOutput(&(fInDims[0]), np, &fWorkspace2);

        fPaddedFFTEngine.SetArgs(&fWorkspace, &fWorkspace2);
        fPaddedFFTEngine.DeselectAllAxes();
        fPaddedFFTEngine.SelectAxis(TIME_AXIS); //only perform padded fft on frequency (to lag) axis
        fPaddedFFTEngine.SetForward();//forward DFT
        fPaddedFFTEngine.SetPaddedSize(np);
        fPaddedFFTEngine.SetEndPadded();//pretty sure this is the default from delay_rate.c
        
        status = fPaddedFFTEngine.Initialize();
        if(!status){msg_error("operators", "Could not initialize padded FFT in MHO_DelayRate_v2." << eom); return false;}

        fCyclicRotator.SetOffset(TIME_AXIS, np/2);
        fCyclicRotator.SetArgs(&fWorkspace2);
        status = fCyclicRotator.Initialize();
        if(!status){msg_error("operators", "Could not initialize cyclic rotation in MHO_DelayRate_v2." << eom); return false;}

        // fSubSampler.SetDimensionAndStride(TIME_AXIS, 2);
        // fSubSampler.SetArgs(out);
        // status = fSubSampler.Initialize();
        // if(!status){msg_error("operators", "Could not initialize sub-sampler in MHO_DelayRate_v2." << eom); return false;}


        fInitialized = true;
    }

    return fInitialized;

}




bool
MHO_DelayRate_v2::ExecuteImpl(const XArgType1* in1, const XArgType2* in2, XArgType3* out)
{

    if(fInitialized)
    {

        // //xform in the time (AP) axis to look for delay/fringe rate
        // //output for the delay
        // ch_visibility_type* sbd_dr_data = sbd_data->CloneEmpty();
        // sbd_dr_data->ZeroArray();
        // sbd_data->GetDimensions(bl_dim);
        // std::size_t nap = bl_dim[TIME_AXIS];
        // bl_dim[TIME_AXIS] = drsp_size;
        // sbd_dr_data->Resize(bl_dim);

        //std::get<CHANNEL_AXIS>(*sbd_dr_data).CopyIntervalLabels( std::get<CHANNEL_AXIS>(*bl_data) );


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
                    fWorkspace.SliceView(pp, ch, ap, ":") *= (*in2)(pp, ch, ap, 0);
                }
            }
        }


        //std::size_t nap = fInDims[TIME_AXIS];
        out->ZeroArray();

        bool ok = fPaddedFFTEngine.Execute();
        check_step_fatal(ok, "calibration", "fft engine execution." << eom );
        
        
        ok = fCyclicRotator.Execute();
        check_step_fatal(ok, "calibration", "cyclic rotation execution." << eom );

        //linear interpolation, and conversion from fringe rate to delay rate step
        int sz = 4*fDRSPSize;
        std::size_t nsbd = fWorkspace2.GetDimension(FREQ_AXIS);
        out->Copy(fWorkspace2);
        out->Resize(pprod, nch, fDRSPSize, nsbd);       
        out->ZeroArray();
        
        std::cout<<"bahhh "<<pprod<<", "<< nch << ", " << fDRSPSize << ", " << nsbd <<std::endl;
        
        for(std::size_t pp=0; pp<pprod; pp++)
        {
            for(std::size_t ch=0; ch<nch; ch++)
            {
                double chan_freq = (std::get<CHANNEL_AXIS>(*in1) )(ch);
                double b = ( (chan_freq / fRefFreq) * sz) / fDRSPSize;
                std::cout<<"b = "<<b<<std::endl;
                for(std::size_t sbd=0; sbd<nsbd; sbd++)
                {
                    for(std::size_t dr=0; dr<fDRSPSize; dr++)
                    {
                        double num = ( (double)dr - (double)(fDRSPSize/2) ) * b + ( (double)sz * 1.5);
                        double l_fp = fmod(  num , (double) sz) ;
                        //printf("L, num, l_fp = %d, %f, %f \n ", dr, num, l_fp);
                        int l_int = (int)l_fp;
                        int l_int2 = l_int+1;
                        if (l_int < 0){ l_int = 0; }
                        if (l_int2 > (sz-1)){ l_int2 = sz - 1;}
                        sbd_type::value_type interp_val = fWorkspace2(pp, ch, l_int, sbd) * (1.0 - l_fp + l_int) + fWorkspace2(pp, ch, l_int2, sbd) * (l_fp - l_int);
                        // if(l_int ==0 && sbd == 0)
                        // {
                        //     std::cout<<"L = "<<dr<<std::endl;
                        //     std::cout<<"l_fp = "<<l_fp<<std::endl;
                        //     std::cout<<"p1 = "<<fWorkspace2(pp, ch, l_int, sbd)<<std::endl;
                        //     std::cout<<"p2 = "<<fWorkspace2(pp, ch, l_int2, sbd)<<std::endl;
                        //     std::cout<<"result = "<<interp_val<<std::endl;
                        // }

                        // rate_spectrum[L] = fringe_spect[l_int] * (1.0 - l_fp + l_int)
                        //                      + fringe_spect[l_int2] * (l_fp - l_int);
                        // 
                        
                        
                        (*out)(pp, ch, dr, sbd) = interp_val;
                    }
                }
            }
        }
        
        
        
        //we need a step here equivalent to the odd interpolation that delay_rate.c does like this:
        
        // for (L = 0; L < np; L++)
        //     {
        //     l_fp = fmod ((L - (np/2) ) * b + (size * 1.5) , (double)size) ;
        //     l_int = (int)l_fp;
        //     l_int2 = l_int+1;
        //     if (l_int < 0) l_int = 0;
        //     if (l_int2 > (size-1)) l_int2 = size - 1;
        //     rate_spectrum[L] = fringe_spect[l_int] * (1.0 - l_fp + l_int)
        //                      + fringe_spect[l_int2] * (l_fp - l_int);
        // 
        // 
        
        
        
        
        

        // ok = fSubSampler.Execute();
        // check_step_fatal(ok, "calibration", "sub sample execution." << eom );
        // 
        // //normalize the array
        // double norm =  1.0/(double) out->GetDimension(TIME_AXIS);
        // *(out) *= norm;

        return true;
    }

    return false;
};




void 
MHO_DelayRate_v2::ConditionallyResizeOutput(const std::size_t* dims,
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
