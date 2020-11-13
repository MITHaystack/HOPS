/*
 * SWIG build interface for simple numpy example tests in Python3
 * (c) Massachusetts Institute of Technology, 2020
 * The contents of the package Copyright statement apply here.
 *
 * This module provides access to the underlying numpy API.
 */

%{
#include "npary_demo.h"
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>
%}

/* put in some markers for the novice */
%pythonbegin %{
# start-up Python material should it be needed
import numpy as np
%}

/* similar markers in the SWIG .c code */
%begin   %{ /* -- begin   section -- */ %}
%runtime %{ /* -- runtime section -- */ %}
%header  %{ /* -- header  section -- */ %}
%wrapper %{ /* -- wrapper section -- */ %}
%init %{
/* -- init    section -- */
/* https://numpy.org/devdocs/reference/c-api/array.html#miscellaneous */
import_array();
%}

/*
 * <complex.h> is {double x; double y;} (historically, anyway)
 * Py_complex  is {double real; double imag;}
 * npy_cdouble is {double real, imag;}
 * gsl_complex is { double dat[2]; }    (dtype=np.complex128)
 * so these are all bitwise equivalent in C.
 * However, we only have typemaps for the npy version so we'll build
 * the interfaces with that and provide Python convenience methods to
 * do any other translations at the Python level which is less effort.
 */
%numpy_typemaps(npy_double,  NPY_DOUBLE, int);
%numpy_typemaps(npy_cdouble, NPY_CDOUBLE, int);
/* %numpy_typemaps(gsl_complex, NPY_CDOUBLE, int); */

/* the interfaces are too complex for SWIG to work out on its own */
/* apply indicates how to use the templaces from numpy.i */
%apply (npy_cdouble *IN_ARRAY1, int DIM1)
    {(npy_cdouble *vis, int nvis)}
%apply (npy_double *ARGOUT_ARRAY1, int DIM1)
    {(npy_double *amp, int namp)}
%apply (npy_double *ARGOUT_ARRAY1, int DIM1)
    {(npy_double *phs, int nphs)}
void get_np_amps(npy_cdouble *vis, int nvis, npy_double *amp, int namp);
void get_np_phases(npy_cdouble *vis, int nvis, npy_double *phs, int nphs);
%clear npy_cdouble *vis, int nvis;
%clear npy_double *amp, int namp;
%clear npy_double *phs, int nphs;
/* the clears have removed those application rules */

/* generate variant interface functions written in C/C++ */
%include "npary_pyvar.h"

/* finally inject any helper functions written in python */
%pythoncode "npary_helpers.py"

/*
 * eof
 */
