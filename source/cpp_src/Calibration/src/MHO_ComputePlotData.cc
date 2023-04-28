#include "MHO_ComputePlotData.hh"

#include "MHO_UniformGridPointsCalculator.hh"

namespace hops
{







xpower_amp_type
MHO_ComputePlotData::calc_mbd()
{

    //calculate the frequency grid for the channel -> MBD FFT
    MHO_UniformGridPointsCalculator fGridCalc;
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

    MHO_FringeRotation frot;

    std::size_t POLPROD = 0;
    std::size_t nchan = fSBDArray->GetDimension(CHANNEL_AXIS);
    std::size_t nap = fSBDArray->GetDimension(TIME_AXIS);

    auto chan_ax = &( std::get<CHANNEL_AXIS>(*fSBDArray) );
    auto ap_ax = &(std::get<TIME_AXIS>(*fSBDArray));
    auto sbd_ax = &( std::get<FREQ_AXIS>(*fSBDArray) );
    double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);

    //TODO FIXME -- should this be the fourfit refrence time? Also...should this be calculated elsewhere?
    double midpoint_time = ( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    std::cout<<"time midpoint = "<<midpoint_time<<std::endl;


    std::complex<double> sum = 0;
    for(std::size_t ch=0; ch < nchan; ch++)
    {
        double freq = (*chan_ax)(ch);//sky freq of this channel
        sum = 0;
        for(std::size_t ap=0; ap < nap; ap++)
        {
            double tdelta = ap_ax->at(ap) + ap_delta/2.0 - midpoint_time; //need time difference from the f.r.t?
            std::complex<double> vis = (*fSBDArray)(POLPROD, ch, ap, fSBDMaxBin); //pick out data at SBD max bin
            std::complex<double> vr = frot.vrot(tdelta, freq, fRefFreq, fDelayRate, 0.0); //apply at MBD=0.0
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

    MHO_FringeRotation frot;

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
    double midpoint_time = ( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    std::cout<<"time midpoint = "<<midpoint_time<<std::endl;

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
                double tdelta = ap_ax->at(ap) + ap_delta/2.0 - midpoint_time; //need time difference from the f.r.t?
                std::complex<double> vis = (*fSBDArray)(POLPROD, ch, ap, i);
                std::complex<double> vr = frot.vrot(tdelta, freq, fRefFreq, fDelayRate, fMBDelay);
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

    MHO_FringeRotation frot;

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
    double midpoint_time = ( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    std::cout<<"time midpoint = "<<midpoint_time<<std::endl;

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
                double tdelta = ap_ax->at(ap) + ap_delta/2.0 - midpoint_time; //need time difference from the f.r.t?
                std::complex<double> vis = (*fSBDArray)(POLPROD, ch, ap, i);
                std::complex<double> vr = frot.vrot(tdelta, freq, fRefFreq, fDelayRate, fMBDelay);
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

    // int nl = nbins/4;
    // std::complex<double> imagUnit(0.0, 1.0);
    // for( int i = 0; i < 2*nl; i++)
    // {
    //    int j = nl - i;
    //    double sb_factor = sqrt (0.5) / (M_PI * total_summed_weights);
    //    // if (j <= 0)
    //    //     sb_factor = status.total_usb_frac > 0 ?
    //    //         sqrt (0.5) / (M_PI * status.total_usb_frac) : 0.0;
    //    // else
    //    //     sb_factor = status.total_lsb_frac > 0 ?
    //    //         sqrt (0.5) / (M_PI * status.total_lsb_frac) : 0.0;
    //
    //    if (j < 0)
    //        j += 4*nl;
    //
    //    double bin = i;
    //    std::complex<double> z = sb_factor*std::exp( 1.0 * imagUnit * fSBDelay * bin * M_PI / sbd_delta * 2.0 * (double)nl );
    //    sbd_xpower_out(i) *= sb_factor;// * sbd_xpower_out(i);
    //    //std::get<0>(sbd_xpower_in)(i) = std::get<0>(sbd_xpower_out)(i);
    //    // plot.cp_spectrum[i] = Y[j] * sb_factor;
    //    //                                  /* Counter rotate to eliminate sband delay */
    //    // Z = exp_complex(-cmplx_unit_I * (status.sbd_max * (i-nl) * M_PI / (status.sbd_sep * 2*nl)));
    //    // plot.cp_spectrum[i] = Z * plot.cp_spectrum[i];
    // }


    return sbd_xpower_out;




    //                                     /* FFT sband spectrum -> XPower spectrum */
    // fftplan = fftw_plan_dft_1d (nbins, (fftw_complex*) sbd_po, (fftw_complex*) Y, FFTW_FORWARD, FFTW_ESTIMATE);
    // fftw_execute (fftplan);
                                        // scale crosspower spectra for correct amplitude
    // for (i = 0; i < 2*nl; i++)
    // {
    //    j = nl - i;
    //    if (j <= 0)
    //        sb_factor = status.total_usb_frac > 0 ?
    //            sqrt (0.5) / (M_PI * status.total_usb_frac) : 0.0;
    //    else
    //        sb_factor = status.total_lsb_frac > 0 ?
    //            sqrt (0.5) / (M_PI * status.total_lsb_frac) : 0.0;
    //
    //    if (j < 0)
    //        j += 4*nl;
    //
    //    plot.cp_spectrum[i] = Y[j] * sb_factor;
    //                                     /* Counter rotate to eliminate sband delay */
    //    Z = exp_complex(-cmplx_unit_I * (status.sbd_max * (i-nl) * M_PI / (status.sbd_sep * 2*nl)));
    //    plot.cp_spectrum[i] = Z * plot.cp_spectrum[i];
    // }


}






















xpower_amp_type
MHO_ComputePlotData::calc_dr()
{
    //grab the total summed weights
    double total_summed_weights = 1.0;
    fWeights->Retrieve("total_summed_weights", total_summed_weights);

    MHO_FringeRotation frot;
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
    double midpoint_time = ( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    std::cout<<"time midpoint = "<<midpoint_time<<std::endl;

    for(std::size_t ch=0; ch < nchan; ch++)
    {
        double freq = (*chan_ax)(ch);//sky freq of this channel
        for(std::size_t ap=0; ap < nap; ap++)
        {
            double tdelta = ap_ax->at(ap) + ap_delta/2.0 - midpoint_time; //need time difference from the f.r.t?
            std::complex<double> vis = (*fSBDArray)(POLPROD, ch, ap, fSBDMaxBin); //pick out data at SBD max bin
            std::complex<double> vr = frot.vrot(tdelta, freq, fRefFreq, fDelayRate, fMBDelay); //why rotate at the max delay rate??
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

    MHO_FringeRotation frot;
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


    frot.SetSBDSeparation(sbd_delta);
    frot.SetSBDMaxBin(fSBDMaxBin);
    frot.SetNSBDBins(sbd_ax->GetSize()/4);  //this is nlags, FACTOR OF 4 is because sbd space is padded by a factor of 4
    //frot.SetSBDMax( (*sbd_ax)(fSBDMaxBin) );
    frot.SetSBDMax( fSBDelay );

    //TODO FIXME -- should this be the fourfit refrence time? Also...should this be calculated elsewhere?
    double midpoint_time = ( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    std::cout<<"time midpoint = "<<midpoint_time<<std::endl;

    std::complex<double> sum_all = 0.0;
    for(std::size_t ch=0; ch < nchan; ch++)
    {
        double freq = (*chan_ax)(ch);//sky freq of this channel
        #pragma message("TODO FIXME FOR NON-LSB DATA!")
        frot.SetSideband(-1);
        for(std::size_t ap=0; ap < nap; ap++)
        {
            double tdelta = ap_ax->at(ap) + ap_delta/2.0 - midpoint_time; //need time difference from the f.r.t?
            std::complex<double> vis = (*fSBDArray)(POLPROD, ch, ap, fSBDMaxBin); //pick out data at SBD max bin
            std::complex<double> vr = frot.vrot(tdelta, freq, fRefFreq, fDelayRate, fMBDelay);
            std::complex<double> z = vis*vr;
            //apply weight and sum
            double w = (*fWeights)(POLPROD, ch, ap, 0);
            std::complex<double> wght_phsr = z*w;
            sum_all += wght_phsr;
        }
    }

    std::cout<<"sbd sep = "<<sbd_delta<<" sbd max = "<<fSBDelay<<std::endl;

    std::cout<<"sum all = "<<sum_all<<std::endl;
    double coh_avg_phase = std::arg(sum_all);
    return coh_avg_phase; //not quite the value which is displayed in the fringe plot (see fill type 208)
}


}
