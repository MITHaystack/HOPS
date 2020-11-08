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
 *       %include is like #include, but scans the file only once only.
 *       #include is ignored (without -includall) to avoid *everything*
 *
 %module <name> is controlled by Makefile.am
 */

%{
#define SWIG_FILE_WITH_INIT
/* these may move to the numpy module */
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/npy_common.h>
#include <numpy/arrayobject.h>
%}

/* access SWIG machinery templates for numpy objects */
/* does include work here */
%import "numpy.i"

/* defines %pointer_{class,funtions,cast} */
/* %include "cpointer.i" */

/* access C string methods */
/* %include "cstring.i" */

/* set up C array methods */
/*
%include "carrays.i"
%array_class(int, intArray);
%array_class(double, doubleArray);
*/

/* wrap_msg.h functionalty */
%include "wrap_msg.i"


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
