#include "MHO_ApMeanIntegrator.hh"
#include "MHO_MathUtilities.hh"
#include "MHO_Message.hh"

namespace hops
{

MHO_ApMeanIntegrator::MHO_ApMeanIntegrator()
    : nxy_(0)
    , begin_(0.0)
    , end_(0.0)
{
}

MHO_ApMeanIntegrator::~MHO_ApMeanIntegrator()
{
}

int MHO_ApMeanIntegrator::Initialize(int n, const double* coords, const double* val1, const double* val2)
{
    if(n <= 0)
    {
        msg_error("math", "ap_mean: n <= 0 in Initialize()" << eom);
        return -1;
    }

    // Allocate 1-based arrays with guard points at 0 and n+1.
    //TODO...if this is a performance concern we could cache these as members
    x_.resize(n + 2);
    y1_.resize(n + 2);
    y2_.resize(n + 2);

    for(int i = 0; i < n; i++)
    {
        x_[i + 1] = coords[i];
        y1_[i + 1] = val1[i];
        y2_[i + 1] = val2[i];
    }

    if(n > 1)
    {
        // Extrapolate linear guard points beyond first and last.
        x_[0] = 2.0 * x_[1] - x_[2];
        y1_[0] = 2.0 * y1_[1] - y1_[2];
        y2_[0] = 2.0 * y2_[1] - y2_[2];
        x_[n + 1] = 2.0 * x_[n] - x_[n - 1];
        y1_[n + 1] = 2.0 * y1_[n] - y1_[n - 1];
        y2_[n + 1] = 2.0 * y2_[n] - y2_[n - 1];
    }
    else
    {
        // Single point: constant extrapolation on both sides.
        x_[0] = x_[1] - 10.0;
        y1_[0] = y1_[1];
        y2_[0] = y2_[1];
        x_[2] = x_[1] + 10.0;
        y1_[2] = y1_[1];
        y2_[2] = y2_[1];
    }

    nxy_ = n + 2;

    // Valid range: midpoint between guard and first/last real point.
    begin_ = (x_[0] + x_[1]) / 2.0;
    end_ = (x_[n] + x_[n + 1]) / 2.0;

    return 0;
}

int MHO_ApMeanIntegrator::Integrate(double start, double stop, int* nstart, double* result1, double* result2)
{
    int i, fst, np, ret;

    double apsize = stop - start;
    if(apsize <= 0.0)
    {
        msg_error("math", "input error in ap_mean() (apsize <= 0)" << eom);
        return -1;
    }

    // Out of range: return zeros.
    if(begin_ > stop || end_ < start)
    {
        msg_debug("math", "out of range in ap_mean(), segment (" << start << ", " << stop
                                                                 << ") not fully in: (" << begin_ << ", " << end_ << ")"
                                                                 << eom);
        *result1 = 0.0;
        *result2 = 0.0;
        return 1;
    }

    // Local per-call buffers for the trapezoidal rule.  At most one point
    // per tabular entry can contribute to a single interval (start point +
    // interior points + stop point), so the largest index ever written is
    // nxy_; size to nxy_ + 1.  No fixed upper limit on points-per-AP.
    std::vector< double > apcoord(nxy_ + 1);
    std::vector< double > apval1(nxy_ + 1);
    std::vector< double > apval2(nxy_ + 1);
    double val;

    // Find the first tabular point >= start.
    for(fst = *nstart; fst < nxy_; fst++)
    {
        if(x_[fst] >= start)
        {
            break;
        }
    }

    apcoord[0] = 0.0;
    if(fst == 0)
    {
        // Start is before the first coordinate.
        apval1[0] = y1_[0];
        apval2[0] = y2_[0];
    }
    else
    {
        // Interpolate at the precise start coordinate.
        ret = MHO_MathUtilities::linterp(x_[fst - 1], y1_[fst - 1], x_[fst], y1_[fst], start, &val);
        if(ret != 0)
        {
            msg_error("math", "interpolation error in ap_mean() (val1 at start)" << eom);
            return -1;
        }
        apval1[0] = val;

        ret = MHO_MathUtilities::linterp(x_[fst - 1], y2_[fst - 1], x_[fst], y2_[fst], start, &val);
        if(ret != 0)
        {
            msg_error("math", "interpolation error in ap_mean() (val2 at start)" << eom);
            return -1;
        }
        apval2[0] = val;
    }

    // Collect interior points within (start, stop).
    np = 1;
    for(i = fst + 1; i < nxy_; i++)
    {
        if(x_[i] <= x_[i - 1])
        {
            msg_error("math", "mis-ordered or redundant coords: " << x_[i - 1] << ", " << x_[i] << eom);
            return -1;
        }
        if(x_[i] < stop)
        {
            apcoord[np] = (x_[i] - start) / apsize;
            apval1[np] = y1_[i];
            apval2[np] = y2_[i];
            np++;
        }
        else
        {
            break;
        }
    }

    *nstart = i - 1; // save for next call

    // Last point (interpolated at stop, or extrapolated).
    apcoord[np] = 1.0;
    if(i == nxy_)
    {
        // Stop is after the last coordinate.
        int n = nxy_ - 2; // original n
        apval1[np] = y1_[n - 1];
        apval2[np] = y2_[n - 1];
    }
    else
    {
        // Find the bracket around stop.
        for(i = fst; i < nxy_; i++)
        {
            if(x_[i] > stop)
            {
                break;
            }
        }

        ret = MHO_MathUtilities::linterp(x_[i - 1], y1_[i - 1], x_[i], y1_[i], stop, &val);
        if(ret != 0)
        {
            msg_error("math", "interpolation error in ap_mean() (val1 at stop)" << eom);
            return -1;
        }
        apval1[np] = val;

        ret = MHO_MathUtilities::linterp(x_[i - 1], y2_[i - 1], x_[i], y2_[i], stop, &val);
        if(ret != 0)
        {
            msg_error("math", "interpolation error in ap_mean() (val2 at stop)" << eom);
            return -1;
        }
        apval2[np] = val;
    }
    np++;

    // Trapezoidal integration (already normalized by apsize via apcoord).
    *result1 = 0.0;
    *result2 = 0.0;
    for(i = 0; i < np - 1; i++)
    {
        *result1 += 0.5 * (apval1[i] + apval1[i + 1]) * (apcoord[i + 1] - apcoord[i]);
        *result2 += 0.5 * (apval2[i] + apval2[i + 1]) * (apcoord[i + 1] - apcoord[i]);
    }

    return 0;
}

} // namespace hops
