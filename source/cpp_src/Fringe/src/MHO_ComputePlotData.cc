#include "MHO_ComputePlotData.hh"

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
    fRefFreq = fParamStore->GetAs<double>("/config/ref_freq");
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
    double frt_offset = fParamStore->GetAs<double>("frt_offset");

    std::complex<double> sum = 0;
    for(std::size_t ch=0; ch < nchan; ch++)
    {
        double freq = (*chan_ax)(ch);//sky freq of this channel
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
    double frt_offset = fParamStore->GetAs<double>("frt_offset");

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





xpower_type
MHO_ComputePlotData::calc_xpower()
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
    double frt_offset = fParamStore->GetAs<double>("frt_offset");

    sbd_amp.Resize(nbins);
    sbd_xpower_in.Resize(nbins);
    sbd_xpower_out.Resize(nbins); //interpolation

    sbd_xpower_in.ZeroArray();
    sbd_xpower_out.ZeroArray();

    //loop over sbd bins (4*nlags) and sum over channel/ap
    for(std::size_t i=0; i<nbins; i++)
    {
        std::complex<double> sum = 0;
        for(std::size_t ch=0; ch < nchan; ch++)
        {
            double freq = (*chan_ax)(ch);//sky freq of this channel
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
        std::get<0>(sbd_xpower_in)(i) = (*sbd_ax)(i);


    }

    fCyclicRotator.SetOffset(0, nbins/2);
    fCyclicRotator.SetArgs(&sbd_xpower_in);
    bool ok = fCyclicRotator.Initialize();
    check_step_fatal(ok, "calibration", "MBD search cyclic rotation initialization." << eom );

    //set up FFT and rotator engines
    fFFTEngine.SetArgs(&sbd_xpower_in, &sbd_xpower_out);
    fFFTEngine.DeselectAllAxes();
    fFFTEngine.SelectAxis(0);
    fFFTEngine.SetForward();
    ok = fFFTEngine.Initialize();
    check_step_fatal(ok, "calibration", "MBD search fft engine initialization." << eom );

    //ok = fCyclicRotator.Execute();
    check_step_fatal(ok, "calibration", "MBD search cyclic rotation execution." << eom );

    //now run an FFT along the sbd axis and cyclic rotate
    ok = fFFTEngine.Execute();
    check_step_fatal(ok, "calibration", "MBD search fft engine execution." << eom );

    return sbd_xpower_out;
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
    double frt_offset = fParamStore->GetAs<double>("frt_offset");

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
    double frt_offset = fParamStore->GetAs<double>("frt_offset");
    auto dr_ax = &(std::get<0>(fDRWorkspace) );

    for(std::size_t i=0; i<drsp_size;i++)
    {
        dr_ax->at(i) = i*ap_delta;
    }


    for(std::size_t ch=0; ch < nchan; ch++)
    {
        double freq = (*chan_ax)(ch);//sky freq of this channel
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
    double frt_offset = fParamStore->GetAs<double>("frt_offset");

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

    // std::cout<<"sbd sep = "<<sbd_delta<<" sbd max = "<<fSBDelay<<std::endl;
    // std::cout<<"sum all = "<<sum_all<<std::endl;

    double coh_avg_phase = std::arg(sum_all);

    return coh_avg_phase; //not quite the value which is displayed in the fringe plot (see fill type 208)
}







xpower_type
MHO_ComputePlotData::calc_xpower_KLUDGE()
{
    //kludge version
    #pragma message("TODO FIXME XPOWER KLUDGE")
    //grab the total summed weights
    double total_summed_weights = 1.0;
    fWeights->Retrieve("total_summed_weights", total_summed_weights);

    int nlags = fSBDArray->GetDimension(FREQ_AXIS)/4;
    int nl = nlags;

    int MAXLAG = 128;//8192;
    xpower_type X;
    xpower_type Y;
    xpower_type cp_spectrum;
    X.Resize(2*MAXLAG); X.ZeroArray();
    Y.Resize(4*MAXLAG); Y.ZeroArray();
    cp_spectrum.Resize(2*nl); cp_spectrum.ZeroArray();

    std::size_t POLPROD = 0;
    std::size_t nchan = fSBDArray->GetDimension(CHANNEL_AXIS);
    std::size_t nap = fSBDArray->GetDimension(TIME_AXIS);
    std::size_t nbins = fSBDArray->GetDimension(FREQ_AXIS);

    auto chan_ax = &( std::get<CHANNEL_AXIS>(*fSBDArray) );
    auto ap_ax = &(std::get<TIME_AXIS>(*fSBDArray));
    auto sbd_ax = &( std::get<FREQ_AXIS>(*fSBDArray) );
    double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);
    double frt_offset = fParamStore->GetAs<double>("frt_offset");

    std::complex<double> sum;
    std::complex<double> Z, vr;
    double frac;
    double bw;
    std::string net_sideband = "?";
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


            #pragma message("TODO FIXME FOR NON-LSB DATA!")
            fRot.SetSideband(0); //DSB
            if(net_sideband == "U")
            {
                fRot.SetSideband(1);
            }

            if(net_sideband == "L")
            {
                fRot.SetSideband(-1);
            }

            sum = 0.0;
            for (int ap = 0; ap < nap; ap++)
            {
                double tdelta = (ap_ax->at(ap) + ap_delta/2.0) - frt_offset; //need time difference from the f.r.t?
                std::complex<double> vis = (*fSBDArray)(POLPROD, ch, ap, 2*lag);
                std::complex<double> vr = fRot.vrot(tdelta, freq, fRefFreq, fDelayRate, fMBDelay);
                std::complex<double> Z = vis*vr;
                //apply weight and sum
                double w = (*fWeights)(POLPROD, ch, ap, 0);
                sum += w*Z;
            }
            X[lag] = X[lag] + sum;
        }
        //need to understand this
        int j = lag - nl;
        if(j < 0){j += 4 * nl;}
        if(lag == 0){j = 2 * nl;}             // pure real lsb/dc channel goes in middle??
        Y[j] = X[lag];
        std::get<0>(Y)(j) = j;
    }

    //set up FFT
    fFFTEngine.SetArgs(&Y, &Y);
    fFFTEngine.DeselectAllAxes();
    fFFTEngine.SelectAxis(0);
    fFFTEngine.SetForward();
    bool ok = fFFTEngine.Initialize();
    check_step_fatal(ok, "calibration", "MBD search fft engine initialization." << eom );

    //now run an FFT along the MBD axis and cyclic rotate
    ok = fFFTEngine.Execute();
    check_step_fatal(ok, "calibration", "MBD search fft engine execution." << eom );

    std::complex<double> cmplx_unit_I(0.0, 1.0);
    int s =  Y.GetDimension(0)/2;
    cp_spectrum.Resize(s);


    //this is seriously broken
    if(net_sideband == "L")
    {
        for(int i=0; i<s; i++)
        {
            cp_spectrum(s-i-1) = Y(i);
            Z = std::exp(cmplx_unit_I * (fSBDelay * (i-s) * M_PI / (sbd_delta *2.0* s)));
            cp_spectrum[s-i-1] *= Z * (sqrt(0.5)/total_summed_weights );
            std::get<0>(cp_spectrum)(s-i-1) = -1.0*(bw)*((double)i/(double)s); //label freq ax
        }
    }
    else
    {
        for(int i=0; i<s; i++)
        {
            cp_spectrum(i) = Y(i+s-1);
            Z = std::exp(cmplx_unit_I * (fSBDelay * (i-s) * M_PI / (sbd_delta *2.0* s)));
            cp_spectrum(i) *= Z * (sqrt(0.5)/total_summed_weights );
            std::get<0>(cp_spectrum)(i) = -1.0*(bw)*((double)i/(double)s); //label freq ax
        }
    }


    return cp_spectrum;

}




















