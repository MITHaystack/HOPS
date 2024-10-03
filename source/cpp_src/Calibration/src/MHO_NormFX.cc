#include "MHO_NormFX.hh"

#include <algorithm>

#define PADDING_FACTOR 4

namespace hops
{

MHO_NormFX::MHO_NormFX(): fInitialized(false){};

MHO_NormFX::~MHO_NormFX(){};

bool MHO_NormFX::InitializeOutOfPlace(const XArgType* in, XArgType* out)
{
    fInitialized = false;
    if(in != nullptr && out != nullptr)
    {
        bool status = true;
        //first we initialize the SBD array, by resizing if necessary
        //all this does is expand the frequency axis by 4, unless there
        //are double sideband channels (which have to be merged once we fill things in)
        fSBDGen.SetArgs(in, out);
        fSBDGen.Initialize(); //re-size the sbd (out) array if needed
        fSBDGen.Execute();    //this is a no-op

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
            msg_debug("calibration", "MHO_NormFX operating on LSB data, N LSB channels: " << n_lsb_chan << eom);
        }
        if(n_usb_chan != 0)
        {
            msg_debug("calibration", "MHO_NormFX operating on USB data, N USB channels: " << n_usb_chan << eom);
        }

        //mixed sideband data should be ok, but warn user since it is not well tested
        if(n_usb_chan != 0 && n_lsb_chan != 0)
        {
            msg_warn("calibration", "support for data with mixed LSB/USB is experimental" << eom);
        }

        std::vector< mho_json > dsb_labels = channel_axis->GetMatchingIntervalLabels("double_sideband");
        std::size_t n_dsb_chan = dsb_labels.size();
        if(n_dsb_chan != 0)
        {
            //tell the user we can't handle double-sideband channels
            msg_error("calibration",
                      "discovered: "
                          << n_dsb_chan
                          << " double-sideband channels, support for this data type is experimental"
                          << eom);
        }

        in->GetDimensions(fInDims);
        out->GetDimensions(fOutDims);
        // fInDims[FREQ_AXIS] -- in the original norm_fx, nlags is 2x this number

        //this operator fills the SBD table from the input visibilities
        //but is only used where there are no double-sideband channels
        fZeroPadder.SetArgs(in, out);
        fZeroPadder.DeselectAllAxes();
        //fZeroPadder.EnableNormFXMode(); //doesnt seem to make any difference
        fZeroPadder.SelectAxis(FREQ_AXIS); //only pad on the frequency (to lag) axis
        fZeroPadder.SetPaddingFactor(PADDING_FACTOR); //original padding factor was 8, but then data was subsampled by 2
        fZeroPadder.SetEndPadded();
        status = fZeroPadder.Initialize();
        if(!status)
        {
            msg_error("calibration", "could not initialize zero padder in MHO_NormFX" << eom);
            return false;
        }

        fNaNBroadcaster.SetArgs(out);
        status = fNaNBroadcaster.Initialize();
        if(!status)
        {
            msg_error("calibration", "could not initialize NaN mask broadcast in MHO_NormFX" << eom);
            return false;
        }

        fFFTEngine.SetArgs(out);
        fFFTEngine.DeselectAllAxes();
        fFFTEngine.SelectAxis(FREQ_AXIS); //only perform padded fft on frequency (to lag) axis
        fFFTEngine.SetForward();          //forward DFT
        status = fFFTEngine.Initialize();
        if(!status)
        {
            msg_error("calibration", "could not initialize FFT in MHO_NormFX" << eom);
            return false;
        }

        fCyclicRotator.SetOffset(FREQ_AXIS, fOutDims[FREQ_AXIS] / 2);
        fCyclicRotator.SetArgs(out);
        status = fCyclicRotator.Initialize();
        if(!status)
        {
            msg_error("calibration", "could not initialize cyclic rotation in MHO_NormFX" << eom);
            return false;
        }

        fInitialized = true;
    }

    return fInitialized;
}

bool MHO_NormFX::ExecuteOutOfPlace(const XArgType* in, XArgType* out)
{

    if(fInitialized)
    {
        bool status;

        //set the freq axis units (FFT handles the tranform to SBD delay units)
        std::vector< mho_json > dsb_labels = std::get< CHANNEL_AXIS >(*(in)).GetMatchingIntervalLabels("double_sideband");
        std::size_t n_dsb_chan = dsb_labels.size();

        FillSBDTableNoDSB(in, out); //fill the table as normal, ignore dsb channels
        // else{FillSBDTableWithDSB(in, out);} //deal with the complication caused by dsb channels

        //filter out any NaNs
        status = fNaNBroadcaster.Execute();
        if(!status)
        {
            msg_error("calibration", "could not execute NaN masker MHO_NormFX" << eom);
            return false;
        }

        status = fFFTEngine.Execute();
        if(!status)
        {
            msg_error("calibration", "could not execute FFT in MHO_NormFX" << eom);
            return false;
        }

        status = fCyclicRotator.Execute();
        if(!status)
        {
            msg_error("calibration", "could not execute cyclic-rotation MHO_NormFX" << eom);
            return false;
        }

        //for lower sideband we complex conjugate the data (could do this before FFT)
        auto chan_ax = &(std::get< CHANNEL_AXIS >(*out));
        for(std::size_t ch = 0; ch < chan_ax->GetSize(); ch++)
        {
            std::string net_sideband;
            bool key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", net_sideband);
            if(!key_present)
            {
                msg_error("calibration", "norm_fx missing net_sideband label for channel " << ch << eom);
            }
            if(net_sideband == "L")
            {
                //just the slice that matches this channel
                auto slice = out->SliceView(":", ch, ":", ":");
                for(auto it = slice.begin(); it != slice.end(); it++)
                {
                    *it = std::conj(*it);
                }
            }
        }

        //normalize the array (due to FFT)
        double norm = 1.0 / (double)fInDims[FREQ_AXIS];
        *(out) *= norm;

        return true;
    }

    return false;
};

