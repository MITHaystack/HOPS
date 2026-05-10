/*
 * Convert the bit mask flag into an FITOPT_NDX index and name
 */
#include "cohfit.h"

char *as_fit_nm_ndx(int flag, int *index)
{
    char *fitnam;
    switch(flag) {
    case FITOPT_AMP_PS:
        *index = FITOPT_NDX_PS;
        fitnam = "plateau-slope fit for amplitude";
        break;
    case FITOPT_AMP_PO:
        *index = FITOPT_NDX_PO;
        fitnam = "plateau-only fit for amplitude";
        break;
    case FITOPT_AMP_SO:
        *index = FITOPT_NDX_SO;
        fitnam = "slope-only fit for amplitude";
        break;
    case FITOPT_SNR_3PT:
        *index = FITOPT_NDX_3PT;
        fitnam = "three-point fit for SNR";
        break;
    case FITOPT_SNR_CBS:
        *index = FITOPT_NDX_CBS;
        fitnam = "cubic spline fit for SNR";
        break;
    default:
        *index = NFITOPT;
        fitnam = "DEVELOPER ERROR";
        break;
    }
    return(fitnam);
}

/*
 * eof vim:nospell
 */
