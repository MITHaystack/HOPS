//
// Wrapper for hops msg() function to use Messenger
// (c) Massachusetts Institute of Technology, 2020
// The contents of the package Copyright statement apply here.
//
// import numpy as np
// vis = np.ones(16, dtype=np.complex128)
//
// FIXME: there is probably some more clever way to launder
//        a npy_complex into a std::complex, but this works.
//

extern "C" {
#include "numpy_test.h"
#include <complex.h>
}

extern "C" void get_np_amps(
    npy_cdouble *vis, int nvis, npy_double *amp, int namp)
{
    for (int ii = 0; ii < nvis && ii < namp; ii++) {
        amp[ii] = cabs((vis[ii].real + I * vis[ii].imag));
    }
}

extern "C" void get_np_phases(
    npy_cdouble *vis, int nvis, npy_double *phs, int nphs)
{
    std::complex<double> temp;
    for (int ii = 0; ii < nvis && ii < nphs; ii++) {
        phs[ii] = carg((vis[ii].real + I * vis[ii].imag));
    }
}

//
// eof
//
