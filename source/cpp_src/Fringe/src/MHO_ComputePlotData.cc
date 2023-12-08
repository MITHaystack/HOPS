#include "MHO_ComputePlotData.hh"
#include "MHO_BasicFringeInfo.hh"
#include "MHO_UniformGridPointsCalculator.hh"
#include "MHO_EndZeroPadder.hh"

namespace hops
{

MHO_ComputePlotData::MHO_ComputePlotData()
{
    fMBDAnchor = "model";
    fParamStore = nullptr;
    fContainerStore = nullptr;
    fVisibilities = nullptr;
    fWeights = nullptr;
    fSBDArray = nullptr;
};


void
MHO_ComputePlotData::Initialize()
{
    fTotalSummedWeights = fParamStore->GetAs<double>("/fringe/total_summed_weights");
    fRefFreq = fParamStore->GetAs<double>("/control/config/ref_freq");
    fMBDelay = fParamStore->GetAs<double>("/fringe/mbdelay");
    fDelayRate = fParamStore->GetAs<double>("/fringe/drate");
    fFringeRate = fParamStore->GetAs<double>("/fringe/frate");
    fSBDelay = fParamStore->GetAs<double>("/fringe/sbdelay");
    fSBDMaxBin = fParamStore->GetAs<double>("/fringe/max_sbd_bin");
    fAmp = fParamStore->GetAs<double>("/fringe/famp");

    fVisibilities = fContainerStore->GetObject<visibility_type>(std::string("vis"));
    fWeights = fContainerStore->GetObject<weight_type>(std::string("weight"));
    fSBDArray = fContainerStore->GetObject<visibility_type>(std::string("sbd"));
    if(fVisibilities == nullptr)
    {
        msg_fatal("fringe", "could not find visibility, object with name 'vis'." << eom);
        std::exit(1);
    }

    if(fWeights == nullptr)
    {
        msg_fatal("fringe", "could not find visibility, object with name 'weight'." << eom);
        std::exit(1);
    }

    if(fSBDArray == nullptr)
    {
        msg_fatal("fringe", "could not find visibility, object with name 'sbd'." << eom);
        std::exit(1);
    }

}

xpower_amp_type
MHO_ComputePlotData::calc_mbd()
{

    //calculate the frequency grid for the channel -> MBD FFT
    MHO_UniformGridPointsCalculator fGridCalc;
    fGridCalc.SetDefaultGridPoints(8192);
    fGridCalc.SetPoints( std::get<CHANNEL_AXIS>(*fSBDArray).GetData(), std::get<CHANNEL_AXIS>(*fSBDArray).GetSize() );
    fGridCalc.Calculate();

    std::size_t fGridStart = fGridCalc.GetGridStart();
    double fGridSpace = fGridCalc.GetGridSpacing();
    std::size_t fNGridPoints = fGridCalc.GetNGridPoints();
    auto fMBDBinMap = fGridCalc.GetGridIndexMap();
    std::size_t fNSBD = fSBDArray->GetDimension(FREQ_AXIS);
    std::size_t fNDR = fSBDArray->GetDimension(TIME_AXIS);

    //resize workspaces (TODO...make conditional on current size -- if already configured)
    fMBDWorkspace.Resize(fNGridPoints);
    fMBDWorkspace.ZeroArray();
    fMBDAmpWorkspace.Resize(fNGridPoints);
    fMBDAmpWorkspace.ZeroArray();

    auto mbd_ax = &(std::get<0>(fMBDWorkspace) );
    for(std::size_t i=0; i<fNGridPoints;i++)
    {
        mbd_ax->at(i) = fGridStart + i*fGridSpace;
    }

    //set up FFT and rotator engines
    fFFTEngine.SetArgs(&fMBDWorkspace);
    fFFTEngine.DeselectAllAxes();
    fFFTEngine.SelectAxis(0);
    fFFTEngine.SetForward();
    bool ok = fFFTEngine.Initialize();
    check_step_fatal(ok, "calibration", "MBD search fft engine initialization." << eom );

    fCyclicRotator.SetOffset(0, fNGridPoints/2);
    fCyclicRotator.SetArgs(&fMBDWorkspace);
    ok = fCyclicRotator.Initialize();
    check_step_fatal(ok, "calibration", "MBD search cyclic rotation initialization." << eom );

    //now we are going to loop over all of the channels/AP
    //and perform the weighted sum of the data at the max-SBD bin
    //with the fitted delay-rate rotation (but mbd=0) applied

    //grab the total summed weights
    double total_summed_weights = 1.0;
    fWeights->Retrieve("total_summed_weights", total_summed_weights);

    std::size_t POLPROD = 0;
    std::size_t nchan = fSBDArray->GetDimension(CHANNEL_AXIS);
    std::size_t nap = fSBDArray->GetDimension(TIME_AXIS);

    auto chan_ax = &( std::get<CHANNEL_AXIS>(*fSBDArray) );
    auto ap_ax = &(std::get<TIME_AXIS>(*fSBDArray));
    auto sbd_ax = &( std::get<FREQ_AXIS>(*fSBDArray) );
    double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);
    double frt_offset = fParamStore->GetAs<double>("/config/frt_offset");

    std::complex<double> sum = 0;
    for(std::size_t ch=0; ch < nchan; ch++)
    {
        double freq = (*chan_ax)(ch);//sky freq of this channel

        MHO_IntervalLabel ilabel(ch,ch);
        std::string net_sideband = "?";
        std::string sidebandlabelkey = "net_sideband";
        auto other_labels = chan_ax->GetIntervalsWhichIntersect(ilabel);
        for(auto olit = other_labels.begin(); olit != other_labels.end(); olit++)
        {
            if( olit->HasKey(sidebandlabelkey) )
            {
                olit->Retrieve(sidebandlabelkey, net_sideband);
                break;
            }
        }
        fRot.SetSideband(0); //DSB
        if(net_sideband == "U"){fRot.SetSideband(1);}
        if(net_sideband == "L"){fRot.SetSideband(-1);}

        sum = 0;
        for(std::size_t ap=0; ap < nap; ap++)
        {
            double tdelta = (ap_ax->at(ap) + ap_delta/2.0) - frt_offset; //need time difference from the f.r.t?
            std::complex<double> vis = (*fSBDArray)(POLPROD, ch, ap, fSBDMaxBin); //pick out data at SBD max bin
            std::complex<double> vr = fRot.vrot(tdelta, freq, fRefFreq, fDelayRate, 0.0); //apply at MBD=0.0
            std::complex<double> z = vis*vr;
            //apply weight and sum
            double w = (*fWeights)(POLPROD, ch, ap, 0);
            sum += w*z;
        }
        //slot the summed data in at the appropriate location in the new grid
        std::size_t mbd_bin = fMBDBinMap[ch];
        fMBDWorkspace(mbd_bin) = sum;
    }

