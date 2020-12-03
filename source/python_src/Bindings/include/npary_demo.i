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
#include "npary_pyvar.h"
%}

%pythonbegin %{
# -- python module beginning --
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
 *
 * However, we only have typemaps for the npy version so we'll build
 * the interfaces with that and provide Python convenience methods to
 * do any other translations at the Python level which is less effort.
 *
 * Per https://numpy.org/devdocs/reference/swig.interface-file.html
 * complex types do not work (completely).  IN_ARRAY and INPLACE_ARRAY
 * work, ARGOUT appears to work for fixed size arrays (i.e. array[N]).
 */
%numpy_typemaps(npy_double,  NPY_DOUBLE, long);
%numpy_typemaps(npy_cdouble, NPY_CDOUBLE, long);
/* %numpy_typemaps(gsl_complex, NPY_CDOUBLE, int); */

/* the interfaces are too complex for SWIG to work out on its own */
/* apply indicates how to use the templaces from numpy.i */
%apply (npy_cdouble *IN_ARRAY1, long DIM1)
    {(npy_cdouble *vis, long nvis)}
%apply (npy_double *INPLACE_ARRAY1, long DIM1)
    {(npy_double *amp, long namp)}
%apply (npy_double *INPLACE_ARRAY1, long DIM1)
    {(npy_double *phs, long nphs)}
void get_np_amps(npy_cdouble *vis, long nvis, npy_double *amp, long namp);
void get_np_phases(npy_cdouble *vis, long nvis, npy_double *phs, long nphs);
%clear npy_cdouble *vis, long nvis;
%clear npy_double *amp, long namp;
%clear npy_double *phs, long nphs;
/* the clears have removed those application rules */

/* generate variant interface functions written in C/C++ */
#define SWIG_INTERFACE
%include "npary_pyvar.h"
#undef SWIG_INTERFACE

/* MyVis was just defined; arrange for Arrays of it */
%array_class(MyVis, MyVisArray);

/* extra functionality:  note that the struct name is needed here, but
   the object is per typedef, and this is C code with C++ish functions;
   a C++ variant can probably be coded as well */
%extend myvis {
    myvis(char *name, double *amps, double *phas, int nvis) {
        int ii;
        MyVis *mv = (MyVis *)malloc(sizeof(MyVis));
        mv->nvis = nvis;
        mv->name = calloc(ii = 50, 1);
        mv->amps = calloc(nvis, sizeof(double));
        mv->phas = calloc(nvis, sizeof(double));
        if (mv->name) { strncpy(mv->name, name, --ii); }
        if (mv->amps) { for (ii=0; ii<nvis; ii++) mv->amps[ii] = amps[ii]; }
        if (mv->phas) { for (ii=0; ii<nvis; ii++) mv->phas[ii] = phas[ii]; }
        return(mv);
    }
    ~myvis() {
        if ($self->name) free($self->name);
        if ($self->amps) free($self->amps);
        if ($self->phas) free($self->phas);
        free($self);
    }
}

/* finally inject any helper functions written in python */
%pythoncode "npary_helpers.py"

/*
 * eof
 */
