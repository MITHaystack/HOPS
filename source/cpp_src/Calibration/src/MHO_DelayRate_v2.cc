#include "MHO_DelayRate_v2.hh"

namespace hops
{

MHO_DelayRate_v2::MHO_DelayRate_v2():
    fInitialized(false)
{};

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

        //out->Copy(*in1);
        out->GetDimensions(fOutDims);

        //borrow this stupid routine from search_windows.c /////////////////////
        std::size_t drsp_size = 8192;
        while ( (drsp_size / 4) > fInDims[TIME_AXIS] ) {drsp_size /= 2;};
        std::cout<<"DRSP size = "<<drsp_size<<std::endl;
        ////////////////////////////////////////////////////////////////////////

        std::size_t np = drsp_size*4;
        ConditionallyResizeOutput(&(fInDims[0]), np, out);

        fPaddedFFTEngine.SetArgs(&fWorkspace, out);
        fPaddedFFTEngine.DeselectAllAxes();
        fPaddedFFTEngine.SelectAxis(TIME_AXIS); //only perform padded fft on frequency (to lag) axis
        fPaddedFFTEngine.SetForward();//forward DFT
        fPaddedFFTEngine.SetPaddedSize(np);
        fPaddedFFTEngine.SetEndPadded();//pretty sure this is the default from delay_rate.c
        
        status = fPaddedFFTEngine.Initialize();
        if(!status){msg_error("operators", "Could not initialize padded FFT in MHO_DelayRate_v2." << eom); return false;}

        fCyclicRotator.SetOffset(TIME_AXIS, np/2);
        fCyclicRotator.SetArgs(out);
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
                    printf("frac( %d, %d)  = %f \n", ch, ap,  (*in2)(pp, ch, ap, 0) );
                    fWorkspace.SliceView(pp, ch, ap, ":") *= (*in2)(pp, ch, ap, 0);
                }
            }
        }


        //std::size_t nap = fInDims[TIME_AXIS];
        out->ZeroArray();

        //copy the data into sbd_dr_data
        // std::size_t nap = fInDims[TIME_AXIS];
        // out->ZeroArray();
        // for(std::size_t ap=0; ap<nap; ap++)
        // {
        //     out->SliceView(":", ":", ap, ":").Copy( in->SliceView(":",":",ap,":") );
        // }

        bool ok = fPaddedFFTEngine.Execute();
        check_step_fatal(ok, "calibration", "fft engine execution." << eom );
        
        
        ok = fCyclicRotator.Execute();
        check_step_fatal(ok, "calibration", "cyclic rotation execution." << eom );
        
        
        
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