    //now run an FFT along the MBD axis and cyclic rotate
    ok = fFFTEngine.Execute();
    check_step_fatal(ok, "calibration", "MBD search fft engine execution." << eom );
    ok = fCyclicRotator.Execute();
    check_step_fatal(ok, "calibration", "MBD search cyclic rotation execution." << eom );

    for(std::size_t i=0; i<fNGridPoints; i++)
    {
        fMBDAmpWorkspace[i] = std::abs(fMBDWorkspace[i])/total_summed_weights;
        std::get<0>(fMBDAmpWorkspace).at(i) = std::get<0>(fMBDWorkspace).at(i);
    }

    return fMBDAmpWorkspace;

}


xpower_amp_type
MHO_ComputePlotData::calc_sbd()
{

    //grab the total summed weights
    double total_summed_weights = 1.0;
    fWeights->Retrieve("total_summed_weights", total_summed_weights);

    xpower_type sbd_xpower_in;
    xpower_type sbd_xpower_out;
    xpower_amp_type sbd_amp;

    std::size_t POLPROD = 0;
    std::size_t nchan = fSBDArray->GetDimension(CHANNEL_AXIS);
    std::size_t nap = fSBDArray->GetDimension(TIME_AXIS);
    std::size_t nbins = fSBDArray->GetDimension(FREQ_AXIS);

    auto chan_ax = &( std::get<CHANNEL_AXIS>(*fSBDArray) );
    auto ap_ax = &(std::get<TIME_AXIS>(*fSBDArray));
    auto sbd_ax = &( std::get<FREQ_AXIS>(*fSBDArray) );
    double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);
    double frt_offset = fParamStore->GetAs<double>("/config/frt_offset");

    sbd_amp.Resize(nbins);
    sbd_xpower_in.Resize(nbins);
    sbd_xpower_out.Resize(4*nbins); //interpolation

    sbd_xpower_in.ZeroArray();
    sbd_xpower_out.ZeroArray();

    //loop over sbd bins (4*nlags) and sum over channel/ap
    for(std::size_t i=0; i<nbins; i++)
    {
        std::complex<double> sum = 0;
        for(std::size_t ch=0; ch < nchan; ch++)
        {
            double freq = (*chan_ax)(ch);//sky freq of this channel

            MHO_IntervalLabel ilabel(ch,ch);
            std::string net_sideband = "?";
            std::string sidebandlabelkey = "net_sideband";
            auto other_labels = chan_ax->GetIntervalsWhichIntersect(ilabel);
            for(auto olit = other_labels.begin(); olit != other_labels.end(); olit++)
            {
                if( olit->HasKey(sidebandlabelkey) )
                {
                    olit->Retrieve(sidebandlabelkey, net_sideband);
                    break;
                }
            }
            fRot.SetSideband(0); //DSB
            if(net_sideband == "U"){fRot.SetSideband(1);}
            if(net_sideband == "L"){fRot.SetSideband(-1);}

            sum = 0;
            for(std::size_t ap=0; ap < nap; ap++)
            {
                double tdelta = (ap_ax->at(ap) + ap_delta/2.0) - frt_offset; //need time difference from the f.r.t?
                std::complex<double> vis = (*fSBDArray)(POLPROD, ch, ap, i);
                std::complex<double> vr = fRot.vrot(tdelta, freq, fRefFreq, fDelayRate, fMBDelay);
                std::complex<double> z = vis*vr;
                //apply weight and sum
                double w = (*fWeights)(POLPROD, ch, ap, 0);
                sum += w*z;
            }
            sbd_xpower_in(i) += sum;
        }

        sbd_amp(i) = std::abs( sbd_xpower_in(i) )/total_summed_weights;
        std::get<0>(sbd_amp)(i) = (*sbd_ax)(i);
    }

    return sbd_amp;
}




phasor_type
MHO_ComputePlotData::calc_segs()
{
    //grab the SBD max bin
    std::size_t max_sbd_bin = (std::size_t)fParamStore->GetAs<int>("/fringe/max_sbd_bin");

    std::size_t POLPROD = 0;
    std::size_t nchan = fSBDArray->GetDimension(CHANNEL_AXIS);
    std::size_t nap = fSBDArray->GetDimension(TIME_AXIS);
    std::size_t nbins = fSBDArray->GetDimension(FREQ_AXIS);

    auto chan_ax = &( std::get<CHANNEL_AXIS>(*fSBDArray) );
    auto ap_ax = &(std::get<TIME_AXIS>(*fSBDArray));
    double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    double frt_offset = fParamStore->GetAs<double>("/config/frt_offset");

    phasor_type phasor_segs;
    phasor_segs.Resize(nchan+1, nap);
    phasor_segs.ZeroArray();

    //grab the fourfit channel name
    std::string chan_label_key = "channel_label";
    std::vector< std::string > channel_labels;
    (&std::get<CHANNEL_AXIS>(*fSBDArray))->CollectAxisElementLabelValues(chan_label_key, channel_labels);

    for(std::size_t ap=0; ap < nap; ap++)
    {
        std::complex<double> sum = 0; //sum over all channels
        double sumwt = 0.0;
        double tdelta = (ap_ax->at(ap) + ap_delta/2.0) - frt_offset; //need time difference from the f.r.t?
        for(std::size_t ch=0; ch < nchan; ch++)
        {
            double freq = (*chan_ax)(ch);//sky freq of this channel

            MHO_IntervalLabel ilabel(ch,ch);
            std::string net_sideband = "?";
            std::string sidebandlabelkey = "net_sideband";
            auto other_labels = chan_ax->GetIntervalsWhichIntersect(ilabel);
            for(auto olit = other_labels.begin(); olit != other_labels.end(); olit++)
            {
                if( olit->HasKey(sidebandlabelkey) )
                {
                    olit->Retrieve(sidebandlabelkey, net_sideband);
                    break;
                }
            }
            fRot.SetSideband(0); //DSB
            if(net_sideband == "U"){fRot.SetSideband(1);}
            if(net_sideband == "L"){fRot.SetSideband(-1);}

            //make sure this plot gets the channel label:
            MHO_IntervalLabel ch_name(ch,ch);
            ch_name.Insert(chan_label_key, channel_labels[ch]);
            (&std::get<0>(phasor_segs))->InsertLabel(ch_name);

            (&std::get<0>(phasor_segs))->at(ch) = freq;
            std::complex<double> vis = (*fSBDArray)(POLPROD, ch, ap, max_sbd_bin);
            std::complex<double> vr = fRot.vrot(tdelta, freq, fRefFreq, fDelayRate, fMBDelay);
            std::complex<double> z = vis*vr;
            phasor_segs(ch, ap) = z;
            (&std::get<0>(phasor_segs))->at(ch) = freq; //set the channel frequency label
            //apply weight and sum
            double w = (*fWeights)(POLPROD, ch, ap, 0);
            std::complex<double> wght_phsr = w*z;
            sum += wght_phsr;
            sumwt += w;
        }
        (&std::get<1>(phasor_segs))->at(ap) = ap_ax->at(ap); //set the ap label
        //add the sum over all channels
        phasor_segs(nchan,ap) = sum/sumwt;
        MHO_IntervalLabel ch_name(nchan,nchan);
        ch_name.Insert(chan_label_key, std::string("All"));
        (&std::get<0>(phasor_segs))->InsertLabel(ch_name);
    }
    return phasor_segs;

}


