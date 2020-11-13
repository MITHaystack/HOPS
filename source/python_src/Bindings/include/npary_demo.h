/*
 * Header for numpy test code
 * (c) Massachusetts Institute of Technology, 2020
 * The contents of the package Copyright statement apply here.
 */
#ifndef npary_demo_h
#define npary_demo_h

#include <numpy/npy_common.h>

/* void functions are somewhat easier to work with */
extern void get_np_amps(
    npy_cdouble *vis, long nvis, npy_double *amp, long namp);
extern void get_np_phases(
    npy_cdouble *vis, long nvis, npy_double *phs, long nphs);

#endif /* npary_demo_h */
/*
 * eof
 */
