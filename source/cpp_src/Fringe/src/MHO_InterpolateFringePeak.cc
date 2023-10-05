#include "MHO_InterpolateFringePeak.hh"
#include "MHO_FringeRotation.hh"

namespace hops
{

MHO_InterpolateFringePeak::MHO_InterpolateFringePeak()
{
    fMBDMaxBin = 0;
    fDRMaxBin = 0;
    fSBDMaxBin = 0;

    fRefFreq = 1.0;
    fTotalSummedWeights = 1.0;
    fSBDArray = nullptr;
    fWeights = nullptr;

    fMBDAxis.Resize(1);
    fDRAxis.Resize(1);

    fDRF.Resize(5,5,5);
}

bool
MHO_InterpolateFringePeak::Initialize()
{
    if(fSBDArray == nullptr){return false;}
    if(fWeights == nullptr){return false;}
    if(fMBDAxis.GetSize() == 1){return false;}
    if(fDRAxis.GetSize() == 1){return false;}
    bool ok = fWeights->Retrieve("total_summed_weights", fTotalSummedWeights);
    if(!ok)
    {
        msg_warn("fringe", "missing 'total_summed_weights' tag in weights object." << eom);
        return false;
    }

    return true;
};

bool
MHO_InterpolateFringePeak::Execute()
{
    fine_peak_interpolation();
    return true;
}

void
MHO_InterpolateFringePeak::SetMaxBins(int sbd_max, int mbd_max, int dr_max)
{
    fSBDMaxBin = sbd_max;
    fMBDMaxBin = mbd_max;
    fDRMaxBin = dr_max;
}


void
MHO_InterpolateFringePeak::fine_peak_interpolation()
{
    //follow the algorithm of interp.c (SIMUL) mode, to fill out a cube and interpolate
    //double drf[5][5][5];// 5x5x5 cube of fringe values
    double xlim[3][2]; //cube limits each dim
    double xi[3];
    double drfmax;

    double total_ap_frac = fTotalSummedWeights;
    
    // std::cout<<"total ap frac = "<<total_ap_frac<<std::endl;

    auto chan_ax = &( std::get<CHANNEL_AXIS>(*fSBDArray) );
    auto ap_ax = &(std::get<TIME_AXIS>(*fSBDArray));
    auto sbd_ax = &( std::get<FREQ_AXIS>(*fSBDArray) );

    // std::cout<<"sbd ax ="<<sbd_ax<<std::endl;
    // std::cout<<"sbdmaxbin="<<fSBDMaxBin<<std::endl;
    // std::cout<<"fSBDArray = "<<fSBDArray<<std::endl;

    std::size_t nap = ap_ax->GetSize();
    std::size_t nchan = chan_ax->GetSize();

    double ap_delta = ap_ax->at(1) - ap_ax->at(0);
    double sbd_delta = sbd_ax->at(1) - sbd_ax->at(0);
    double dr_delta = fDRAxis.at(1) - fDRAxis.at(0);
    double mbd_delta = fMBDAxis.at(1) - fMBDAxis.at(0);

    //TODO FIXME -- shoudl this be the fourfit refrence time? Also...should this be calculated elsewhere?
    double midpoint_time = ( ap_ax->at(nap-1) + ap_delta  + ap_ax->at(0) )/2.0;
    // std::cout<<"time midpoint = "<<midpoint_time<<std::endl;
    // std::cout<<"dr delta = "<<dr_delta<<std::endl;
    // std::cout<<"ref freq = "<<fRefFreq<<std::endl;
    // printf("max bin (sbd, mbd, dr) = %d, %d, %d\n", fSBDMaxBin, fMBDMaxBin, fDRMaxBin );
    // printf("mbd delta, dr delta = %.15f, %.15f \n", mbd_delta, dr_delta/fRefFreq);

    double sbd_lower = 1e30;
    double sbd_upper = -1e30;
    double mbd_lower = 1e30;
    double mbd_upper = -1e30;
    double dr_lower = 1e30;
    double dr_upper = -1e30;


    int sbd_bin, dr_bin, mbd_bin;
    double sbd, dr, mbd;

    //std::cout<< std::setprecision(15);
    for (int isbd=0; isbd<5; isbd++)
    {
        for (int imbd=0; imbd<5; imbd++)
        {
            for(int idr=0; idr<5; idr++)
            {
                std::complex<double> z = 0.0;

                // calculate location of this tabular point (should modulo % axis size)
                sbd_bin = (fSBDMaxBin + isbd - 2) % (int) sbd_ax->GetSize();
                dr_bin = (fDRMaxBin + idr - 2 ) % (int) fDRAxis.GetSize() ;
                mbd_bin = ( fMBDMaxBin + imbd - 2) % (int) fMBDAxis.GetSize() ;;
                //
                sbd = sbd_ax->at( (std::size_t) sbd_bin);
                mbd = fMBDAxis.at(fMBDMaxBin) + 0.5 * (imbd - 2) * mbd_delta;
                dr  = (fDRAxis.at(fDRMaxBin) + (0.5 * (idr - 2)  * dr_delta) )/fRefFreq;

                //printf("idr = %d and dr = %.8f \n", idr, dr);

                if(sbd < sbd_lower){sbd_lower = sbd;}
                if(sbd > sbd_upper){sbd_upper = sbd;}

                if(dr < dr_lower){dr_lower = dr;}
                if(dr > dr_upper){dr_upper = dr;}

                if(mbd < mbd_lower){mbd_lower = mbd;}
                if(mbd > mbd_upper){mbd_upper = mbd;}
                
                // std::cout<<"fMBDMaxBin = "<<fMBDMaxBin<<" max = "<<fMBDAxis.at(fMBDMaxBin)<<std::endl;
                // std::cout<<"mbd delta = "<<mbd_delta<<std::endl;
                // std::cout<<"mbd info = "<<mbd<<", "<<imbd<<std::endl;
                
                // counter-rotate data from all freqs. and AP's
                for(std::size_t fr = 0; fr < nchan; fr++)
                {
                    double freq = (*chan_ax)(fr);//sky freq of this channel
                    for(std::size_t ap = 0; ap < nap; ap++)
                    {
                        double tdelta = ap_ax->at(ap) + ap_delta/2.0 - midpoint_time; //need time difference from the f.r.t?
                        visibility_element_type vis = (*fSBDArray)(0,fr,ap,sbd_bin);
                        
                        //std::cout<<"fr, ap ="<<fr<<", "<<ap<<" vrot input( "<<tdelta<<", "<<freq<<", "<<fRefFreq<<", "<<dr<<", "<<mbd<<")"<<std::endl;
                        std::complex<double> vr = fRot.vrot(tdelta, freq, fRefFreq, dr, mbd);
                        std::complex<double> x = vis * vr;// vrot_mod(tdelta, dr, mbd, freq, fRefFreq);
                        x *= (*fWeights)(0,fr,ap,0); //multiply by the 'weight'
                        z = z + x;
                    }
                }
                z = z * 1.0 / total_ap_frac;
                // drf[isbd][imbd][idr] = std::abs(z);
                fDRF(isbd, imbd, idr) = std::abs(z);
                // printf ("drf[%ld][%ld][%ld] %lf \n", isbd, imbd, idr, fDRF(isbd, imbd, idr) );
            }
        }
    }



    xlim[0][0] = -1;// sbd_lower;// / status.sbd_sep - status.max_delchan + nl;
    xlim[0][1] = 1;//sbd_upper;// / status.sbd_sep - status.max_delchan + nl;

    xlim[1][0] = -2;//mbd_lower;// - status.mbd_max_global) / status.mbd_sep;
    xlim[1][1] = 2;//mbd_upper;// - status.mbd_max_global) / status.mbd_sep;

    xlim[2][0] = -2;//dr_lower;// - status.dr_max_global) / status.rate_sep;
    xlim[2][1] = 2;//dr_upper;// - status.dr_max_global) / status.rate_sep;
    // 
    // std::cout<< "xlim's "<< xlim[0][0]<<", "<< xlim[0][1] <<", "<< xlim[1][0] <<", "<< xlim[1][1] <<", " << xlim[2][0] <<", "<< xlim[2][1] <<std::endl;
    //                             // find maximum value within cube via interpolation
    max555(fDRF, xlim, xi, &drfmax);

    // std::cout<< "xi's "<< xi[0]<<", "<< xi[1] <<", "<< xi[2] <<std::endl;
    // std::cout<<"drf max = "<<drfmax<<std::endl;

    // calculate location of this tabular point (should modulo % axis size)
    // std::size_t sbd_bin = loc[3];
    // std::size_t dr_bin = loc[2];
    //std::size_t
    sbd_bin = fSBDMaxBin;
    dr_bin = fDRMaxBin;
    mbd_bin = fMBDMaxBin;
    fFringeAmp = drfmax;

    // std::cout<<"----------------------------------------"<<std::endl;
    // std::cout<<"sbd ax ="<<sbd_ax<<std::endl;
    // std::cout<<"sbd_bin="<<sbd_bin<<std::endl;
    // std::cout<<"mbd_bin="<<mbd_bin<<std::endl;

    sbd = sbd_ax->at(sbd_bin);// + 0.5*sbd_delta;
    dr =  (fDRAxis.at(dr_bin) )*(1.0/fRefFreq);
    mbd = (fMBDAxis.at(mbd_bin));

    double sbd_change = xi[0] * sbd_delta;
    double mbd_change = xi[1] * 0.5 * mbd_delta;
    double dr_change =  (xi[2] * 0.5 * dr_delta)/fRefFreq;

    double sbd_max = (sbd + sbd_change);
    double mbd_max_global = mbd + mbd_change;
    double dr_max_global  = dr + dr_change;

    fSBDelay = sbd_max;
    fMBDelay = mbd_max_global;
    fDelayRate = dr_max_global;
    fFringeRate = fDRAxis.at(dr_bin) + (xi[2] * 0.5 * dr_delta);

    // std::cout<< std::setprecision(15);
    // std::cout<<"coarse location (sbd, mbd, dr) = "<<sbd<<", "<<mbd<<", "<<dr<<std::endl;
    // std::cout<<"change (sbd, mbd, dr) = "<<sbd_change<<", "<<mbd_change<<", "<<dr_change<<std::endl;
    msg_info("fringe", "Peak max555, sbd "<<sbd_max<<" mbd "<<mbd_max_global<<" dr "<<dr_max_global<< eom );

}



void MHO_InterpolateFringePeak::max555 (MHO_NDArrayWrapper<double, 3>& drf,   // input: real function
             double xlim[3][2],     // input: lower & upper bounds in 3 dimensions
             double xi[3],          // output: coordinates at maximum value
             double *drfmax)        // output: maximum value
    {
    int i,
        j,
        k,
        l;

    double dx0,
           dx1,
           dx2,
           center[3],
           x[3],
           x0_lower, x1_lower, x2_lower,
           x0_upper, x1_upper, x2_upper,
           value,
           bestval,
           xbest[3],
           epsilon = 0.0001;        // convergence criterion


                                    // initialize search to center of cube
    for (l=0; l<3; l++)
        center[l] = 0.0;
    dx0 = dx1 = dx2 = 0.4;

    do
        {
                                    // search over 11x11x11 cube for max
                                    // first compress search range to fit into bounds
        x0_lower = dwin (center[0] - 5 * dx0, xlim[0][0], xlim[0][1]);
        x1_lower = dwin (center[1] - 5 * dx1, xlim[1][0], xlim[1][1]);
        x2_lower = dwin (center[2] - 5 * dx2, xlim[2][0], xlim[2][1]);

        x0_upper = dwin (center[0] + 5 * dx0, xlim[0][0], xlim[0][1]);
        x1_upper = dwin (center[1] + 5 * dx1, xlim[1][0], xlim[1][1]);
        x2_upper = dwin (center[2] + 5 * dx2, xlim[2][0], xlim[2][1]);

        dx0 = (x0_upper - x0_lower) / 10.0;
        dx1 = (x1_upper - x1_lower) / 10.0;
        dx2 = (x2_upper - x2_lower) / 10.0;

        center[0] = (x0_lower + x0_upper) / 2.0;
        center[1] = (x1_lower + x1_upper) / 2.0;
        center[2] = (x2_lower + x2_upper) / 2.0;

        bestval = 0.0;
        for (i=0; i<11; i++)
            for (j=0; j<11; j++)
                for (k=0; k<11; k++)
                    {
                    x[0] = center[0] + dx0 * (i-5);
                    x[1] = center[1] + dx1 * (j-5);
                    x[2] = center[2] + dx2 * (k-5);
                                    // find interpolated value at this point
                    interp555 (drf, x, &value);
                                    // is this a new maximum?
                                    // if so, save value and coords.
                    if (value > bestval)
                        {
                        bestval = value;
                        for (l=0; l<3; l++)
                            xbest[l] = x[l];
                        }
                    }
                                    // relocate center and reduce grid size
        for (l=0; l<3; l++)
            center[l] = xbest[l];
        dx0 /= 5.0;
        dx1 /= 5.0;
        dx2 /= 5.0;
        }

    while (dx0 > epsilon || dx1 > epsilon || dx2 > epsilon);
                                    // return result to caller
    *drfmax = bestval;
    for (l=0; l<3; l++)
        xi[l] = xbest[l];
    }


double
MHO_InterpolateFringePeak::dwin(double value, double lower, double upper)
{
    if (value < lower) return (lower);
    else if (value > upper) return (upper);
    else return (value);
}




void
MHO_InterpolateFringePeak::interp555 (MHO_NDArrayWrapper<double, 3>& drf,// input: real function
                                      double xi[3],       // input: coordinates to be evaluated at
                                      double *drfval)     // output: interpolated value
{
    int i,
    j,
    k;

    double a[5][3],
    p,
    p2;

    for (j=0; j<3; j++)
    {
        p = xi[j];
        p2 = p * p;
        // Lagrange interpolating polynomials based
        // on Abramowitz & Stegun's 25.2.15
        a[0][j] = (p2-1) * p * (p-2) / 24;
        a[1][j] = -(p-1) * p * (p2-4) / 6;
        a[2][j] = (p2-1) * (p2-4) / 4;
        a[3][j] = -(p+1) * p * (p2-4) / 6;
        a[4][j] = (p2-1) * p * (p+2) / 24;
    }

    *drfval = 0.0;

    for (i=0; i<5; i++)
        for (j=0; j<5; j++)
            for (k=0; k<5; k++)
            {
                *drfval += a[i][0] * a[j][1] * a[k][2] * drf(i,j,k);
            }
}



}