xpower_amp_type
MHO_ComputePlotData::calc_dr()
{
    //grab the total summed weights
    double total_summed_weights = 1.0;
    fWeights->Retrieve("total_summed_weights", total_summed_weights);

    std::size_t POLPROD = 0;
    std::size_t nchan = fSBDArray->GetDimension(CHANNEL_AXIS);
    std::size_t nap = fSBDArray->GetDimension(TIME_AXIS);
    //borrow this stupid routine from search_windows.c /////////////////////

    #pragma message("Fix the DRSP size calculation to remove upper limit of 8192.")
    std::size_t drsp_size = 8192;
    while ( (drsp_size / 4) > nap ) {drsp_size /= 2;};
    if(drsp_size < 256){drsp_size = 256;}

    ////////////////////////////////////////////////////////////////////////

    //resize workspaces (TODO...make conditional on current size -- if already configured)
    fDRWorkspace.Resize(drsp_size);
    fDRWorkspace.ZeroArray();
    fDRAmpWorkspace.Resize(drsp_size);
    fDRAmpWorkspace.ZeroArray();

    //set up FFT and rotator engines
    fFFTEngine.SetArgs(&fDRWorkspace);
    fFFTEngine.DeselectAllAxes();
    fFFTEngine.SelectAxis(0);
    fFFTEngine.SetForward();
    bool ok = fFFTEngine.Initialize();
    check_step_fatal(ok, "calibration", "MBD search fft engine initialization." << eom );

    fCyclicRotator.SetOffset(0, drsp_size/2);
    fCyclicRotator.SetArgs(&fDRWorkspace);
    ok = fCyclicRotator.Initialize();
    check_step_fatal(ok, "calibration", "MBD search cyclic rotation initialization." << eom );

    //now we are going to loop over all of the channels/AP
    //and perform the weighted sum of the data at the max-SBD bin
    //with the fitted delay-rate rotation (but mbd=0) applied
    auto chan_ax = &( std::get<CHANNEL_AXIS>(*fSBDArray) );
    auto ap_ax = &(std::get<TIME_AXIS>(*fSBDArray));
    auto sbd_ax = &( std::get<FREQ_AXIS>(*fSBDArray) );
    double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);
    double frt_offset = fParamStore->GetAs<double>("/config/frt_offset");
    auto dr_ax = &(std::get<0>(fDRWorkspace) );

    for(std::size_t i=0; i<drsp_size;i++)
    {
        dr_ax->at(i) = i*ap_delta;
    }


    for(std::size_t ch=0; ch < nchan; ch++)
    {
        double freq = (*chan_ax)(ch);//sky freq of this channel

        MHO_IntervalLabel ilabel(ch,ch);
        std::string net_sideband = "?";
        std::string sidebandlabelkey = "net_sideband";
        auto other_labels = chan_ax->GetIntervalsWhichIntersect(ilabel);
        for(auto olit = other_labels.begin(); olit != other_labels.end(); olit++)
        {
            if( olit->HasKey(sidebandlabelkey) )
            {
                olit->Retrieve(sidebandlabelkey, net_sideband);
                break;
            }
        }
        fRot.SetSideband(0); //DSB
        if(net_sideband == "U"){fRot.SetSideband(1);}
        if(net_sideband == "L"){fRot.SetSideband(-1);}


        for(std::size_t ap=0; ap < nap; ap++)
        {
            double tdelta = (ap_ax->at(ap) + ap_delta/2.0) - frt_offset; //need time difference from the f.r.t?
            std::complex<double> vis = (*fSBDArray)(POLPROD, ch, ap, fSBDMaxBin); //pick out data at SBD max bin
            std::complex<double> vr = fRot.vrot(tdelta, freq, fRefFreq, fDelayRate, fMBDelay); //why rotate at the max delay rate??
            std::complex<double> z = vis*vr;
            //apply weight and sum
            double w = (*fWeights)(POLPROD, ch, ap, 0);
            fDRWorkspace(ap) += w*z;
        }

    }

    //now run an FFT along the MBD axis and cyclic rotate
    ok = fFFTEngine.Execute();
    check_step_fatal(ok, "calibration", "MBD search fft engine execution." << eom );
    ok = fCyclicRotator.Execute();
    check_step_fatal(ok, "calibration", "MBD search cyclic rotation execution." << eom );

    for(std::size_t i=0; i<drsp_size; i++)
    {
        fDRAmpWorkspace[i] = std::abs(fDRWorkspace[i])/total_summed_weights;
        #pragma message("TODO FIXME, factor 1/1000 is due to need to plot axis in ns/s")
        std::get<0>(fDRAmpWorkspace).at(i) = (std::get<0>(fDRWorkspace).at(i) )/(fRefFreq/1000.0);
    }

    return fDRAmpWorkspace;
}



