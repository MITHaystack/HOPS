//
// Demo code to exercise some SWIG interfaces
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
#include "npary_demo.h"
#include <complex.h>
}

// This is an artificial example, but these are "library" functions
// that can be directly wrapped by SWIG with only minor effort.

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
