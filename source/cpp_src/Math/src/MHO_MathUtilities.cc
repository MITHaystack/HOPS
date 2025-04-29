#include "MHO_MathUtilities.hh"
#include "MHO_Constants.hh"
#include "MHO_Message.hh"

//TODO FIXME eliminate these in favor of dynamic allocation (ap_mean function)
#define MAXSTATAP 100
#define MAXSTATPER 3600

namespace hops
{

double MHO_MathUtilities::dwin(double value, double lower, double upper)
{
    if(value < lower)
        return (lower);
    else if(value > upper)
        return (upper);
    else
        return (value);
}

int MHO_MathUtilities::parabola(double y[3], double lower, double upper, double* x_max, double* amp_max, double q[3])
{
    int i, rc;
    double x, range;
    //extern double dwin(double, double, double);
    range = std::fabs(upper - lower);

    q[0] = (y[0] - 2 * y[1] + y[2]) / 2; /* This is trivial to derive,
                               or see rjc's 94.1.10 derivation */
    q[1] = (y[2] - y[0]) / 2;
    q[2] = y[1];

    if(q[0] < 0.0)
        x = -q[1] / (2 * q[0]); /* x value at maximum y */
    else                        /* no max, pick higher side */
        x = (y[2] > y[0]) ? 1.0 : -1.0;

    *x_max = dwin(x, lower, upper);

    *amp_max = q[0] * *x_max * *x_max + q[1] * *x_max + q[2];

    // Test for error conditions

    rc = 0;       // default: indicates error-free interpolation
    if(q[0] >= 0) // 0 or positive curvature is an interpolation error
        rc = 2;
    // Is maximum at either edge?
    // (simple floating point equality test can fail
    // in machine-dependent way)
    else if(std::fabs(*x_max - x) > (0.001 * range))
        rc = 1;

    return (rc);
}

int MHO_MathUtilities::minvert3(double a[3][3], double ainv[3][3])
{
    // minvert () is passed an n x n matrix a
    // and returns its inverse in ainv
    // rc is non-zero if matrix a is singular
    // initial code (adapted from internet version)  2019.9.6  rjc

    int i, j, k, rc = 0;

    double x[3][2 * 3], ratio, c;

    for(i = 0; i < 3; i++) // copy a xrix into work area
        for(j = 0; j < 2 * 3; j++)
            if(j < 3)
                x[i][j] = a[i][j];
            else
                x[i][j] = 0;

    for(i = 0; i < 3; i++)
        for(j = 3; j < 2 * 3; j++)
            if(i == (j - 3))
                x[i][j] = 1.0;
            else
                x[i][j] = 0.0;

    for(i = 0; i < 3; i++)
        for(j = 0; j < 3; j++)
            if(i != j)
            {
                ratio = x[j][i] / x[i][i];
                for(k = 0; k < 2 * 3; k++)
                    x[j][k] -= ratio * x[i][k];
            }

    for(i = 0; i < 3; i++)
    {
        c = x[i][i];
        for(j = 3; j < 2 * 3; j++)
        {
            x[i][j] /= c;
            rc |= std::isnan(x[i][j]);
        }
    }

    for(i = 0; i < 3; i++) // copy inverse into ainv
        for(j = 0; j < 3; j++)
            ainv[i][j] = x[i][j + 3];

    return rc;
}


int 
MHO_MathUtilities::linterp (double coord1, double value1, double coord2, double value2, double coord, double *value)
{
    double cdiff, lweight, uweight, lower, upper, lval, uval;
    // Condition inputs 
    if (coord1 < coord2)
    {
        lower = coord1;
        lval = value1;
        upper = coord2;
        uval = value2;
    }
    else if (coord2 < coord1)
    {
        lower = coord2;
        lval = value2;
        upper = coord1;
        uval = value1;
    }
    else if (coord != coord1) // Trap case of upper=lower
    {
        msg_error("math", "degenerate inputs to linterp()" << eom);
        msg_error("math", "(coord, lower, upper) = ("<< coord <<", " << lower <<", "<< upper << ")" << eom);
        return -1;
    }
    else
    {
        *value = value1;
        return 0;
    }
    //Range check
    if ((coord < lower) || (coord > upper))
    {
        msg_error("math", "out of range inputs to linterp()" << eom);
        msg_error("math", "(coord, lower, upper) = ("<< coord <<", " << lower <<", "<< upper << ")" << eom);
        return -1;
    }

    // Simple linear interpolation
    cdiff = upper - lower;
    lweight = (upper - coord) / cdiff;
    uweight = (coord - lower) / cdiff;
    *value = lweight * lval + uweight * uval;

    return 0;
}



int 
MHO_MathUtilities::ap_mean(double start, double stop, 
                           double *coords, double *val1, double *val2, 
                           int n, int *nstart, double *result1, double *result2)
{
    int i, fst, np, ret;
    static int nxy;

    double apcoord[MAXSTATAP], apval1[MAXSTATAP], apval2[MAXSTATAP], val;
    static double apsize, begin, end, x[MAXSTATPER+2], y1[MAXSTATPER+2], y2[MAXSTATPER+2];

    apsize = stop - start;
    if (apsize <= 0.0)
    {
        msg_error("math", "input error in ap_mean()" << eom);
        return -1;
    }

    if (*nstart == 0)
    {
        /* extend data array beyond full interval
        * of first and last tabular points */
        for (i=0; i<n; i++)
        {
            x[i+1] = coords[i];
            y1[i+1] = val1[i];
            y2[i+1] = val2[i];
        }

        if (n > 1)
        {
            x[0] = 2 * x[1] - x[2];
            y1[0] = 2 * y1[1] - y1[2];
            y2[0] = 2 * y2[1] - y2[2];
            x[n+1] = 2 * x[n] - x[n-1];
            y1[n+1] = 2 * y1[n] - y1[n-1];
            y2[n+1] = 2 * y2[n] - y2[n-1];
        }
        else // if only one PC point, extrapolate constant value.  rjc 2002.11.14 
        {
            x[0] = x[1] - 10.0;
            y1[0] = y1[1];
            y2[0] = y2[1];
            x[2] = x[1] + 10.0;
            y1[2] = y1[1];
            y2[2] = y2[1];
        }
        nxy = n + 2;
        /* phase cal data really represents
        * interval from midway between the
        * two endpoints */
        begin = (x[0] + x[1]) / 2;
        end = (x[n] + x[n+1]) / 2;
    }

    /* Hopelessly out of range */
    /* Set to zero to indicate missing data */
    if (begin > stop || end < start)
    {
        msg_warn("math", "out of range in ap_mean(), "<<start<<", "<<stop<<", "<<begin<<", "<<end<< eom);
        *result1 = 0.0;
        *result2 = 0.0;
        return 1;
    }

    /* Find and set up first point */
    for(fst=*nstart; fst<nxy; fst++)
    {
        if (x[fst] >= start) {break;}
    }

    apcoord[0] = 0.0;
    /* Start is before first coord, so just */
    /* use value of first array element */
    if (fst == 0) 
    {
        apval1[0] = y1[0];
        apval2[0] = y2[0];
    }
    else
    {
        /* Linearly interpolate to find value */
        /* at precise start coordinate */
        ret = linterp (x[fst-1], y1[fst-1], x[fst], y1[fst], start, &val);
        if (ret == 0){apval1[0] = val;}
        else
        {
            msg_error("math", "interpolation error in ap_mean()" << eom );
            return (-1);
        }
        ret = linterp (x[fst-1], y2[fst-1], x[fst], y2[fst], start, &val);
        apval2[0] = val;
    }

    /* Get the points contained within */
    /* the start-stop interval */
    np = 1;
    for (i=fst+1; i<nxy; i++)
    {
        /* Coords must increase monotonically */
        if (x[i] <= x[i-1])
        {
            msg_error("math", "mis-ordered or redundant coords: "<< x[i-1] <<", "<< x[i] << eom);
            return -1;
        }
        /* Coords and values copied directly */
        if (x[i] < stop)
        {
            if (np >= MAXSTATAP)
            {
                msg_error("math", "too many points per AP in ap_mean()" << eom);
                return -1;
            }
            apcoord[np] = (x[i] - start) / apsize;
            apval1[np] = y1[i];
            apval2[np] = y2[i];
            np++;
        }
        else 
        break;
    }

    *nstart = i - 1;                    // save starting point for next call
    /* Get the last point */
    apcoord[np] = 1.0;
    /* Stop is after last coord, so just */
    /* use value of last array element */
    if (i == nxy) 
    {
        apval1[np] = y1[n-1];
        apval2[np] = y2[n-1];
    }
    else
    {
        /* Linearly interpolate to find value */
        /* at precise stop coordinate */
        for (i=fst; i<nxy; i++)
        {
            if (x[i] > stop){ break; }
        }

        ret = linterp (x[i-1], y1[i-1], x[i], y1[i], stop, &val);
        if(ret == 0)
        {
            apval1[np] = val;
        }
        else
        {
            msg_error("math", "interpolation error in ap_mean()" << eom);
            return -1;
        }
        ret = linterp (x[i-1], y2[i-1], x[i], y2[i], stop, &val);
        apval2[np] = val;
    }
    np++;
    /* Perform the integration, pre-normalized */
    *result1 = 0.0;
    *result2 = 0.0;

    for (i=0; i<np-1; i++)
    {
        *result1 += 0.5 * (apval1[i] + apval1[i+1]) * (apcoord[i+1] - apcoord[i]);
        *result2 += 0.5 * (apval2[i] + apval2[i+1]) * (apcoord[i+1] - apcoord[i]);
    }
    return 0;



}



double MHO_MathUtilities::average(std::vector< double >& vec)
{
    std::size_t s = vec.size();
    if(s == 0)
    {
        return 0.0;
    }
    double ave = 0;
    for(std::size_t i = 0; i < s; i++)
    {
        ave += vec[i];
    }
    ave /= (double)s;
    return ave;
}

double MHO_MathUtilities::angular_average(std::vector< double >& vec)
{
    std::size_t s = vec.size();
    if(s == 0)
    {
        return 0.0;
    }
    std::complex< double > ave = 0;
    std::complex< double > imagUnit = MHO_Constants::imag_unit;
    for(std::size_t i = 0; i < s; i++)
    {
        std::complex< double > phasor = std::exp(1.0 * imagUnit * (vec[i]));
        ave += phasor;
    }
    ave /= (double)s;
    return std::arg(ave);
}

} // namespace hops
