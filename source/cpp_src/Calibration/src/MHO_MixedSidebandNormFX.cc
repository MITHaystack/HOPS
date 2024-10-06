#include "MHO_MixedSidebandNormFX.hh"

namespace hops
{

MHO_MixedSidebandNormFX::MHO_MixedSidebandNormFX(): fInitialized(false){};

MHO_MixedSidebandNormFX::~MHO_MixedSidebandNormFX(){};

bool MHO_MixedSidebandNormFX::InitializeOutOfPlace(const XArgType* in, XArgType* out)
{
    fInitialized = false;
    if(in != nullptr && out != nullptr)
    {
        bool status = true;
        //figure out if we have USB or LSB data (or a mixture)
        auto channel_axis = &(std::get< CHANNEL_AXIS >(*(in)));

        std::string sb_key = "net_sideband";
        std::string usb_flag = "U";
        std::string lsb_flag = "L";
        auto usb_chan = channel_axis->GetMatchingIndexes(sb_key, usb_flag);
        auto lsb_chan = channel_axis->GetMatchingIndexes(sb_key, lsb_flag);

        std::size_t n_usb_chan = usb_chan.size();
        std::size_t n_lsb_chan = lsb_chan.size();
        if(n_lsb_chan != 0)
        {
            msg_debug("calibration", "MHO_MixedSidebandNormFX operating on LSB data, N LSB channels: " << n_lsb_chan << eom);
        }
        if(n_usb_chan != 0)
        {
            msg_debug("calibration", "MHO_MixedSidebandNormFX operating on USB data, N USB channels: " << n_usb_chan << eom);
        }

        //mixed sideband data should be ok, but warn user since it is not well tested
        if(n_usb_chan != 0 && n_lsb_chan != 0)
        {
            msg_warn("calibration", "support for data with mixed USB/LSB is experimental" << eom);
        }

        std::vector< mho_json > dsb_labels = channel_axis->GetMatchingIntervalLabels("double_sideband");
        std::size_t n_dsb_chan = dsb_labels.size();
        if(n_dsb_chan != 0)
        {
            msg_error("calibration", "MHO_MixedSidebandNormFX discovered: "
                                         << n_dsb_chan << " double-sideband channels, this data type is not yet supported"
                                         << eom);
            // return false;
        }

        //allocate the SBD space
        std::size_t sbd_dim[visibility_type::rank::value];
        in->GetDimensions(sbd_dim);
        sbd_dim[FREQ_AXIS] *= 4; //normfx implementation demands this
        out->Resize(sbd_dim);
        out->ZeroArray();

        //copy all axes but sub-channel frequency
        std::get< POLPROD_AXIS >(*out).Copy(std::get< POLPROD_AXIS >(*in));
        std::get< CHANNEL_AXIS >(*out).Copy(std::get< CHANNEL_AXIS >(*in));
        std::get< TIME_AXIS >(*out).Copy(std::get< TIME_AXIS >(*in));

        in->GetDimensions(fInDims);
        out->GetDimensions(fOutDims);

        //check that the output dimensions are correct
        if(fInDims[POLPROD_AXIS] != fOutDims[POLPROD_AXIS])
        {
            status = false;
        }
        if(fInDims[CHANNEL_AXIS] != fOutDims[CHANNEL_AXIS])
        {
            status = false;
        }
        if(fInDims[TIME_AXIS] != fOutDims[TIME_AXIS])
        {
            status = false;
        }
        if(4 * fInDims[FREQ_AXIS] != fOutDims[FREQ_AXIS])
        {
            status = false;
        }
        if(!status)
        {
            msg_error("calibration", "Could not initialize MHO_MixedSidebandNormFX, in/out dimension mis-match." << eom);
            return false;
        }

        std::size_t nlags = fInDims[FREQ_AXIS]; //in the original norm_fx, nlags is 2x this number

        //temp fWorkspace
        out->GetDimensions(fWorkDims);
        fWorkDims[FREQ_AXIS] *= 2;
        fWorkspace.Resize(fWorkDims);
        fWorkspace.SetArray(std::complex< double >(0.0, 0.0));

        fZeroPadder.SetArgs(in, &fWorkspace);
        fZeroPadder.DeselectAllAxes();
        fZeroPadder.SelectAxis(FREQ_AXIS); //only pad on the frequency (to lag) axis
        fZeroPadder.SetPaddingFactor(8);
        fZeroPadder.SetEndPadded(); //for both LSB and USB (what about DSB?)

        fNaNBroadcaster.SetArgs(&fWorkspace);
        status = fNaNBroadcaster.Initialize();
        if(!status)
        {
            msg_error("calibration", "Could not initialize NaN mask broadcast in MHO_MixedSidebandNormFX." << eom);
            return false;
        }

        fFFTEngine.SetArgs(&fWorkspace);
        fFFTEngine.DeselectAllAxes();
        fFFTEngine.SelectAxis(FREQ_AXIS); //only perform padded fft on frequency (to lag) axis
        fFFTEngine.SetForward();          //forward DFT

        status = fZeroPadder.Initialize();
        if(!status)
        {
            msg_error("calibration", "Could not initialize zero padder in MHO_MixedSidebandNormFX." << eom);
            return false;
        }

        status = fFFTEngine.Initialize();
        if(!status)
        {
            msg_error("calibration", "Could not initialize FFT in MHO_MixedSidebandNormFX." << eom);
            return false;
        }

        fSubSampler.SetDimensionAndStride(FREQ_AXIS, 2);
        fSubSampler.SetArgs(&fWorkspace, out);
        status = fSubSampler.Initialize();
        if(!status)
        {
            msg_error("calibration", "Could not initialize sub-sampler in MHO_MixedSidebandNormFX." << eom);
            return false;
        }

        fCyclicRotator.SetOffset(FREQ_AXIS, 2 * nlags);
        fCyclicRotator.SetArgs(out);
        status = fCyclicRotator.Initialize();
        if(!status)
        {
            msg_error("calibration", "Could not initialize cyclic rotation in MHO_MixedSidebandNormFX." << eom);
            return false;
        }

        //double it
        nlags *= 2;

        fInitialized = true;
    }

    return fInitialized;
}

bool MHO_MixedSidebandNormFX::ExecuteOutOfPlace(const XArgType* in, XArgType* out)
{

    if(fInitialized)
    {
        bool status;

        // status = fZeroPadder.Execute();
        // if(!status)
        // {
        //     msg_error("calibration", "Could not execute zero padder in MHO_MixedSidebandNormFX." << eom);
        //     return false;
        // }

        FillWorkspace(in, &fWorkspace);

        //first thing we do is filter out any NaNs
        //(ADHOC flagging would likely also be implemented in a similar fashion)
        status = fNaNBroadcaster.Execute();
        if(!status)
        {
            msg_error("calibration", "Could not execute NaN masker MHO_MixedSidebandNormFX." << eom);
            return false;
        }


        status = fFFTEngine.Execute();
        if(!status)
        {
            msg_error("calibration", "Could not execute FFT in MHO_MixedSidebandNormFX." << eom);
            return false;
        }

        // 
        // //invert the application of data weights
        // if(this->fWeights != nullptr)
        // {
        //     status = ApplyWeights(&fWorkspace, this->fWeights, true);
        //     if(!status)
        //     {
        //         msg_error("calibration", "could not apply weights in MHO_MixedSidebandNormFX" << eom);
        //         return false;
        //     }
        // }
        // else 
        // {
        //     msg_warn("calibration", "no visibility data weights available for MHO_MixedSidebandNormFX" << eom);
        // }

        status = fSubSampler.Execute();
        if(!status)
        {
            msg_error("calibration", "Could not execute sub-sampler in MHO_MixedSidebandNormFX." << eom);
            return false;
        }

        status = fCyclicRotator.Execute();
        if(!status)
        {
            msg_error("calibration", "Could not execute cyclic-rotation MHO_MixedSidebandNormFX." << eom);
            return false;
        }

        //normalize the array (due to FFT)
        double norm = 1.0 / (double)fInDims[FREQ_AXIS];
        *(out) *= norm;

        return true;
    }

    return false;
};

bool MHO_MixedSidebandNormFX::InitializeInPlace(XArgType* in)
{
    XArgType* tmp = new XArgType();
    bool status = InitializeOutOfPlace(in, tmp);
    in->Copy(*tmp);
    delete tmp;
    return status;
}

bool MHO_MixedSidebandNormFX::ExecuteInPlace(XArgType* in)
{
    XArgType* tmp = new XArgType();
    bool status = ExecuteOutOfPlace(in, tmp);
    in->Copy(*tmp);
    delete tmp;
    return status;
}

// void 
// MHO_MixedSidebandNormFX::FillWorkspace(const visibility_type* in, visibility_type* workspace)
// {
//     bool status = fZeroPadder.Execute();
//     if(!status)
//     {
//         msg_error("calibration", "Could not execute zero padder in MHO_MixedSidebandNormFX." << eom);
//     }
//     workspace->ZeroArray();
// 
//     //ok...now the real copy 
//     auto pp_ax = &(std::get<POLPROD_AXIS>(*in));
//     auto chan_ax = &(std::get<CHANNEL_AXIS>(*in));
//     auto ap_ax = &(std::get<TIME_AXIS>(*in));
//     auto freq_ax = &(std::get<FREQ_AXIS>(*in));
// 
//     auto in_freq_ax = &(std::get<FREQ_AXIS>(*in));
//     auto out_freq_ax = &(std::get<FREQ_AXIS>(*workspace));
//     std::size_t lsb_shift = out_freq_ax->GetSize()/2;
//     double eps = 1e-4;
// 
//     for(std::size_t pp =0; pp<pp_ax->GetSize(); pp++)
//     {
//         for(std::size_t ch=0; ch<chan_ax->GetSize(); ch++)
//         {
//             std::string net_sideband;
//             bool net_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", net_sideband);
//             for(std::size_t ap =0; ap<ap_ax->GetSize(); ap++)
//             {
//                 for(std::size_t fr=0; fr<freq_ax->GetSize(); fr++)
//                 {
//                     if(net_sideband == "L")
//                     {
//                         workspace->at(pp,ch,ap, lsb_shift - fr) +=  std::conj(in->at(pp,ch,ap,fr) );
//                     }
//                     if(net_sideband == "U")
//                     {
//                         workspace->at(pp,ch,ap,fr) += in->at(pp,ch,ap,fr);
//                     }
//                 }
//             }
//         }
//     }
// }


void 
MHO_MixedSidebandNormFX::FillWorkspace(const visibility_type* in, visibility_type* workspace)
{
    bool status = fZeroPadder.Execute();
    if(!status)
    {
        msg_error("calibration", "Could not execute zero padder in MHO_MixedSidebandNormFX." << eom);
    }
    workspace->ZeroArray();

    //ok...now the real copy 
    auto pp_ax = &(std::get<POLPROD_AXIS>(*in));
    auto chan_ax = &(std::get<CHANNEL_AXIS>(*in));
    auto ap_ax = &(std::get<TIME_AXIS>(*in));
    auto freq_ax = &(std::get<FREQ_AXIS>(*in));

    auto in_freq_ax = &(std::get<FREQ_AXIS>(*in));
    auto out_freq_ax = &(std::get<FREQ_AXIS>(*workspace));
    std::size_t lsb_shift = out_freq_ax->GetSize()/2;

    for(std::size_t pp =0; pp<pp_ax->GetSize(); pp++)
    {
        for(std::size_t ch=0; ch<chan_ax->GetSize(); ch++)
        {
            std::string net_sideband;
            bool net_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", net_sideband);
            for(std::size_t ap =0; ap<ap_ax->GetSize(); ap++)
            {
                for(std::size_t fr=0; fr<freq_ax->GetSize(); fr++)
                {
                    if(net_sideband == "L")
                    {
                        //conjugate, place with offset in reverse
                        workspace->at(pp,ch,ap, lsb_shift - fr) += std::conj(in->at(pp,ch,ap,fr) );
                    }

                    if(net_sideband == "U")
                    {
                        workspace->at(pp,ch,ap,fr) += in->at(pp,ch,ap,fr);
                    }
                }
            }
        }
    }
}


void 
MHO_MixedSidebandNormFX::TreatDoubleSidebandChannels(const visibility_type* in, visibility_type* workspace)
{
    // //ok...now the real copy 
    // auto pp_ax = &(std::get<POLPROD_AXIS>(*in));
    // auto chan_ax = &(std::get<CHANNEL_AXIS>(*in));
    // auto ap_ax = &(std::get<TIME_AXIS>(*in));
    // auto freq_ax = &(std::get<FREQ_AXIS>(*in));
    // 
    // auto in_freq_ax = &(std::get<FREQ_AXIS>(*in));
    // auto out_freq_ax = &(std::get<FREQ_AXIS>(*workspace));
    // std::size_t lsb_shift = out_freq_ax->GetSize()/2;
    // double eps = 1e-4;
    // 
    // for(std::size_t pp =0; pp<pp_ax->GetSize(); pp++)
    // {
    //     for(std::size_t ch=0; ch<chan_ax->GetSize(); ch++)
    //     {
    //         std::string net_sideband;
    //         bool net_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", net_sideband);
    //         int dsb_partner = 0;
    //         bool dsb_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "dsb_partner", dsb_partner);
    //         if(!dsb_present){dsb_present = 0;}
    //         else if(net_sideband == "L"){dsb_partner = 1;}
    //         else if(net_sideband == "U"){dsb_partner = -1;}
    //         int other = ch + dsb_partner;
    // 
    //         //check if the partner is out of bounds
    //         if(other < 0){other = ch;}
    //         if(other >= chan_ax->GetSize() - 1 ){other = ch;}
    // 
    //         //check that the sky freqs are the same (just to sure the partner wasn't cut)
    //         if(other != ch)
    //         {
    //             double f1, f2;
    //             bool tmp1 = chan_ax->RetrieveIndexLabelKeyValue(ch, "sky_freq", f1);
    //             bool tmp2 = chan_ax->RetrieveIndexLabelKeyValue(other, "sky_freq", f2);
    //             if(std::fabs(f2-f1) > eps){other = ch;} //partner is not correct, treat as stand alone channel
    //         }
    // 
    //         for(std::size_t ap =0; ap<ap_ax->GetSize(); ap++)
    //         {
    //             double w = 1.0;
    //             if(this->fWeights != nullptr)
    //             {   
    //                 double w1 = this->fWeights->at(pp, ch, ap, 0);
    //                 double w2 = this->fWeights->at(pp, other, ap, 0);
    //                 if(ch != other){w = w1/(w1+w2);}
    //                 else{w1 = 1.0;}
    //             } //TODO check dimensions match
    // 
    //             for(std::size_t fr=0; fr<freq_ax->GetSize(); fr++)
    //             {
    //                 if(net_sideband == "L")
    //                 {
    //                     auto val = std::conj(in->at(pp,ch,ap,fr) );
    //                     if(ch != other)
    //                     {
    //                         //double sideband channel...split this point across both channels (they'll be summed together later)
    //                         workspace->at(pp,ch,ap, lsb_shift - fr) += w*val;
    //                         workspace->at(pp,ch,ap,fr) += w*( in->at(pp, other, ap, fr) );
    //                     }
    //                     else 
    //                     {
    //                         workspace->at(pp,ch,ap, lsb_shift - fr) += w*val; //stand alone channel
    //                     }
    //                 }
    //                 if(net_sideband == "U")
    //                 {
    //                     auto val = in->at(pp,ch,ap,fr);
    //                     if(ch != other)
    //                     {
    //                         //double sideband channel...split this point across both channels (they'll be summed together later)
    //                         workspace->at(pp,ch,ap,fr) += w*val;
    //                         workspace->at(pp,ch,ap, lsb_shift - fr) += w*( std::conj(in->at(pp,other,ap,fr) ) );
    //                     }
    //                     else
    //                     {
    //                         workspace->at(pp,ch,ap,fr) += w*val; //stand alone channel
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }
}