double
MHO_ComputePlotData::calc_phase()
{
    //grab the total summed weights
    double total_summed_weights = 1.0;
    fWeights->Retrieve("total_summed_weights", total_summed_weights);

    std::size_t POLPROD = 0;
    std::size_t nchan = fSBDArray->GetDimension(CHANNEL_AXIS);
    std::size_t nap = fSBDArray->GetDimension(TIME_AXIS);

    //now we are going to loop over all of the channels/AP
    //and perform the weighted sum of the data at the max-SBD bin
    //with the fitted delay-rate rotation (but mbd=0) applied
    auto chan_ax = &( std::get<CHANNEL_AXIS>(*fSBDArray) );
    auto ap_ax = &(std::get<TIME_AXIS>(*fSBDArray));
    auto sbd_ax = &( std::get<FREQ_AXIS>(*fSBDArray) );
    double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);

    fRot.SetSBDSeparation(sbd_delta);
    fRot.SetSBDMaxBin(fSBDMaxBin);
    fRot.SetNSBDBins(sbd_ax->GetSize()/2);  //this is effective nlags
    fRot.SetSBDMax( fSBDelay );
    double frt_offset = fParamStore->GetAs<double>("/config/frt_offset");

    std::complex<double> sum_all = 0.0;
    for(std::size_t ch=0; ch < nchan; ch++)
    {
        double freq = (*chan_ax)(ch);//sky freq of this channel
        MHO_IntervalLabel ilabel(ch,ch);
        std::string net_sideband = "?";
        std::string sidebandlabelkey = "net_sideband";
        auto other_labels = chan_ax->GetIntervalsWhichIntersect(ilabel);
        for(auto olit = other_labels.begin(); olit != other_labels.end(); olit++)
        {
            if( olit->HasKey(sidebandlabelkey) )
            {
                olit->Retrieve(sidebandlabelkey, net_sideband);
                break;
            }
        }

        fRot.SetSideband(0); //DSB
        if(net_sideband == "U")
        {
            fRot.SetSideband(1);
        }

        if(net_sideband == "L")
        {
            fRot.SetSideband(-1);
        }

        for(std::size_t ap=0; ap < nap; ap++)
        {
            double tdelta = (ap_ax->at(ap) + ap_delta/2.0) - frt_offset; //need time difference from the f.r.t?
            std::complex<double> vis = (*fSBDArray)(POLPROD, ch, ap, fSBDMaxBin); //pick out data at SBD max bin
            std::complex<double> vr = fRot.vrot(tdelta, freq, fRefFreq, fDelayRate, fMBDelay);
            std::complex<double> z = vis*vr;
            //apply weight and sum
            double w = (*fWeights)(POLPROD, ch, ap, 0);
            std::complex<double> wght_phsr = z*w;
            if(net_sideband == "U")
            {
                sum_all += -1.0*wght_phsr;
            }
            else
            {
                sum_all += wght_phsr;
            }
        }
    }

    double coh_avg_phase = std::arg(sum_all);

    return coh_avg_phase; //not quite the value which is displayed in the fringe plot (see fill type 208)
}