//not used
bool MHO_NormFX::InitializeInPlace(XArgType* in)
{
    XArgType* tmp = new XArgType();
    bool status = InitializeOutOfPlace(in, tmp);
    in->Copy(*tmp);
    delete tmp;
    return status;
}

bool MHO_NormFX::ExecuteInPlace(XArgType* in)
{
    XArgType* tmp = new XArgType();
    bool status = ExecuteOutOfPlace(in, tmp);
    in->Copy(*tmp);
    delete tmp;
    return status;
}

bool MHO_NormFX::FillSBDTableNoDSB(const XArgType* in, XArgType* out)
{
    //copy in the visibility data in a zero-padded fashion (zeros go on the end)
    //this is only valid when there are no double-sideband channels that need to
    //be merged
    out->ZeroArray();
    bool status = fZeroPadder.Execute(); //use pre-configured zero-padder operator
    if(!status)
    {
        msg_error("calibration", "could not execute zero padder in MHO_NormFX" << eom);
        return false;
    }

    // //for lower sideband we complex conjugate and flip the data and conjugate (equivalent to a conjugation after the FFT)
    // auto pp_ax = &(std::get<POLPROD_AXIS>(*out));
    // auto chan_ax = &(std::get<CHANNEL_AXIS>(*out));
    // auto time_ax = &(std::get<TIME_AXIS>(*out));
    // for(std::size_t pp=0; pp < pp_ax->GetSize(); pp++)
    // {
    //     for(std::size_t ch=0; ch < chan_ax->GetSize(); ch++)
    //     {
    //         std::string net_sideband;
    //         bool key_present = chan_ax->RetrieveIndexLabelKeyValue(ch, "net_sideband", net_sideband);
    //         if(!key_present){msg_error("calibration", "norm_fx missing net_sideband label for channel "<< ch << eom); }
    //         if(net_sideband == "L")
    //         {
    //             for(std::size_t ap=0; ap< time_ax->GetSize(); ap++)
    //             {
    //                 //just grab the slice that matches this chunk
    //                 auto slice = out->SubView(pp, ch, ap);
    //                 //flip the order of the elements,
    //                 std::reverse(slice.begin(), slice.end() );
    //                 //then conjugate it
    //                 for(auto it = slice.begin(); it != slice.end(); it++){*it = std::conj(*it);}
    //             }
    //         }
    //     }
    // }

    return status;
}

bool MHO_NormFX::FillSBDTableWithDSB(const XArgType* in, XArgType* out)
{
    //now we copy the data from in -> out keeping in mind that:
    //USB and LSB channels get copied in at the front (with the zero-padding at the end)
    //DSB channels must be merged, with USB at the front and DSB at the back
    out->ZeroArray();

    //copy the pol-product and time axes (as they are the same size as input table)
    auto pp_ax = std::get< POLPROD_AXIS >(*in);
    auto ap_ax = std::get< TIME_AXIS >(*in);
    //
    // //now deal with the channel axis...these have a different sizes, because we
    // //need to merge any DSB channels
    // auto in_chan_ax = &(std::get<CHANNEL_AXIS>(*in));
    // auto out_chan_ax = &(std::get<CHANNEL_AXIS>(*out));
    // std::size_t out_chan_count = 0;
    // for(std::size_t i=0; i<in_chan_ax->GetSize(); i++)
    // {
    //     mho_json index_label_obj = in_chan_ax->GetLabelObject(i);
    //     //check if this channel is a member of a double side-band pair
    //     if(index_label_obj.contains("dsb_partner"))
    //     {
    //         //this is a double sideband channel, get the partner's index
    //         //this should be the USB partner
    //         int pindex = index_label_obj["dsb_partner"].get<int>();
    //         if(pindex != i+1)
    //         {
    //             //something has gone seriously wrong, this data can't be processed
    //             msg_fatal("calibration", "non-adjacent double-sideband pair, cannot merge LSB/USB channels of this form " << eom);
    //             std::exit(1);
    //         }
    //         //copy the sky frequency value
    //         out_chan_ax->at(out_chan_count) = in_chan_ax->at(i);
    //         //copy the label, but modify the net_sideband
    //         index_label_obj["net_sideband"] = "D";
    //         out_chan_ax->SetLabelObject(index_label_obj, out_chan_count);
    //         i = pindex; //jump to adjacent USB channel
    //         out_chan_count++;
    //     }
    //     else
    //     {
    //         //just a regular channel
    //         out_chan_ax->at(out_chan_count) = in_chan_ax->at(i); //copy sky frequency
    //         out_chan_ax->SetLabelObject(index_label_obj, out_chan_count); //copy labels
    //         out_chan_count++;
    //     }
    // }
    //

    return false;
}

} // namespace hops
