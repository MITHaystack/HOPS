/*
 * SWIG build interface to hops functionality for Python3
 * (c) Massachusetts Institute of Technology, 2020
 * The contents of the package Copyright statement apply here.
 *
 * This module provides access to the underlying C/C++
 * functionality of HOPS to allow scripting in Python.
 *
 * This file provides the interface specification.
 * Specifically it mentions functions and structs to be implemented,
 * with comments indicating where the underlying code is to be found.
 *
 * See http://www.swig.org/Doc4.0/SWIGDocumentation.html
 * for documentation on what all of this does and how it works.
 *
 * Note: %import collects information, but doesn't generate anything
 *        * %include is like #include, but scans the file only once only.
 */

/* this is the name for the Python module */
%module mhops

%{
#define SWIG_FILE_WITH_INIT
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/npy_common.h>
#include <numpy/arrayobject.h>
#include "wrap_msg.h"
%}

/* set up SWIG machinery for as-needed use with numpy */
%import "numpy.i"

/* set up C pointer methods */
%include "cpointer.i"

/* set up C string methods */
%include "cstring.i"

/* set up C array methods */
%include "carrays.i"
%array_class(int, intArray);
%array_class(double, doubleArray);

/* wrap_msg.h functionalty */

/* expose constants with same name */
%constant int MIN_SEVERITY = MIN_SEVERITY;
%constant int MAX_SEVERITY = MAX_SEVERITY;
%constant int MAX_VERBOSITY = MAX_VERBOSITY;

/* specify interfaces to be created from wrap_msg */
void wrap_message(int verbosity, int severity, char *message);

void set_wrap_msglevel(int msglev);
int get_wrap_msglevel(void);

void set_wrap_progname(const char *name);
/* %newobject is needed if something malloc'd is returned */
/* %newobject get_wrap_progname; */
char *get_wrap_progname(void);

/* other modules */

/* generate numpy-related interface functions written in C/C++ */
/* %include "mhoary.i" */

/* generate variant interface functions written in C/C++ */
/* %include "mho_pyvar.h" */

/* finally inject any helper functions written in python */
/* %pythoncode "mho_helpers.py" */

/*
 * eof
 */