xpower_type
MHO_ComputePlotData::calc_xpower_spec()
{
    //kludge version - this code is adapted from a combination of what is found in
    //make_plotdata.c and generate_graphs.c. It is extremely convoluted
    //and we ought to find a cleaner/clearer way to do the same thing

    //grab the total summed weights
    double total_summed_weights = 1.0;
    fWeights->Retrieve("total_summed_weights", total_summed_weights);

    int nlags = fSBDArray->GetDimension(FREQ_AXIS)/2;
    int nl = nlags;

    xpower_type X;
    xpower_type Y;
    xpower_type cp_spectrum;
    xpower_type cp_spectrum_out;
    X.Resize(2*nl); X.ZeroArray();
    Y.Resize(4*nl); Y.ZeroArray();
    cp_spectrum.Resize(2*nl); cp_spectrum.ZeroArray();

    std::size_t POLPROD = 0;
    std::size_t nchan = fSBDArray->GetDimension(CHANNEL_AXIS);
    std::size_t nap = fSBDArray->GetDimension(TIME_AXIS);
    std::size_t nbins = fSBDArray->GetDimension(FREQ_AXIS);

    //single channel xpower spectrum, need this info for the 'sbdbox' parameters
    //printed out on the fourfit plot
    std::vector< xpower_type > sbxsp;
    std::vector< int > maxlag;
    std::vector< double > maxlag_amp;
    std::vector< double > sbdbox;
    sbxsp.resize(nchan);
    maxlag.resize(nchan, 0);
    maxlag_amp.resize(nchan, 0.0);
    sbdbox.resize(nchan, 0.0);
    for(std::size_t ch=0; ch < nchan; ch++){sbxsp[ch].Resize(2*nl);}

    //TODO FIXME...this is temporary, as it doesn't account for min_weight cuts
    std::vector<int> nusb_ap; nusb_ap.resize(nchan, 0);
    std::vector<int> nlsb_ap; nlsb_ap.resize(nchan, 0);

    auto chan_ax = &( std::get<CHANNEL_AXIS>(*fSBDArray) );
    auto ap_ax = &(std::get<TIME_AXIS>(*fSBDArray));
    auto sbd_ax = &( std::get<FREQ_AXIS>(*fSBDArray) );
    double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);
    double frt_offset = fParamStore->GetAs<double>("/config/frt_offset");

    std::complex<double> sum;
    std::complex<double> Z, vr;
    double frac;
    double bw;
    std::string net_sideband = "?";
    //count the sidebands encountered
    int nusb = 0;
    int nlsb = 0;

    for(int lag = 0; lag < 2*nl; lag++)
    {
        for(int ch = 0; ch < nchan; ch++)
        {
            double freq = (*chan_ax)(ch);//sky freq of this channel
            MHO_IntervalLabel ilabel(ch,ch);

            std::string sidebandlabelkey = "net_sideband";
            std::string bandwidthlabelkey = "bandwidth";
            auto other_labels = chan_ax->GetIntervalsWhichIntersect(ilabel);
            for(auto olit = other_labels.begin(); olit != other_labels.end(); olit++)
            {
                if( olit->HasKey(sidebandlabelkey) )
                {
                    olit->Retrieve(sidebandlabelkey, net_sideband);
                    break;
                }
            }

            for(auto olit = other_labels.begin(); olit != other_labels.end(); olit++)
            {
                if( olit->HasKey(bandwidthlabelkey) )
                {
                    olit->Retrieve(bandwidthlabelkey, bw);
                    break;
                }
            }

            fRot.SetSideband(0); //DSB
            if(net_sideband == "U")
            {
                nusb += 1;
                fRot.SetSideband(1);
                nusb_ap[ch] = nap;
            }
            else if(net_sideband == "L")
            {
                nlsb +=1;
                fRot.SetSideband(-1);
                nlsb_ap[ch] = nap;
            }

            sum = 0.0;
            for (int ap = 0; ap < nap; ap++)
            {
                double tdelta = (ap_ax->at(ap) + ap_delta/2.0) - frt_offset; //need time difference from the f.r.t?
                std::complex<double> vis = (*fSBDArray)(POLPROD, ch, ap, lag);
                std::complex<double> vr = fRot.vrot(tdelta, freq, fRefFreq, fDelayRate, fMBDelay);
                std::complex<double> Z = vis*vr;
                //apply weight and sum
                double w = (*fWeights)(POLPROD, ch, ap, 0);
                sum += w*Z;
            }
            sbxsp[ch].at(lag) = sum;
            X[lag] = X[lag] + sum;

            if( std::abs(sum) > maxlag_amp[ch])
            {
                maxlag_amp[ch] = std::abs(sum);
                maxlag[ch] = lag;
            }

        }
        //need to understand this
        int j = lag - nl;
        if(j < 0){j += 4 * nl;}
        if(lag == 0){j = 2 * nl;} // pure real lsb/dc channel goes in middle??
        Y[j] = X[lag];
        std::get<0>(Y)(j) = j;
    }

    //set up FFT and run
    fFFTEngine.SetArgs(&Y, &Y);
    fFFTEngine.DeselectAllAxes();
    fFFTEngine.SelectAxis(0);
    fFFTEngine.SetForward();
    bool ok = fFFTEngine.Initialize();
    ok = fFFTEngine.Execute();

    std::complex<double> cmplx_unit_I(0.0, 1.0);
    int s =  Y.GetDimension(0)/2;
    cp_spectrum.Resize(s);

    for(int i=0; i<2*nl; i++)
    {
        int j = nl - i;
        double sbfactor = 0.0;
        if(j <= 0){ if(net_sideband == "U"){sbfactor = sqrt(0.5)/(M_PI*total_summed_weights );}}
        else{ if(net_sideband == "L"){sbfactor = sqrt(0.5)/( M_PI*total_summed_weights );} }
        if(j < 0){ j += 4*nl; }

        cp_spectrum[i] = Y[j];
        Z = std::exp(-1.0*cmplx_unit_I * (fSBDelay * (i-nl) * M_PI / (sbd_delta *2.0*nl)));
        cp_spectrum[i] = Z * sbfactor * cp_spectrum[i];
    }

    //now have to tweak the section exported to the plot data
    //depending on how many USB/LSB/DSB channels we have encountered
    double xstart;
    double xend;
    int ncp, izero;
    if (nusb > 0 && nlsb > 0)           /* DSB */
    {
        xstart = -bw;
        xend = bw;
        ncp = 2 * nl;
        izero = 0;
    }
    else if (nlsb > 0)                  /* LSB only */
    {
        xstart = -bw;
        xend = 0.0;
        ncp = nl;
        izero = 0;
    }
    else                                /* USB only */
    {
        xstart = 0.0;
        xend = bw;
        ncp = nl;
        izero = nl;
    }

    cp_spectrum_out.Resize(ncp);

    for(int i=0; i<ncp; i++)
    {
        double spec_axis_value = xstart + (xend - xstart) * i / ncp;
        std::get<0>(cp_spectrum_out)(i) = spec_axis_value;
        cp_spectrum_out[i] = cp_spectrum[i+izero];
    }

    //now compute the sbdbox for each channel
    double yy[3], q[3];
    double peak, maxv;
    for(int ch = 0; ch < nchan; ch++)
    {
        for(int i=0; i<3; i++)
        {
            int idx = std::max( maxlag[ch] - 1 + i, 0);
            yy[i] = std::abs( sbxsp[ch].at(idx) );
        }
        MHO_MathUtilities::parabola(yy, -1.0, 1.0, &peak, &maxv, q);
        sbdbox[ch] = maxlag[ch] + peak + 1;
    }
    //'this is the All-channel sbdbox
    sbdbox.push_back( nl + 1 + fSBDelay/sbd_delta);

    //copy this into the accessible arrays
    fSBDBox = sbdbox;
    fNLSBAP = nlsb_ap;
    fNUSBAP = nusb_ap;

    return cp_spectrum_out;

}



