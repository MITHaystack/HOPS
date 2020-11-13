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

%define DOCSTRING
"The mhops module contains python interfaces to connect to the MIT HOPS
library capabilities.  These interfaces are intended to allow convenient
scripting in Python of more sophisticated analyses than is possible using
only the native tools."
%enddef
%module(docstring=DOCSTRING) defaultname
/* package=".." is available to organize into packages */

/* set up a docstring default */
%feature("autodoc","1");

%{
#define SWIG_FILE_WITH_INIT
%}

/* imports/includes for common machinery */

/* access SWIG machinery templates for numpy objects */
%import "numpy.i"

/* defines %pointer_{class,funtions,cast} */
%include "cpointer.i"

/* access C string methods */
%include "cstring.i"

/* set up (non-numpy) C array methods with */
%include "carrays.i"
%feature("docstring", "An object holding an array of doubles") doubleArray;
%array_class(double, doubleArray);

/* include component modules */

/* wrap_msg.h functionalty */
%include "wrap_msg.i"

/* numpy demonstration test code */
%include "npary_demo.i"

/*
 * eof
 */
