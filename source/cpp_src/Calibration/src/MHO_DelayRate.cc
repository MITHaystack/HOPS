#include "MHO_DelayRate.hh"

namespace hops
{

MHO_DelayRate::MHO_DelayRate():
    fInitialized(false)
{};

MHO_DelayRate::~MHO_DelayRate(){};


bool
MHO_DelayRate::InitializeOutOfPlace(const XArgType* in, XArgType* out)
{

    fInitialized = false;
    if(in != nullptr && out != nullptr)
    {
        bool status = true;

        in->GetDimensions(fInDims);
        out->Copy(*in);
        out->GetDimensions(fOutDims);

        //borrow this stupid routine from search_windows.c
        std::size_t drsp_size = 8192;
        while ( (drsp_size / 4) > fInDims[CH_TIME_AXIS] ) {drsp_size /= 2;};
        std::cout<<"DRSP size = "<<drsp_size<<std::endl;

        std::size_t np = drsp_size*4;
        ConditionallyResizeOutput(&(fInDims[0]), np, out);

        // fPaddedFFTEngine.SetArgs(in, out);
        // fPaddedFFTEngine.DeselectAllAxes();
        // fPaddedFFTEngine.SelectAxis(CH_TIME_AXIS); //only perform padded fft on frequency (to lag) axis
        // fPaddedFFTEngine.SetForward();//forward DFT
        // fPaddedFFTEngine.SetPaddedSize(np);
        // fPaddedFFTEngine.SetEndPadded();//pretty sure this is the default from delay_rate.c
        // 
        // status = fPaddedFFTEngine.Initialize();
        // if(!status){msg_error("operators", "Could not initialize padded FFT in MHO_DelayRate." << eom); return false;}

        // fSubSampler.SetDimensionAndStride(CH_FREQ_AXIS, 2);
        // fSubSampler.SetArgs(&fWorkspace, out);
        // status = fSubSampler.Initialize();
        // if(!status){msg_error("operators", "Could not initialize sub-sampler in MHO_DelayRate." << eom); return false;}

        fCyclicRotator.SetOffset(CH_TIME_AXIS, np/2);
        fCyclicRotator.SetArgs(out);
        status = fCyclicRotator.Initialize();
        if(!status){msg_error("operators", "Could not initialize cyclic rotation in MHO_DelayRate." << eom); return false;}

        fInitialized = true;
    }

    return fInitialized;

}




bool
MHO_DelayRate::ExecuteOutOfPlace(const XArgType* in1, XArgType* out)
{

    if(fInitialized)
    {

        // //xform in the time (AP) axis to look for delay/fringe rate
        // //output for the delay
        // ch_visibility_type* sbd_dr_data = sbd_data->CloneEmpty();
        // sbd_dr_data->ZeroArray();
        // sbd_data->GetDimensions(bl_dim);
        // std::size_t nap = bl_dim[CH_TIME_AXIS];
        // bl_dim[CH_TIME_AXIS] = drsp_size;
        // sbd_dr_data->Resize(bl_dim);

        //std::get<CH_CHANNEL_AXIS>(*sbd_dr_data).CopyIntervalLabels( std::get<CH_CHANNEL_AXIS>(*bl_data) );

        //copy the data into sbd_dr_data
        std::size_t nap = fInDims[CH_TIME_AXIS];
        out->ZeroArray();
        // for(std::size_t ap=0; ap<nap; ap++)
        // {
        //     out->SliceView(":", ":", ap, ":").Copy( in->SliceView(":",":",ap,":") );
        // }

        bool ok = fPaddedFFTEngine.Execute();
        check_step_fatal(ok, "calibration", "fft engine execution." << eom );
        ok = fCyclicRotator.Execute();
        check_step_fatal(ok, "calibration", "cyclic rotation execution." << eom );


        return true;
    }

    return false;
};




void 
MHO_DelayRate::ConditionallyResizeOutput(const std::size_t* dims,
                               std::size_t size,
                               XArgType* out)
{
    auto out_dim = out->GetDimensionArray();
    bool have_to_resize = false;
    for(std::size_t i=0; i<XArgType::rank::value; i++)
    {
        if(i == CH_TIME_AXIS)
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