void
MHO_ComputePlotData::DumpInfoToJSON(mho_json& plot_dict)
{
    auto sbd_amp = calc_sbd();
    auto mbd_amp = calc_mbd();
    auto dr_amp = calc_dr();
    //TODO FIXME -- move the residual phase calc elsewhere (but we need it for the moment to set the fRot parameters)
    double coh_avg_phase = calc_phase();
    auto sbd_xpower = calc_xpower_spec();
    auto phasors = calc_segs();

    double coh_avg_phase_deg = std::fmod(coh_avg_phase * (180.0/M_PI), 360.0);
    fParamStore->Set("/fringe/raw_resid_phase", coh_avg_phase_deg);

    //calculate AP period
    double ap_delta = std::get<TIME_AXIS>(*fVisibilities)(1) - std::get<TIME_AXIS>(*fVisibilities)(0);

    std::size_t npts = sbd_amp.GetSize();
    for(std::size_t i=0;i<npts;i++)
    {
        plot_dict["SBD_AMP"].push_back( sbd_amp(i) );
        plot_dict["SBD_AMP_XAXIS"].push_back( std::get<0>(sbd_amp)(i) );
    }

    npts = mbd_amp.GetSize();
    for(std::size_t i=0;i<npts;i++)
    {
        plot_dict["MBD_AMP"].push_back( mbd_amp(i) );
        plot_dict["MBD_AMP_XAXIS"].push_back( std::get<0>(mbd_amp)(i) );
    }

    npts = dr_amp.GetSize();
    for(std::size_t i=0;i<npts;i++)
    {
        plot_dict["DLYRATE"].push_back( dr_amp(i) );
        plot_dict["DLYRATE_XAXIS"].push_back( std::get<0>(dr_amp)(i) );
    }

    npts = sbd_xpower.GetSize();
    for(std::size_t i=0;i<npts;i++)
    {
        plot_dict["XPSPEC-ABS"].push_back( std::abs(sbd_xpower(i) ) );
        plot_dict["XPSPEC-ARG"].push_back( std::arg(sbd_xpower(i) )*(180.0/M_PI) );
        plot_dict["XPSPEC_XAXIS"].push_back( std::get<0>(sbd_xpower)(i) );
    }

    //grab the fourfit channel labels
    std::string chan_label_key = "channel_label";
    std::vector< std::string > channel_labels;
    (&std::get<0>(phasors))->CollectAxisElementLabelValues(chan_label_key, channel_labels);
    plot_dict["ChannelsPlotted"] = channel_labels;

    //grab the per-channel/AP phasors, and average down if necessary
    std::size_t nplot = phasors.GetDimension(0);
    std::size_t naps = phasors.GetDimension(1);
    std::size_t nseg = naps;
    std::vector< std::vector<double> > seg_amp; seg_amp.resize(nplot);
    std::vector< std::vector<double> > seg_arg; seg_arg.resize(nplot);

    std::vector<double> ch_amp;
    std::vector<double> ch_arg;

    //use fourfit default method to determine how many APs to average together (see calc_rms.c)
    int ap_per_seg = fParamStore->GetAs<int>("/cmdline/ap_per_seg");
    std::size_t apseg;
    if(nplot == 2){nplot = 1;}
    if(ap_per_seg == 0)
    {
        nseg = 200/nplot; //max of 200 points across plot
        if(nseg > naps){nseg = naps;}
        apseg = naps / nseg;
    }
    else
    {
        if(ap_per_seg > naps){apseg = naps;}
        else{apseg = ap_per_seg;}
    }

    // Number of segments, starting at AP 0
    // and using integer apseg per segment
    nseg = naps / apseg;
    //Remainder goes into last segment (???)
    if( (naps % apseg) != 0){ nseg += 1;}


    for(std::size_t i=0; i<nplot; i++)
    {
        std::complex<double> ph = 0;
        std::complex<double> phsum = 0;
        for(std::size_t j=0; j<naps; j++)
        {
            ph += phasors(i,j);
            phsum += phasors(i,j);
            if(j % apseg == apseg-1 ) //push the last one back
            {
                ph *= 1.0/(double)apseg; //average
                seg_amp[i].push_back( std::abs(ph) );
                seg_arg[i].push_back( std::arg(ph) );
                ph = 0.0;
            }
        }
        phsum *= 1.0/(double)naps;
        ch_amp.push_back( std::abs(phsum) );
        ch_arg.push_back( std::arg(phsum) );
    }

    //need to flatten and transpose this data to match the plot_data_dir format
    std::vector< double > transposed_flatted_seg_amp;
    std::vector< double > transposed_flatted_seg_arg;

    std::size_t xsize = seg_amp.size();
    std::size_t ysize = seg_amp[0].size();

    for(std::size_t j=0; j<ysize; j++)
    {
        for(std::size_t i=0; i<xsize; i++)
        {
            transposed_flatted_seg_amp.push_back( seg_amp[i][j] );
            transposed_flatted_seg_arg.push_back( seg_arg[i][j] );
        }
    }

    plot_dict["SEG_AMP"] = transposed_flatted_seg_amp;
    plot_dict["SEG_PHS"] = transposed_flatted_seg_arg;


    plot_dict["NSeg"] = nseg;
    plot_dict["NPlots"] = nplot; //nchan+1
    plot_dict["StartPlot"] = 0;

    //add the 'PLOT_INFO' section
    std::vector<std::string> pltheader{
        "#Ch",
        "Freq(MHz)",
        "Phase",
        "Ampl",
        "SbdBox",
        "APsRf",
        "APsRm",
        "PCdlyRf",
        "PCdlyRm",
        "PCPhsRf",
        "PCPhsRm",
        "PCOffRf",
        "PCOffRm",
        "PCAmpRf",
        "PCAmpRm",
        "ChIdRf",
        "TrkRf",
        "ChIdRm",
        "TrkRm"
    };
    plot_dict["PLOT_INFO"]["header"] = pltheader;

    //includes the 'All' channel
    for(std::size_t i=0; i<nplot; i++)
    {
        double freq = std::get<0>(phasors).at(i);
        plot_dict["PLOT_INFO"]["#Ch"].push_back(channel_labels[i]);
        plot_dict["PLOT_INFO"]["Freq(MHz)"].push_back(freq);
        plot_dict["PLOT_INFO"]["Phase"].push_back(ch_arg[i]*(180.0/M_PI));
        plot_dict["PLOT_INFO"]["Ampl"].push_back(ch_amp[i]);
        plot_dict["PLOT_INFO"]["SbdBox"].push_back(fSBDBox[i]);
    }

    //just the normal channels (no 'All')
    for(std::size_t i=0; i<nplot-1; i++)
    {
        //the following two quanties are mis-named in the plot_data_dir file
        //but we'll make due for now, since we don't really compute these correctly yet...we ignore the min_weight parameter.
        //this is not AP's used by Ref station, but rather APs with USB data used
        plot_dict["PLOT_INFO"]["APsRf"].push_back(fNUSBAP[i]);
        //this is not AP's used by Rem station, but rather APs with LSB data used
        plot_dict["PLOT_INFO"]["APsRm"].push_back(fNLSBAP[i]);

        plot_dict["PLOT_INFO"]["PCdlyRf"].push_back(0.0);
        plot_dict["PLOT_INFO"]["PCdlyRm"].push_back(0.0);
        plot_dict["PLOT_INFO"]["PCPhsRf"].push_back(0.0);
        plot_dict["PLOT_INFO"]["PCPhsRm"].push_back(0.0);
        plot_dict["PLOT_INFO"]["PCOffRf"].push_back(0.0);
        plot_dict["PLOT_INFO"]["PCOffRm"].push_back(0.0);
        plot_dict["PLOT_INFO"]["PCAmpRf"].push_back(0.0);
        plot_dict["PLOT_INFO"]["PCAmpRm"].push_back(0.0);
        plot_dict["PLOT_INFO"]["ChIdRf"].push_back("-");
        plot_dict["PLOT_INFO"]["TrkRf"].push_back("-");
        plot_dict["PLOT_INFO"]["ChIdRm"].push_back("-");
        plot_dict["PLOT_INFO"]["TrkRm"].push_back("-");
    }

    //for the time being, we add the window info here:
    double sb_win_low = std::get<0>(sbd_amp)(0);
    double sb_win_high = std::get<0>(sbd_amp)( sbd_amp.GetSize() - 1);
    double sb_sep = std::get<0>(sbd_amp)(1) - std::get<0>(sbd_amp)(0);
    plot_dict["extra"]["sb_win"].push_back(sb_win_low);
    plot_dict["extra"]["sb_win"].push_back(sb_win_high+sb_sep);

    double mb_win_low = std::get<0>(mbd_amp)(0);
    double mb_win_high = std::get<0>(mbd_amp)( mbd_amp.GetSize() - 1);
    double mb_sep = std::get<0>(mbd_amp)(1) - std::get<0>(mbd_amp)(0);
    plot_dict["extra"]["mb_win"].push_back(mb_win_low);
    plot_dict["extra"]["mb_win"].push_back(mb_win_high + mb_sep);

    double dr_win_low = std::get<0>(dr_amp)(0);
    double dr_win_high = std::get<0>(dr_amp)( dr_amp.GetSize() - 1);
    double dr_sep = std::get<0>(dr_amp)(1) - std::get<0>(dr_amp)(0);
    plot_dict["extra"]["dr_win"].push_back(dr_win_low);
    plot_dict["extra"]["dr_win"].push_back(dr_win_high+dr_sep);

    //currently no dTEC fitting support
    plot_dict["extra"]["ion_win"].push_back(0.0);
    plot_dict["extra"]["ion_win"].push_back(0.0);

    double fringe_amp = fParamStore->GetAs<double>("/fringe/famp");
    double tsum_weights = fParamStore->GetAs<double>("/fringe/total_summed_weights");

    //we use the raw phase resid, because it is unaffected by the 'mbd_anchor' parameter
    double resid_phase = fParamStore->GetAs<double>("/fringe/raw_resid_phase_rad");

    double snr = fParamStore->GetAs<double>("/fringe/snr");
    double freqrms_phase, freqrms_amp, timerms_phase, timerms_amp, inc_avg_amp, inc_avg_amp_freq;
    calc_freqrms(phasors, resid_phase, fringe_amp, tsum_weights, snr, freqrms_phase, freqrms_amp, inc_avg_amp_freq);
    calc_timerms(phasors, nseg, apseg, resid_phase, fringe_amp, tsum_weights, snr, timerms_phase, timerms_amp, inc_avg_amp);

    plot_dict["extra"]["freqrms_phase"] = freqrms_phase;
    plot_dict["extra"]["freqrms_amp"] = freqrms_amp;
    plot_dict["extra"]["timerms_phase"] = timerms_phase;
    plot_dict["extra"]["timerms_amp"] = timerms_amp;
    plot_dict["extra"]["inc_avg_amp"] = inc_avg_amp;
    plot_dict["extra"]["inc_avg_amp_freq"] = inc_avg_amp_freq;

    //stuff this in the parameter store too (reorganize this later)
    fParamStore->Set("/fringe/freqrms_phase", freqrms_phase);
    fParamStore->Set("/fringe/freqrms_amp", freqrms_amp);
    fParamStore->Set("/fringe/timerms_phase", timerms_phase);
    fParamStore->Set("/fringe/timerms_amp", timerms_amp);

    //calculate the theory values
    int nchan =  fParamStore->GetAs<double>("/config/nchannels");
    double th_timerms_phase = MHO_BasicFringeInfo::calculate_theory_timerms_phase(nseg, snr);
    double th_timerms_amp = MHO_BasicFringeInfo::calculate_theory_timerms_amp(nseg, snr);
    double th_freqrms_phase = MHO_BasicFringeInfo::calculate_theory_freqrms_phase(nchan, snr);
    double th_freqrms_amp = MHO_BasicFringeInfo::calculate_theory_freqrms_amp(nchan, snr);

    plot_dict["extra"]["theory_timerms_phase"] = th_timerms_phase;
    plot_dict["extra"]["theory_timerms_amp"] = th_timerms_amp;
    plot_dict["extra"]["theory_freqrms_phase"] = th_freqrms_phase;
    plot_dict["extra"]["theory_freqrms_amp"] = th_freqrms_amp;

    //stuff theory values in the parameter store too
    fParamStore->Set("/fringe/theory_timerms_phase", th_timerms_phase);
    fParamStore->Set("/fringe/theory_timerms_amp", th_timerms_amp);
    fParamStore->Set("/fringe/theory_freqrms_phase", th_freqrms_phase);
    fParamStore->Set("/fringe/theory_freqrms_amp", th_freqrms_amp);

    //calculate the quality code
    std::string qc = calc_quality_code();
    fParamStore->Set("/fringe/quality_code", qc);
    plot_dict["Quality"] = qc;

    plot_dict["extra"]["nlags"] = fParamStore->GetAs<int>("/config/nlags");

}


