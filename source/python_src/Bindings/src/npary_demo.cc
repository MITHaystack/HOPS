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
}
#include <complex>

// This is an artificial example, but these are "library" functions
// that can be directly wrapped by SWIG with only minor effort.

extern "C" void get_np_amps(
    npy_cdouble *vis, long nvis, npy_double *amp, long namp)
{
    std::complex<double> eye (0.0,1.0);
    for (int ii = 0; ii < nvis && ii < namp; ii++) {
        amp[ii] = std::abs((vis[ii].real + eye * vis[ii].imag));
    }
}

extern "C" void get_np_phases(
    npy_cdouble *vis, long nvis, npy_double *phs, long nphs)
{
    std::complex<double> temp;
    std::complex<double> eye (0.0,1.0);
    for (int ii = 0; ii < nvis && ii < nphs; ii++) {
        phs[ii] = std::arg((vis[ii].real + eye * vis[ii].imag));
    }
}

//
// eof
//
