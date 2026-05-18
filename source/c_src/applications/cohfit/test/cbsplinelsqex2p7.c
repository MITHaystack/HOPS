/*
 * GSL 2.7-API variant of cbsplinelsqex.c
 *
 * Equivalent to the example at
 *   https://www.gnu.org/software/gsl/doc/html/bspline.html
 * but written against the older bspline interface that lacks
 * gsl_bspline_ncontrol / gsl_bspline_init_uniform / gsl_bspline_wlssolve
 * / gsl_bspline_calc.  The fit is set up explicitly as a weighted linear
 * least-squares problem using gsl_multifit_wlinear, with the bspline basis
 * evaluated at each x_i via gsl_bspline_eval.
 *
 * The stderr "breakpoints:" output matches the 2.8 variant in format so
 * cbsplinecheck.sh can grep + cmp against a stored reference file.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <gsl/gsl_math.h>
#include <gsl/gsl_bspline.h>
#include <gsl/gsl_multifit.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_statistics.h>

static double
fit_one(const size_t nbreak, const gsl_vector *x, const gsl_vector *y,
        const gsl_vector *w, size_t n, double *chisq_out)
{
    const size_t k = 4;
    gsl_bspline_workspace *work = gsl_bspline_alloc(k, nbreak);
    const size_t p = gsl_bspline_ncoeffs(work);
    gsl_vector *c   = gsl_vector_alloc(p);
    gsl_vector *B   = gsl_vector_alloc(p);
    gsl_matrix *X   = gsl_matrix_alloc(n, p);
    gsl_matrix *cov = gsl_matrix_alloc(p, p);
    gsl_multifit_linear_workspace *mw = gsl_multifit_linear_alloc(n, p);
    double chisq;
    size_t i, j;

    gsl_bspline_knots_uniform(gsl_vector_get(x, 0),
                              gsl_vector_get(x, n-1), work);

    for (i = 0; i < n; ++i) {
        gsl_bspline_eval(gsl_vector_get(x, i), B, work);
        for (j = 0; j < p; ++j)
            gsl_matrix_set(X, i, j, gsl_vector_get(B, j));
    }

    gsl_multifit_wlinear(X, w, y, c, cov, &chisq, mw);
    *chisq_out = chisq;

    gsl_bspline_free(work);
    gsl_vector_free(c);
    gsl_vector_free(B);
    gsl_matrix_free(X);
    gsl_matrix_free(cov);
    gsl_multifit_linear_free(mw);
    return (double)(n - p);
}

int
main(void)
{
    const size_t n = 500;
    const double a = 0.0, b = 15.0;
    const double sigma = 0.2;
    gsl_vector *x = gsl_vector_alloc(n);
    gsl_vector *y = gsl_vector_alloc(n);
    gsl_vector *w = gsl_vector_alloc(n);
    gsl_rng *r;
    double chisq1, chisq2, dof1, dof2;
    size_t i;

    gsl_rng_env_setup();
    r = gsl_rng_alloc(gsl_rng_default);

    for (i = 0; i < n; ++i) {
        double xi = (b - a) / (n - 1.0) * i + a;
        double yi = cos(xi) * exp(-0.1 * xi);
        yi += gsl_ran_gaussian(r, sigma);
        gsl_vector_set(x, i, xi);
        gsl_vector_set(y, i, yi);
        gsl_vector_set(w, i, 1.0 / (sigma * sigma));
        printf("data: %f %f\n", xi, yi);
    }
    fflush(stdout);

    dof1 = fit_one(40, x, y, w, n, &chisq1);
    dof2 = fit_one(10, x, y, w, n, &chisq2);

    fprintf(stderr, "40 breakpoints: chisq/dof = %e\n", chisq1 / dof1);
    fprintf(stderr, "10 breakpoints: chisq/dof = %e\n", chisq2 / dof2);

    printf("\n\n");

    /* re-fit so we can plot the curves (cheap) */
    {
        const size_t k = 4;
        gsl_bspline_workspace *w1 = gsl_bspline_alloc(k, 40);
        gsl_bspline_workspace *w2 = gsl_bspline_alloc(k, 10);
        const size_t p1 = gsl_bspline_ncoeffs(w1);
        const size_t p2 = gsl_bspline_ncoeffs(w2);
        gsl_vector *c1 = gsl_vector_alloc(p1);
        gsl_vector *c2 = gsl_vector_alloc(p2);
        gsl_vector *B1 = gsl_vector_alloc(p1);
        gsl_vector *B2 = gsl_vector_alloc(p2);
        gsl_matrix *X1 = gsl_matrix_alloc(n, p1);
        gsl_matrix *X2 = gsl_matrix_alloc(n, p2);
        gsl_matrix *cv1 = gsl_matrix_alloc(p1, p1);
        gsl_matrix *cv2 = gsl_matrix_alloc(p2, p2);
        gsl_multifit_linear_workspace *mw1 = gsl_multifit_linear_alloc(n, p1);
        gsl_multifit_linear_workspace *mw2 = gsl_multifit_linear_alloc(n, p2);
        double chisq, xi, r1, r2, yerr;
        size_t j;

        gsl_bspline_knots_uniform(a, b, w1);
        gsl_bspline_knots_uniform(a, b, w2);
        for (i = 0; i < n; ++i) {
            gsl_bspline_eval(gsl_vector_get(x, i), B1, w1);
            gsl_bspline_eval(gsl_vector_get(x, i), B2, w2);
            for (j = 0; j < p1; ++j) gsl_matrix_set(X1, i, j, gsl_vector_get(B1, j));
            for (j = 0; j < p2; ++j) gsl_matrix_set(X2, i, j, gsl_vector_get(B2, j));
        }
        gsl_multifit_wlinear(X1, w, y, c1, cv1, &chisq, mw1);
        gsl_multifit_wlinear(X2, w, y, c2, cv2, &chisq, mw2);

        for (xi = a; xi <= b; xi += 0.1) {
            gsl_bspline_eval(xi, B1, w1);
            gsl_multifit_linear_est(B1, c1, cv1, &r1, &yerr);
            gsl_bspline_eval(xi, B2, w2);
            gsl_multifit_linear_est(B2, c2, cv2, &r2, &yerr);
            printf("spline: %f %f %f\n", xi, r1, r2);
        }
        fflush(stdout);

        gsl_bspline_free(w1); gsl_bspline_free(w2);
        gsl_vector_free(c1); gsl_vector_free(c2);
        gsl_vector_free(B1); gsl_vector_free(B2);
        gsl_matrix_free(X1); gsl_matrix_free(X2);
        gsl_matrix_free(cv1); gsl_matrix_free(cv2);
        gsl_multifit_linear_free(mw1);
        gsl_multifit_linear_free(mw2);
    }

    gsl_rng_free(r);
    gsl_vector_free(x);
    gsl_vector_free(y);
    gsl_vector_free(w);
    return 0;
}
