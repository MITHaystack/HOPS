/*
 * Additional SWIG wrapper demo source
 * (c) Massachusetts Institute of Technology, 2020
 * The contents of the package Copyright statement apply here.
 *
 * This example is fully contrived, but you want such a file when
 * the functions you have (from lower levels) have an interface that
 * is somewhat awkward to pass through the SWIG/numpy rules and it
 * is just easiest to build something that plays better in C/C++.
 *
 * The notion here is that MyVis can function as an object.
 */
#ifndef npary_pyvar_h
#define npary_pyvar_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct myvis {
    char    *name;          /* name of something */
    double  *amps;          /* pointer to some amplitudes */
    double  *phas;          /* pointer to some phases */
    int     nvis;           /* length of both arrays */
} MyVis;

/* function that describes one or more MyVis into (created) *buf */
#ifdef SWIG_INTERFACE
/* specifies that (char **buf) arguments are released by a free() call */
%cstring_output_allocate(char **buf, free(*$1));
/* create pointer methods for the MyVis object */
%pointer_functions(MyVis, MyVisPtr);
#endif /* SWIG_INTERFACE */
extern void describe_one_vis(MyVis *mv, char **buf);
extern void describe_all_vis(MyVis *mv, long num, char **buf);

/* other support functions */

#endif /* npary_pyvar_h */
/*
 * eof
 */