xpower_type
MHO_ComputePlotData::calc_xpower_KLUDGE2()
{
    //kludge version
    #pragma message("TODO FIXME XPOWER KLUDGE")
    //grab the total summed weights
    double total_summed_weights = 1.0;
    fWeights->Retrieve("total_summed_weights", total_summed_weights);

    int N = fVisibilities->GetDimension(FREQ_AXIS);
    xpower_type X;
    xpower_type Y;
    X.Resize(N);
    Y.Resize(N);
    xpower_type cp_spectrum;

    std::size_t POLPROD = 0;
    std::size_t nchan = fVisibilities->GetDimension(CHANNEL_AXIS);
    std::size_t nap = fVisibilities->GetDimension(TIME_AXIS);
    std::size_t nbins = fVisibilities->GetDimension(FREQ_AXIS);

    auto chan_ax = &( std::get<CHANNEL_AXIS>(*fVisibilities) );
    auto ap_ax = &(std::get<TIME_AXIS>(*fVisibilities));
    auto freq_ax = &( std::get<FREQ_AXIS>(*fVisibilities) );
    double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    double freq_delta = freq_ax->at(1) - freq_ax->at(0);


    //double frt_offset = ( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    double frt_offset = fParamStore->GetAs<double>("frt_offset");

    std::complex<double> sum;
    std::complex<double> Z, vr;
    double frac;
    double bw;
    X.ZeroArray();
    for(int n = 0; n < N; n++)
    {
        for(int ch = 0; ch < nchan; ch++)
        {
            double freq = (*chan_ax)(ch);//sky freq of this channel
            MHO_IntervalLabel ilabel(ch,ch);
            std::string net_sideband = "?";
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
                fRot.SetSideband(1);
            }

            if(net_sideband == "L")
            {
                fRot.SetSideband(-1);
            }

            sum = 0.0;
            for (int ap = 0; ap < nap; ap++)
            {
                double tdelta = (ap_ax->at(ap) + ap_delta/2.0) - frt_offset; //need time difference from the f.r.t?
                std::complex<double> vis = (*fVisibilities)(POLPROD, ch, ap, n);
                std::complex<double> vr = fRot.vrot(tdelta, freq, fRefFreq, fDelayRate, fMBDelay);
                std::complex<double> Z = vis*vr;
                //apply weight and sum
                double w = (*fWeights)(POLPROD, ch, ap, 0);
                sum += w*Z;
            }
            X[n] += sum;
        }
        (&std::get<0>(X))->at(n) = (*freq_ax)(n);
    }

    // MHO_EndZeroPadder< xpower_type > padder;
    // padder.SetPaddingFactor(2);
    // padder.SetEndPadded();
    // padder.SetArgs(&X);
    // bool init = padder.Initialize();
    // bool exe = padder.Execute();
    //
    //
    // //set up FFT
    // fFFTEngine.SetArgs(&X);
    // fFFTEngine.DeselectAllAxes();
    // fFFTEngine.SelectAxis(0);
    // fFFTEngine.SetForward();
    // bool ok = fFFTEngine.Initialize();
    // check_step_fatal(ok, "calibration", "MBD search fft engine initialization." << eom );
    //
    // //now run an FFT along the MBD axis and cyclic rotate
    // ok = fFFTEngine.Execute();
    // check_step_fatal(ok, "calibration", "MBD search fft engine execution." << eom );
    //
    // //set up FFT
    // fFFTEngine.SetArgs(&X);
    // fFFTEngine.DeselectAllAxes();
    // fFFTEngine.SelectAxis(0);
    // fFFTEngine.SetBackward();
    // ok = fFFTEngine.Initialize();
    // check_step_fatal(ok, "calibration", "MBD search fft engine initialization." << eom );
    //
    // //now run an FFT along the MBD axis and cyclic rotate
    // ok = fFFTEngine.Execute();
    // check_step_fatal(ok, "calibration", "MBD search fft engine execution." << eom );
    //


    //now run an FFT along the MBD axis and cyclic rotate
    // ok = fFFTEngine.Execute();
    check_step_fatal(ok, "calibration", "MBD search fft engine execution." << eom );

    std::complex<double> cmplx_unit_I(0.0, 1.0);
    cp_spectrum.Resize(X.GetSize());

    // std::cout<<"sbd delay = "<<fSBDelay<<std::endl;
    // std::cout<<"sbd delta = "<<freq_delta<<std::endl;

    for(int i=0; i<X.GetSize(); i++)
    {
        cp_spectrum(i) = X(i);
        double arg = 2.0*M_PI*fSBDelay*(std::get<0>(X)(i));
        Z = std::exp(-1.0*cmplx_unit_I * arg) ;
        cp_spectrum[i] *= Z;//
        cp_spectrum[i] *= (sqrt(0.5)/total_summed_weights );
        std::get<0>(cp_spectrum)(i) = std::get<0>(X)(i);

    }

    return cp_spectrum;

}





























