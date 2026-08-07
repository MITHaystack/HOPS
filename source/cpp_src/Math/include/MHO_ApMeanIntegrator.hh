#ifndef MHO_ApMeanIntegrator_HH__
#define MHO_ApMeanIntegrator_HH__

#include <vector>

namespace hops
{

/**
 * @brief Trapezoidal integrator over tabular data with linear interpolation.
 *
 * Replaces the static-buffer ap_mean() function with a re-entrant object.
 *  The caller initializes the integrator once with a set of (coord,
 * val1, val2) points, then queries integrated averages over successive
 * [start, stop] intervals.  The optional @p nstart pointer is an optimizer
 * hint so that the search for the first relevant tabular point does not
 * restart from the beginning on each call.
 *
 * Thread-safe by design: each instance owns its own buffers.
 */
class MHO_ApMeanIntegrator
{
    public:
        MHO_ApMeanIntegrator();
        ~MHO_ApMeanIntegrator();

        /**
         * @brief Load tabular data.  Must be called before Integrate().
         *
         * @param n Number of data points (must be > 0).
         * @param coords Monotonically increasing coordinate array of size @p n.
         * @param val1 First value array (e.g. real part) of size @p n.
         * @param val2 Second value array (e.g. imaginary part) of size @p n.
         * @return 0 on success, -1 if n <= 0.
         */
        int Initialize(int n, const double* coords, const double* val1, const double* val2);

        /**
         * @brief Compute the average of val1 and val2 over [start, stop].
         *
         * Uses trapezoidal integration with linear interpolation at the
         * interval boundaries.  Guard points are extrapolated beyond the
         * first and last tabular entries so that edge intervals are handled
         * gracefully.
         *
         * @param start Interval start coordinate.
         * @param stop Interval stop coordinate (must be > start).
         * @param nstart In/out optimizer index.  Set to 0 for the first call
         *   after Initialize(); the method updates it for subsequent calls.
         * @param result1 Output average of val1.
         * @param result2 Output average of val2.
         * @return 0 on success, 1 if the interval is out of range (results
         *   set to 0), -1 on interpolation error.
         */
        int Integrate(double start, double stop, int* nstart, double* result1, double* result2);

    private:
        // 1-based tabular arrays with guard points at index 0 and n+1.
        std::vector< double > x_;
        std::vector< double > y1_;
        std::vector< double > y2_;
        int nxy_;      // effective number of entries (n + 2)
        double begin_; // half-open begin of valid range
        double end_;   // half-open end of valid range
};

} // namespace hops

#endif /*! end of include guard: MHO_ApMeanIntegrator_HH__ */
