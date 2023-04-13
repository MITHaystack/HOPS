#include "MHO_InterpolateFringePeak.hh"
#include "MHO_FringeRotation.hh"

namespace hops 
{

void MHO_InterpolateFringePeak::fine_peak_interpolation()
{
    //follow the algorithm of interp.c (SIMUL) mode, to fill out a cube and interpolate
    double drf[5][5][5];// 5x5x5 cube of fringe values
    double xlim[3][2]; //cube limits each dim
    double xi[3];
    double drfmax;

    auto chan_ax = &( std::get<CHANNEL_AXIS>(*fSBDArray) );
    auto ap_ax = &(std::get<TIME_AXIS>(*fSBDArray));
    auto sbd_ax = &( std::get<FREQ_AXIS>(*fSBDArray) );

    std::size_t nap = ap_ax->GetSize();
    std::size_t nchan = chan_ax->GetSize();

    double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);
    double dr_delta = fDRAxis->at(1) - fDRAxis->at(0);
    double mbd_delta = fMBDAxis->at(1) - fMBDAxis->at(0);

    double midpoint_time = ( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    std::cout<<"time midpoint = "<<midpoint_time<<std::endl;

    printf("max bin (sbd, mbd, dr) = %d, %d, %d\n", fSBDMaxBin, fMBDMaxBin, fDRMaxBin );
    printf("mbd delta, dr delta = %.7f, %.7f \n", mbd_delta, dr_delta/fRefFreq);


    double sbd_lower = 1e30;
    double sbd_upper = -1e30;
    double mbd_lower = 1e30;
    double mbd_upper = -1e30;
    double dr_lower = 1e30;
    double dr_upper = -1e30;

    MHO_FringeRotation frot;
    int sbd_bin, dr_bin, mbd_bin;
    double sbd, dr, mbd;

    std::cout<< std::setprecision(15);
    for (int isbd=0; isbd<5; isbd++)
    {
        for (int imbd=0; imbd<5; imbd++)
        {
            for(int idr=0; idr<5; idr++)
            {
                std::complex<double> z = 0.0;

                // calculate location of this tabular point (should modulo % axis size)
                sbd_bin = (fSBDMaxBin + isbd - 2) % (int) sbd_ax->GetSize();
                dr_bin = (fDRMaxBin + idr - 2 ) % (int) fDRAxis->GetSize() ;
                mbd_bin = ( fMBDMaxBin + imbd - 2) % (int) fMBDAxis->GetSize() ;;
                //
                sbd = sbd_ax->at( (std::size_t) sbd_bin);
                mbd = fMBDAxis->at(fMBDMaxBin) + 0.5 * (imbd - 2) * mbd_delta;
                dr  = (fDRAxis->at(fDRMaxBin) + (0.5 * (idr - 2)  * dr_delta) )/fRefFreq;

                printf("idr = %d and dr = %.8f \n", idr, dr);

                if(sbd < sbd_lower){sbd_lower = sbd;}
                if(sbd > sbd_upper){sbd_upper = sbd;}

                if(dr < dr_lower){dr_lower = dr;}
                if(dr > dr_upper){dr_upper = dr;}

                if(mbd < mbd_lower){mbd_lower = mbd;}
                if(mbd > mbd_upper){mbd_upper = mbd;}

                // counter-rotate data from all freqs. and AP's
                for(std::size_t fr = 0; fr < nchan; fr++)
                {
                    double freq = (*chan_ax)(fr);//sky freq of this channel
                    for(std::size_t ap = 0; ap < nap; ap++)
                    {
                        double tdelta = ap_ax->at(ap) + ap_delta/2.0 - midpoint_time; //need time difference from the f.r.t?
                        visibility_element_type vis = (*fSBDArray)(0,fr,ap,sbd_bin);
                        std::complex<double> vr = frot.vrot(tdelta, freq, fRefFreq, dr, mbd);
                        std::complex<double> x = vis * vr;// vrot_mod(tdelta, dr, mbd, freq, fRefFreq);
                        x *= (*fWeights)(0,fr,ap,0); //multiply by the 'weight'
                        z = z + x;
                    }
                }

                z = z * 1.0 / (double) total_ap_frac;
                drf[isbd][imbd][idr] = std::abs(z);
                printf ("drf[%ld][%ld][%ld] %lf \n", isbd, imbd, idr, drf[isbd][imbd][idr]);
            }
        }
    }



    xlim[0][0] = -1;// sbd_lower;// / status.sbd_sep - status.max_delchan + nl;
    xlim[0][1] = 1;//sbd_upper;// / status.sbd_sep - status.max_delchan + nl;

    xlim[1][0] = -2;//mbd_lower;// - status.mbd_max_global) / status.mbd_sep;
    xlim[1][1] = 2;//mbd_upper;// - status.mbd_max_global) / status.mbd_sep;

    xlim[2][0] = -2;//dr_lower;// - status.dr_max_global) / status.rate_sep;
    xlim[2][1] = 2;//dr_upper;// - status.dr_max_global) / status.rate_sep;

    std::cout<< "xlim's "<< xlim[0][0]<<", "<< xlim[0][1] <<", "<< xlim[1][0] <<", "<< xlim[1][1] <<", " << xlim[2][0] <<", "<< xlim[2][1] <<std::endl;
                                // find maximum value within cube via interpolation
    max555(drf, xlim, xi, &drfmax);

    std::cout<< "xi's "<< xi[0]<<", "<< xi[1] <<", "<< xi[2] <<std::endl;
    std::cout<<"drf max = "<<drfmax<<std::endl;



    // calculate location of this tabular point (should modulo % axis size)
    // std::size_t sbd_bin = loc[3];
    // std::size_t dr_bin = loc[2];
    //std::size_t
    sbd_bin = fSBDMaxBin;
    dr_bin = fDRMaxBin;
    mbd_bin = fMBDMaxBin;

    sbd = sbd_ax->at(sbd_bin);// + 0.5*sbd_delta;
    dr =  (fDRAxis->at(dr_bin) )*(1.0/fRefFreq);
    mbd = (fMBDAxis->at(mbd_bin));

    double sbd_change = xi[0] * sbd_delta;
    double mbd_change = xi[1] * 0.5 * mbd_delta;
    double dr_change =  (xi[2] * 0.5 * dr_delta)/fRefFreq;

    double sbd_max = (sbd + sbd_change);
    double mbd_max_global = mbd + mbd_change;
    double dr_max_global  = dr + dr_change;

    std::cout<< std::setprecision(15);
    std::cout<<"coarse location (sbd, mbd, dr) = "<<sbd<<", "<<mbd<<", "<<dr<<std::endl;
    std::cout<<"change (sbd, mbd, dr) = "<<sbd_change<<", "<<mbd_change<<", "<<dr_change<<std::endl;
    std::cout<<"Peak max555, sbd "<<sbd_max<<" mbd "<<mbd_max_global<<" dr "<<dr_max_global<<std::endl;
}





}