xpower_type
MHO_ComputePlotData::calc_xpower_KLUDGE3()
{
    //kludge version
    #pragma message("TODO FIXME XPOWER KLUDGE")
    //grab the total summed weights
    double total_summed_weights = 1.0;
    fWeights->Retrieve("total_summed_weights", total_summed_weights);

    int N = fSBDArray->GetDimension(FREQ_AXIS);
    int NLAGS = fVisibilities->GetDimension(FREQ_AXIS);

    xpower_type X;
    xpower_type Y;
    xpower_type cp_spectrum;
    X.Resize(N); X.ZeroArray();
    Y.Resize(N); Y.ZeroArray();
    cp_spectrum.Resize(N); cp_spectrum.ZeroArray();

    std::size_t POLPROD = 0;
    std::size_t nchan = fSBDArray->GetDimension(CHANNEL_AXIS);
    std::size_t nap = fSBDArray->GetDimension(TIME_AXIS);
    std::size_t nbins = fSBDArray->GetDimension(FREQ_AXIS);

    auto chan_ax = &( std::get<CHANNEL_AXIS>(*fSBDArray) );
    auto ap_ax = &(std::get<TIME_AXIS>(*fSBDArray));
    auto sbd_ax = &( std::get<FREQ_AXIS>(*fSBDArray) );
    double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);

    //double frt_offset = ( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    double frt_offset = fParamStore->GetAs<double>("frt_offset");

    std::complex<double> sum;
    std::complex<double> Z, vr;
    double frac;
    double bw;
    for(int lag = 0; lag < N; lag++)
    {

        for(int ch = 0; ch < nchan; ch++)
        {
            double freq = (*chan_ax)(ch);//sky freq of this channel
            MHO_IntervalLabel ilabel(ch,ch);
            std::string net_sideband = "?";
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
                fRot.SetSideband(1);
            }

            if(net_sideband == "L")
            {
                fRot.SetSideband(-1);
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
            X[lag] = X[lag] + sum;
            std::get<0>(X)(lag) = (*sbd_ax)(lag);
        }
    }

    //std::cout<<X<<std::endl;

    //set up FFT
    fFFTEngine.SetArgs(&X);
    fFFTEngine.DeselectAllAxes();
    fFFTEngine.SelectAxis(0);
    fFFTEngine.SetForward();
    bool ok = fFFTEngine.Initialize();
    check_step_fatal(ok, "calibration", "MBD search fft engine initialization." << eom );

    //now run an FFT along the MBD axis and cyclic rotate
    ok = fFFTEngine.Execute();
    check_step_fatal(ok, "calibration", "MBD search fft engine execution." << eom );

    std::complex<double> cmplx_unit_I(0.0, 1.0);
    int s =  X.GetDimension(0);
    cp_spectrum.Resize(128);

    for(int i=0; i<128; i++)
    {
        cp_spectrum(i) = X(i);
        Z = std::exp(-1.0*cmplx_unit_I * (fSBDelay * (i) * M_PI / (sbd_delta *2.0* s)));
        cp_spectrum(i) *= Z * (sqrt(0.5)/total_summed_weights );
        std::get<0>(cp_spectrum)(i) = -1.0*(bw)*((double)i/(double)s); //label freq ax
        //std::cout<<"i, abs(c)  ="<<i<<", "<<std::abs(cp_spectrum(i))<<std::endl;
    }

    return cp_spectrum;
}






void
MHO_ComputePlotData::DumpInfoToJSON(mho_json& plot_dict)
{
    auto sbd_amp = calc_sbd();
    auto mbd_amp = calc_mbd();
    auto dr_amp = calc_dr();
    //TODO FIXME -- move the residual phase calc elsewhere (but we need it for the moment to set the fRot parameters)
    double coh_avg_phase = calc_phase();
    auto sbd_xpower = calc_xpower_KLUDGE();
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

    //grab the per-channe/AP phasors, and average down if necessary
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
        plot_dict["PLOT_INFO"]["SbdBox"].push_back(0.0);
    }

    //just the normal channels (no 'All')
    for(std::size_t i=0; i<nplot-1; i++)
    {
        plot_dict["PLOT_INFO"]["APsRf"].push_back(0.0);
        plot_dict["PLOT_INFO"]["APsRm"].push_back(0.0);
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

}



}//end namespace
