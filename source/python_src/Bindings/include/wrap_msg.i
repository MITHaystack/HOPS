/*
 * SWIG build interface to for Message wrapper functionality for Python3
 * (c) Massachusetts Institute of Technology, 2020
 * The contents of the package Copyright statement apply here.
 *
 * This module provides access to the underlying C/C++
 * functionality of the Message module to using it in Python scripting.
 *
 * This file provides the interface specification.
 * Specifically it mentions functions and structs to be implemented,
 * with comments indicating where the underlying code is to be found.
 *
 * See http://www.swig.org/Doc4.0/SWIGDocumentation.html
 * for documentation on what all of this does and how it works.
 *
 * In the case of the wrap_msg code, the wrap_msg.h include is
 * simple enough that SWIG can use it "as is" for code generation.
 * We define SWIG_NODEFINE here so that the #define'd constants
 * defined in wrap_msg.h do not get defined a second time as a
 * side effect of the interface %include directive.
 */

%{
#include "wrap_msg.h"
%}

/* expose constants with same name name */
%constant int MIN_SEVERITY = MIN_SEVERITY;
%constant int MAX_SEVERITY = MAX_SEVERITY;
%constant int MAX_VERBOSITY = MAX_VERBOSITY;

#define SWIG_NODEFINE
%include "wrap_msg.h"
#undef SWIG_NODEFINE

/*
 * eof
 */
