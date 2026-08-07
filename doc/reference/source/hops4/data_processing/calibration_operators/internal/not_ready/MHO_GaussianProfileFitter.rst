MHO\_GaussianProfileFitter
==========================

Purpose
-------
``MHO_GaussianProfileFitter`` fits a 1-D Gaussian profile with a constant baseline to amplitude data using the Levenberg--Marquardt non-linear least-squares algorithm (via Eigen's ``NonLinearOptimization`` module). The fitted parameters are the Gaussian amplitude, mean, standard deviation, and baseline offset.

Control File Trigger
--------------------
This class is an internal utility with no control file keyword. It is used by other calibration operators to characterise the shape of spectral or delay profiles.

Input Data
----------
This class accepts raw coordinate data as two ``std::vector<double>`` arrays --- an independent variable vector $x$ and a dependent variable vector $y$. The caller supplies the data and an initial parameter guess via ``SetData`` and ``SetInitialGuess``.

Algorithm
---------
The model is

.. math::

   f(x) = A \, \exp\!\left(-\frac{(x-\mu)^2}{2\sigma^2}\right) + B

where $A$ is the amplitude, $\mu$ is the mean, $\sigma$ is the standard deviation, and $B$ is the constant baseline.

**Setup:**

1. Call ``SetData(x, y)`` to provide the $x$ and $y$ vectors.
2. Call ``SetInitialGuess(amp, mu, sigma, baseline)`` to provide starting values $[A_0, \mu_0, \sigma_0, B_0]$. Default guess is (1, 0, 1, 0).
3. Optionally call ``SetMaxChi2`` to set the reduced-$\chi^2$ threshold (default 10).

**Fit (``Fit``):**

1. Validate that at least 4 data points are present and that the initial $\sigma_0$ is non-zero.
2. Construct a ``GaussianFunctor`` which computes residuals:

   .. math::

      r_i = y_i - \left( A \, \exp\!\left(-\frac{(x_i-\mu)^2}{2\sigma^2}\right) + B \right)

   and provides the analytic Jacobian with respect to $[A, \mu, \sigma, B]$:

   .. math::

      \frac{\partial r_i}{\partial A} &= -g_i \\
      \frac{\partial r_i}{\partial \mu} &= -A \, g_i \, \frac{(x_i-\mu)}{\sigma^2} \\
      \frac{\partial r_i}{\partial \sigma} &= -A \, g_i \, \frac{(x_i-\mu)^2}{\sigma^3} \\
      \frac{\partial r_i}{\partial B} &= -1

   where $g_i = \exp\!\bigl(-(x_i-\mu)^2/(2\sigma^2)\bigr)$.

3. Run Eigen's ``LevenbergMarquardt`` minimizer with up to 2000 function evaluations and tolerances $10^{-10}$ for both $\chi^2$ and parameter change.
4. After convergence, compute the reduced chi-squared:

   .. math::

      \chi^2_\nu = \frac{\sum_i r_i^2}{N - 4}

   where $N$ is the number of data points and 4 is the number of fitted parameters.
5. Validate the fit by requiring:

   - The optimizer reports convergence (relative reduction, relative error, or cosine criterion).
   - Fitted $\sigma > 0$ and $\sigma$ is narrower than the $x$-data span.
   - Reduced $\chi^2_\nu \leq$ the configured maximum (default 10).

The full-width at half-maximum is computed from the fitted $\sigma$:

.. math::

   \mathrm{FWHM} = 2\sqrt{2\ln 2}\;|\sigma| \approx 2.355\,|\sigma|.

Effect on Data
--------------
This class is read-only with respect to the input data. It produces fitted parameters (amplitude, mean, sigma, FWHM, baseline, and reduced chi-squared) accessible via getter methods. The ``Evaluate(x)`` method allows the caller to sample the fitted Gaussian at any $x$.