void
MHO_ComputePlotData::calc_freqrms(phasor_type& phasors, double coh_avg_phase, double fringe_amp,
                                  double total_summed_weights, double snr,
                                  double& freqrms_phase, double& freqrms_amp, double& inc_avg_amp_freq)
{
    std::size_t nchan = phasors.GetDimension(0)-1; //-1 is for the 'all' channel tacked on the end
    std::size_t nap = phasors.GetDimension(1);

    freqrms_phase = 0;
    freqrms_amp = 0;
    inc_avg_amp_freq = 0.0;

    #pragma message("TODO FIXME -- implement amp_corr_fact.");
    double amp_corr_fact = 1.0;

    for(std::size_t ch = 0; ch < nchan; ch++)
    {
        std::complex<double> sum = 0;
        double sumwt = 0.0;
        for(std::size_t ap =0; ap < nap; ap ++)
        {
            double wt = (*fWeights)(0, ch, ap, 0);
            sum += wt*phasors(ch, ap);
            sumwt += wt;
        }
        inc_avg_amp_freq += std::abs(sum) * amp_corr_fact;

        if(sumwt == 0){sum = 0.0;}
        else{ sum /= sumwt; }

        double c = std::arg(sum) - coh_avg_phase;
        // condition to lie in [-pi,pi] interval
        //TODO FIXME -- this is the original implementation, but it is incorrect!
        #pragma message("TODO FIXME, this way of computing an average phase angle is incorrect, should compute the average vector first, then take the angle of that.")
        c = std::fmod(c, 2.0 * M_PI);
        if (c > M_PI){c -= 2.0 * M_PI;}
        else if (c < - M_PI){c += 2.0 * M_PI;}
        freqrms_phase += c * c;
        c = std::abs(sum) - fringe_amp;
        freqrms_amp += c * c;
    }

    if(nchan > 2)
    {
        // avoid 0/0 singularity
        freqrms_phase = std::sqrt(freqrms_phase / (nchan - 2) ) * 180./M_PI;
    }
    else
    {
        freqrms_phase = 0.0;
    }
    freqrms_amp = std::sqrt(freqrms_amp / nchan) * 100. / fringe_amp;

    inc_avg_amp_freq /= total_summed_weights;
    /* Noise bias correction ? */
    inc_avg_amp_freq /= ((1.0 + 1.0/(2.0 * snr*snr / (double) nchan )));
}


