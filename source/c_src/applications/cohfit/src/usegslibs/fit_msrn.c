/*
 * Fit the SNR versus segmentation time to fine the max SNR.
 *
 * fit_snr() does a 3pt parabolic interpolation about the max SNR
 *  but with noise, it can easily find a bump.
 * fit_cbs() does a cubic spline fit to the entire shape allowing
 *  a more likely SNR maximum.
 *
 * Note that both use SNRs as renormalized in normalize_snr()
 * return an integer peak-SNR time and populate fitsnr[].
 *
 * Created GBC May 1 2026.
 */
#include "fit_gsl.h"

int fit_msnr(cosumary *codatum, int npt)
{
    double pk3pt, pkcbs;
    int    ik3pt, ikcbs, isnrmax;

    if (codatum->fitmask & FITOPT_SNR_3PT) pk3pt = fit_snr(codatum, npt);
    else pk3pt = -3.0;
    ik3pt = round(pk3pt);
    msg("fit_snr() = %lf -> %d", 1, pk3pt, ik3pt);
    if (codatum->fitmask & FITOPT_SNR_CBS) pkcbs = fit_cbs(codatum, npt);
    else pkcbs = -3.0;
    ikcbs = round(pkcbs);
    msg("fit_cbs() = %lf -> %d", 1, pkcbs, ikcbs);

    /* it seems to me that the cbs should always be better */
    if (pkcbs > 0) {
        codatum->snr_cotime = pkcbs;
        isnrmax = ikcbs;
        msg("Using CBS result for the position of SNR maximum", 2);
    } else {
        codatum->snr_cotime = pk3pt;
        isnrmax = ik3pt;
        msg("Using 3PT result for the position of SNR maximum", 2);
    }

    return(round(codatum->snr_cotime));
}

/*
 * eof vim:nospell
 */