// bool MHO_MixedSidebandNormFX::ApplyWeights(visibility_type* out, weight_type* w, bool invert)
// {
//     std::size_t vis_dim[ visibility_type::rank::value];
//     std::size_t wt_dim[ visibility_type::rank::value];
// 
//     out->GetDimensions(vis_dim);
//     w->GetDimensions(wt_dim);
// 
//     //make sure the first 3 dimensions (polprod, channels, time) are the same!
//     bool same_size = true;
//     for(std::size_t i=0; i<3; i++)
//     {
//         if(vis_dim[i] != wt_dim[i]){same_size = false;}
//     }
// 
//     if(!same_size)
//     {
//         msg_error("calibration", "could not apply weights in MHO_MixedSidebandNormFX, dimension mismatch " << eom );
//         return false;
//     }
// 
//     for(std::size_t pp = 0; pp < vis_dim[POLPROD_AXIS]; pp++)
//     {
//         for(std::size_t ch = 0; ch < vis_dim[CHANNEL_AXIS]; ch++)
//         {
//             for(std::size_t ap = 0; ap < vis_dim[TIME_AXIS]; ap++)
//             {
//                 double factor = (*w)(pp,ch,ap,0); //apply the data weights
//                 if(invert)
//                 {
//                     if(factor > 0.0){factor = 1.0/factor;}
//                     else{factor = 0.0;}
//                 }
//                 out->SubView(pp, ch, ap) *= factor;
//             }
//         }
//     }
// 
//     return true;
// }


} // namespace hops
