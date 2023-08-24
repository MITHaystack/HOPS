#include "MHO_ComputePlotData.hh"

#include "MHO_UniformGridPointsCalculator.hh"
#include "MHO_EndZeroPadder.hh"
namespace hops
{

MHO_ComputePlotData::MHO_ComputePlotData()
{
    fMBDAnchor = "model";
    fParamStore = nullptr;
};

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

    std::cout<<"sizes = "<<fNGridPoints<<", "<<fNSBD<<", "<<fNDR<<std::endl;

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

    //TODO FIXME -- should this be the fourfit refrence time? Also...should this be calculated elsewhere?
    double frt_offset = fParamStore->GetAs<double>("frt_offset");
    
    //( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    std::cout<<"time midpoint = "<<frt_offset<<std::endl;


    std::complex<double> sum = 0;
    for(std::size_t ch=0; ch < nchan; ch++)
    {
        double freq = (*chan_ax)(ch);//sky freq of this channel
        sum = 0;
        for(std::size_t ap=0; ap < nap; ap++)
        {
            double tdelta = ap_ax->at(ap) + ap_delta/2.0 - frt_offset; //need time difference from the f.r.t?
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


    //TODO FIXME -- shoudl this be the fourfit refrence time? Also...should this be calculated elsewhere?
    //double frt_offset = ( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    double frt_offset = fParamStore->GetAs<double>("frt_offset");
    std::cout<<"time midpoint = "<<frt_offset<<std::endl;

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
                double tdelta = ap_ax->at(ap) + ap_delta/2.0 - frt_offset; //need time difference from the f.r.t?
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


    //TODO FIXME -- shoudl this be the fourfit refrence time? Also...should this be calculated elsewhere?
    //double frt_offset = ( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    double frt_offset = fParamStore->GetAs<double>("frt_offset");
    std::cout<<"time midpoint = "<<frt_offset<<std::endl;

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
                double tdelta = ap_ax->at(ap) + ap_delta/2.0 - frt_offset; //need time difference from the f.r.t?
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

        std::cout<<"sbd_xpower_in @ "<< i << " = " << sbd_xpower_in(i) <<std::endl; //at this point SBD AMP is correct
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

    for(std::size_t i=0;i<sbd_xpower_out.GetSize(); i++)
    {
        std::cout<<"sbd_xpower_out @ "<< i << " = " << sbd_xpower_out(i) <<std::endl; //at this point SBD AMP is correct
    }

    return sbd_xpower_out;

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
    std::cout<<"DRSP size = "<<drsp_size<<std::endl;
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

    auto dr_ax = &(std::get<0>(fDRWorkspace) );
    for(std::size_t i=0; i<drsp_size;i++)
    {
        dr_ax->at(i) = i*ap_delta;
    }

    //TODO FIXME -- should this be the fourfit refrence time? Also...should this be calculated elsewhere?
    //double frt_offset = ( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    double frt_offset = fParamStore->GetAs<double>("frt_offset");
    std::cout<<"time midpoint = "<<frt_offset<<std::endl;

    for(std::size_t ch=0; ch < nchan; ch++)
    {
        double freq = (*chan_ax)(ch);//sky freq of this channel
        for(std::size_t ap=0; ap < nap; ap++)
        {
            double tdelta = ap_ax->at(ap) + ap_delta/2.0 - frt_offset; //need time difference from the f.r.t?
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
    fRot.SetNSBDBins(sbd_ax->GetSize()/4);  //this is nlags, FACTOR OF 4 is because sbd space is padded by a factor of 4
    //fRot.SetSBDMax( (*sbd_ax)(fSBDMaxBin) );
    fRot.SetSBDMax( fSBDelay );

    //TODO FIXME -- should this be the fourfit refrence time? Also...should this be calculated elsewhere?
    double frt_offset = fParamStore->GetAs<double>("frt_offset");
    //double frt_offset = ( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    std::cout<<"time midpoint = "<<frt_offset<<std::endl;

    std::complex<double> sum_all = 0.0;
    for(std::size_t ch=0; ch < nchan; ch++)
    {
        double freq = (*chan_ax)(ch);//sky freq of this channel
        MHO_IntervalLabel ilabel(ch,ch);
        std::string net_sideband = "?";
        std::string sidebandlabelkey = "net_sideband";
        auto other_labels = chan_ax->GetIntervalsWhichIntersect(&ilabel);
        for(auto olit = other_labels.begin(); olit != other_labels.end(); olit++)
        {
            if( (*olit)->HasKey(sidebandlabelkey) )
            {
                (*olit)->Retrieve(sidebandlabelkey, net_sideband);
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

        for(std::size_t ap=0; ap < nap; ap++)
        {
            double tdelta = ap_ax->at(ap) + ap_delta/2.0 - frt_offset; //need time difference from the f.r.t?
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

    std::cout<<"sbd sep = "<<sbd_delta<<" sbd max = "<<fSBDelay<<std::endl;
    std::cout<<"sum all = "<<sum_all<<std::endl;

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

    //TODO FIXME -- should this be the fourfit refrence time? Also...should this be calculated elsewhere?
    //double frt_offset = ( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    double frt_offset = fParamStore->GetAs<double>("frt_offset");
    std::cout<<"time midpoint = "<<frt_offset<<std::endl;

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
            auto other_labels = chan_ax->GetIntervalsWhichIntersect(&ilabel);
            for(auto olit = other_labels.begin(); olit != other_labels.end(); olit++)
            {
                if( (*olit)->HasKey(sidebandlabelkey) )
                {
                    (*olit)->Retrieve(sidebandlabelkey, net_sideband);
                    break;
                }
            }
            
            for(auto olit = other_labels.begin(); olit != other_labels.end(); olit++)
            {
                if( (*olit)->HasKey(bandwidthlabelkey) )
                {
                    (*olit)->Retrieve(bandwidthlabelkey, bw);
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
                double tdelta = ap_ax->at(ap) + ap_delta/2.0 - frt_offset; //need time difference from the f.r.t?
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

    //TODO FIXME -- should this be the fourfit refrence time? Also...should this be calculated elsewhere?
    //double frt_offset = ( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    double frt_offset = fParamStore->GetAs<double>("frt_offset");
    std::cout<<"time midpoint = "<<frt_offset<<std::endl;

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
            auto other_labels = chan_ax->GetIntervalsWhichIntersect(&ilabel);
            for(auto olit = other_labels.begin(); olit != other_labels.end(); olit++)
            {
                if( (*olit)->HasKey(sidebandlabelkey) )
                {
                    (*olit)->Retrieve(sidebandlabelkey, net_sideband);
                    break;
                }
            }

            for(auto olit = other_labels.begin(); olit != other_labels.end(); olit++)
            {
                if( (*olit)->HasKey(bandwidthlabelkey) )
                {
                    (*olit)->Retrieve(bandwidthlabelkey, bw);
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
                double tdelta = ap_ax->at(ap) + ap_delta/2.0 - frt_offset; //need time difference from the f.r.t?
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

    std::cout<<"sbd delay = "<<fSBDelay<<std::endl;
    std::cout<<"sbd delta = "<<freq_delta<<std::endl;

    for(int i=0; i<X.GetSize(); i++)
    {
        cp_spectrum(i) = X(i);
        double arg = 2.0*M_PI*fSBDelay*(std::get<0>(X)(i));
        std::cout<<"arg "<<i<<" = "<<arg<<std::endl;
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

    //TODO FIXME -- should this be the fourfit refrence time? Also...should this be calculated elsewhere?
    //double frt_offset = ( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    double frt_offset = fParamStore->GetAs<double>("frt_offset");
    std::cout<<"time midpoint = "<<frt_offset<<std::endl;

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
            auto other_labels = chan_ax->GetIntervalsWhichIntersect(&ilabel);
            for(auto olit = other_labels.begin(); olit != other_labels.end(); olit++)
            {
                if( (*olit)->HasKey(sidebandlabelkey) )
                {
                    (*olit)->Retrieve(sidebandlabelkey, net_sideband);
                    break;
                }
            }
            
            for(auto olit = other_labels.begin(); olit != other_labels.end(); olit++)
            {
                if( (*olit)->HasKey(bandwidthlabelkey) )
                {
                    (*olit)->Retrieve(bandwidthlabelkey, bw);
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
                double tdelta = ap_ax->at(ap) + ap_delta/2.0 - frt_offset; //need time difference from the f.r.t?
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
    
    std::cout<<X<<std::endl;

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
        std::cout<<"i, abs(c)  ="<<i<<", "<<std::abs(cp_spectrum(i))<<std::endl;
    }
    
    return cp_spectrum;


    // //now run an FFT along the MBD axis and cyclic rotate
    // // ok = fFFTEngine.Execute();
    // check_step_fatal(ok, "calibration", "MBD search fft engine execution." << eom );
    // 
    // //std::complex<double> cmplx_unit_I(0.0, 1.0);
    // // cp_spectrum.Resize(N);
    // // 
    // // 
    // // std::cout<<"sbd delay = "<<fSBDelay<<std::endl;
    // // std::cout<<"sbd delta = "<<freq_delta<<std::endl;
    // // 
    // for(int i=0; i<N; i++)
    // {
    //     cp_spectrum(i) = X(i);
    //     // double arg = 2.0*M_PI*fSBDelay*((*freq_ax)(i));
    //     // std::cout<<"arg "<<i<<" = "<<arg<<std::endl;
    //     // Z = std::exp(-1.0*cmplx_unit_I * arg) ;
    //     // cp_spectrum[i] *= Z;//
    //     // cp_spectrum[i] *= (sqrt(0.5)/total_summed_weights );
    //     std::get<0>(cp_spectrum)(i) = std::get<0>(X)(i);
    // }
    // 
    // std::cout<<"BOOO"<<std::endl;
    // std::cout<<X<<std::endl;
    // 
    // return X;//cp_spectrum;

}



















void
MHO_ComputePlotData::DumpInfoToJSON(mho_json& plot_dict)
{
    auto sbd_amp = calc_sbd();
    auto mbd_amp = calc_mbd();
    auto dr_amp = calc_dr();
    auto sbd_xpower = calc_xpower_KLUDGE();
    double coh_avg_phase = calc_phase();

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

    mho_json exper_section = fVexInfo["$EXPER"];
    auto exper_info = exper_section.begin().value();

    mho_json src_section = fVexInfo["$SOURCE"];
    auto src_info = src_section.begin().value();

    mho_json freq_section = fVexInfo["$FREQ"];
    auto freq_info = freq_section.begin().value();
    double sample_rate = freq_info["sample_rate"]["value"];
    double samp_period = 1.0/(sample_rate*1e6);

    plot_dict["Quality"] = "-";

    //Poor imitation of SNR -- needs corrections
    //hardcoded dummy values right now
    double eff_npol = 1.0;
    double amp_corr_factor = 1.0;
    double fact1 = 1.0; //more than 16 lags
    double fact2 = 0.881; //2bit x 2bit
    double fact3 = 0.970; //difx
    double acc_period = ap_delta;
    double inv_sigma = fact1 * fact2 * fact3 * std::sqrt(acc_period/samp_period);
    plot_dict["SNR"] = fAmp * inv_sigma *  sqrt(fTotalSummedWeights * eff_npol)/(1e4* amp_corr_factor);

    std::size_t nchan = std::get<CHANNEL_AXIS>(*fVisibilities).GetSize();
    plot_dict["IntgTime"] = fTotalSummedWeights*acc_period /(double)nchan;

    plot_dict["Amp"] = fAmp;

    if( fMBDAnchor == "sbd" )
    {
        #pragma message("TODO FIXME -- when control file parameter mbd_anchor sbd is used there is an additional correction done to fringe phase, see fill_208.c line 158!!")
        msg_warn("calibration", "support for mbd_anchor is not yet implemented." <<eom);
    }

    plot_dict["ResPhase"] = std::fmod(coh_avg_phase * (180.0/M_PI), 360.0);
    plot_dict["PFD"] = "-";
    plot_dict["ResidSbd(us)"] = fSBDelay;
    plot_dict["ResidMbd(us)"] = fMBDelay;
    plot_dict["FringeRate(Hz)"]  = fFringeRate;
    plot_dict["IonTEC(TEC)"] = "-";
    plot_dict["RefFreq(MHz)"] = fRefFreq;
    plot_dict["AP(sec)"] = ap_delta;
    plot_dict["ExperName"] = exper_info["exper_name"];
    plot_dict["ExperNum"] = "-";
    plot_dict["YearDOY"] = "-";
    plot_dict["Start"] = "-";
    plot_dict["Stop"] = "-";
    plot_dict["FRT"] = "-";
    plot_dict["CorrTime"] = "-";
    plot_dict["FFTime"] = "-";
    plot_dict["BuildTime"] = "-";

    plot_dict["RA"] = src_info["ra"];
    plot_dict["Dec"] = src_info["dec"];

}























}