void
MHO_ComputePlotData::calc_timerms(phasor_type& phasors, std::size_t nseg, std::size_t apseg,
                                  double coh_avg_phase, double fringe_amp, double total_summed_weights, double snr,
                                  double& timerms_phase, double& timerms_amp, double& inc_avg_amp)
{
    #pragma message("TODO FIXME -- implement amp_corr_fact.");
    double amp_corr_fact = 1.0;

    timerms_phase = 0;
    timerms_amp = 0;
    inc_avg_amp = 0.0;

    std::size_t nplot = phasors.GetDimension(0);
    std::size_t nchan = nplot-1; //-1 is for the 'all' channel tacked on the end
    std::size_t naps = phasors.GetDimension(1);
    auto chan_ax = &( std::get<CHANNEL_AXIS>(*fSBDArray) );
    double totwt, totap, wt, wtf, wtf_dsb, wt_dsb, apwt, ap_in_seg, usbfrac, lsbfrac, c;
    totwt = 0.0; totap = 0.0;

    std::complex<double> vsum, vsumf, wght_phsr;

    std::string net_sideband = "?";
    for(std::size_t seg = 0; seg < nseg; seg++)
    {
        vsum = 0.0;
        wt = 0.0;                       /* Loop over freqs, and ap's in segment */
        wt_dsb = 0.0;                   /* Loop over freqs, and ap's in segment */
                                        /* forming vector sum */
        for(std::size_t fr = 0; fr < nchan; fr++)
        {
            vsumf = 0.0;
            wtf = 0.0;
            wtf_dsb = 0.0;
            ap_in_seg = 0.0;
            usbfrac = lsbfrac = 0.0;

            MHO_IntervalLabel ilabel(fr,fr);
            std::string sidebandlabelkey = "net_sideband";
            auto other_labels = chan_ax->GetIntervalsWhichIntersect(ilabel);
            for(auto olit = other_labels.begin(); olit != other_labels.end(); olit++)
            {
                if( olit->HasKey(sidebandlabelkey) )
                {
                    olit->Retrieve(sidebandlabelkey, net_sideband);
                    break;
                }
            }

            for(std::size_t ap = seg * apseg; ap < (seg+1)*apseg; ap++)
            {
                if(ap >= naps){break;}
                apwt = fabs( (*fWeights)(0, fr, ap, 0) );
                wt += apwt;
                wtf += apwt;

                if(net_sideband == "U")
                {
                    usbfrac += apwt;
                    wtf_dsb += apwt;
                    wt_dsb += apwt;
                }
                else if(net_sideband == "L")
                {
                    lsbfrac += apwt;
                    wtf_dsb += apwt;
                    wt_dsb += apwt;
                }

                if (apwt > 0.0){totap += 1.0;}
                wght_phsr = phasors(fr,ap) * apwt;
                vsum = vsum + wght_phsr;
                vsumf = vsumf + wght_phsr;
            }
        }

        c = std::arg(vsum) - coh_avg_phase;
        // condition to lie in [-pi,pi] interval
        c = std::fmod (c, 2.0 * M_PI);
        if (c > M_PI){c -= 2.0 * M_PI;}
        else if (c < - M_PI){ c += 2.0 * M_PI;}
        timerms_phase += wt_dsb * c * c;
        /* Performs scalar sum over segments */
        /* of vector sums within segments and */
        /* over all freqs */
        c = std::abs(vsum);
        inc_avg_amp += c * amp_corr_fact;
        if (wt_dsb == 0){ c = 0.0; }
        else { c = c / wt_dsb; }
        /* delres_max is amplitude at peak */
        c = c * amp_corr_fact - fringe_amp;
        timerms_amp += wt_dsb * c*c;
        totwt += wt_dsb;
    }

    /* This removes noise bias based on */
    /* SNR of each segment/freq */
    inc_avg_amp /= ((1.0 + (float)nseg/(2.0 * snr * snr)));
    inc_avg_amp /= totwt;

    /* Correct rms values for fringe segmenting */
    timerms_phase = std::sqrt(timerms_phase / totwt) * 180. / M_PI;
    timerms_amp = sqrt(timerms_amp / totwt) * 100./fringe_amp;
}


std::string
MHO_ComputePlotData::calc_quality_code()
{
    //retrieve from param store
    double freqrms_phase = fParamStore->GetAs<double>("/fringe/freqrms_phase");
    double freqrms_amp = fParamStore->GetAs<double>("/fringe/freqrms_amp");
    double timerms_phase = fParamStore->GetAs<double>("/fringe/timerms_phase");
    double timerms_amp = fParamStore->GetAs<double>("/fringe/timerms_amp");

    double th_timerms_phase = fParamStore->GetAs<double>("/fringe/theory_timerms_phase");
    double th_timerms_amp = fParamStore->GetAs<double>("/fringe/theory_timerms_amp");
    double th_freqrms_phase = fParamStore->GetAs<double>("/fringe/theory_freqrms_phase");
    double th_freqrms_amp = fParamStore->GetAs<double>("/fringe/theory_freqrms_amp");

    char qcode[1];
    qcode[0] = '9';
    if (timerms_phase > 11.46 && th_timerms_phase < 5.73){(*qcode)--;}
    if (timerms_phase > 22.92 && th_timerms_phase < 11.46){(*qcode)--;}
    if (freqrms_phase > 11.46 && th_freqrms_phase < 5.73){(*qcode)--;}
    if (freqrms_phase > 22.92 && th_freqrms_phase < 11.46){(*qcode)--;}
    if (timerms_amp > 20.0 && th_timerms_amp < 10.0){(*qcode)--;}
    if (timerms_amp > 40.0 && th_timerms_amp < 20.0){(*qcode)--;}
    if (freqrms_amp > 20.0 && th_freqrms_amp < 10.0){(*qcode)--;}
    if (freqrms_amp > 40.0 && th_freqrms_amp < 20.0){(*qcode)--;}
    if (*qcode < '1') *qcode = '1';

    double prob_false = fParamStore->GetAs<double>("/fringe/prob_false_detect");
    // No fringes, 0-code
    if(prob_false > 1.E-4) *qcode = '0';
    return std::string(1, qcode[0]);
}


}//end namespace
