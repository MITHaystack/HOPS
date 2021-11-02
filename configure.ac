#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# (c) Massachusetts Institute of Technology, 2020-2022
# The contents of the package Copyright statement apply here.
#
# See GENERAL PLAN note below for more information about this script.
#
AC_PREREQ([2.69])
AC_INIT([mhops], [4.00], [hops@mit.edu], [mhops],
[https://www.haystack.mit.edu/haystack-observatory-postprocessing-system-hops])
AC_CONFIG_SRCDIR([README.md])
AC_MHO_INIT([Configuring "$PACKAGE_NAME" version "$PACKAGE_VERSION"])
# libtool and other macros into m4
AC_CONFIG_MACRO_DIR([m4])
LT_INIT

#
# mho_config.h is the new configure file,
# but the old tools will only ever include hops_config.h
AH_TOP(dnl
[\\
\\ this can go at the top
\\ #include "hops_config.h"
\\
])
AC_CONFIG_HEADERS([mho_config.h])
AH_BOTTOM(dnl
[\\
\\ or this can be at the bottom
\\#include "hops_config.h"
\\
])
# or conversely.  Decisions on how to do this are deferred.

# put summary configuration into an additional file; AC_MHO macros are
# defined in the m4/mho.m4 and accessed by   aclocal -I m4 # --force
AC_MHO_NOTICE([Autotools configure scripts are in "$srcdir/m4"]) 
AC_MHO_NOTICE([CMake configure scripts are in "$srcdir/cmake"]) 

#
# The GENERAL PLAN is that to the extent possible, tests that can be
# stuffed into scripts put into m4/something.sh and written to support
# both autoconf and cmake, should be so written...now or eventually.
# The directory containing configure is known as $ac_confdir in the script;
# but config.status seems to only know about srcdir so access to the
# scripts must be via $srcdir/m4/... or $srcdir/cmake/....
#
# Other comments:
#   [ ] are quotes to m4sugar.
#
# FIXME: mhops / hops package name collisions; what's the plan?
# FIXME: mailman hops@mit.edu mailing list
#
# for pre-2.69 versions, LT initialization was done differently:
#   m4_ifset([LT_INIT], [LT_INIT], [AC_PROG_LIBTOOL])
#   m4_ifset([LT_WITH_LTDL], [LT_WITH_LTDL], [AC_LIB_LTDL])

# this is not a GNU project, preserve tree in build dir,
# check --help/--version and finally this no $(EXEEXT) support
AM_INIT_AUTOMAKE([foreign subdir-objects std-options no-exeext])

# this macro adds a space-delimited list of additional recursive targets
# coverage allows recursive coverage       (coverage-local: ...)
# hops provides a report on unported code  (hops-local: ...)
# mho provides a report on new code        (mho-local: ...)
# reqs provides a report on requirements   (reqs-local: ...)
AM_EXTRA_RECURSIVE_TARGETS([coverage hops mho reqs])

# user path configuration requires a root directory
AC_ARG_VAR(MHO_ROOT, [Root directory for the MIT HOPS tools installation])
# default --prefix to $MHO_ROOT/$arch-$VERSION at make install time
arch=`uname -m`am
AC_PREFIX_DEFAULT([${MHO_ROOT-'${MHO_ROOT}'}/${arch}-${VERSION-'${VERSION}'}])
if [test -z "$MHO_ROOT"] ; then
    cwd=`pwd`
    MHO_ROOT=`AS_DIRNAME($cwd)`
    AC_MHO_SUMMARY([Setting MHO_ROOT to $MHO_ROOT])
else
    AC_MHO_SUMMARY([MHO_ROOT inherited as $MHO_ROOT])
fi
prefix="$MHO_ROOT/${arch}-$VERSION"
AC_MHO_SUMMARY([Installation is to $prefix])

# features are a bit confusing:
# AC_ARG_ENABLE(FEATURE, HELP-STRING, [ACTION-IF-GIVEN], [ACTION-IF-NOT-GIVEN])
# ACTION-IF-GIVEN refers to any of these:
#   --enable-FEATURE=yes or
#   --enable-FEATURE -> enableval=$enable_FEATURE and enable_FEATURE=yes
#   --disable-FEATURE -> enableval=$enable_FEATURE and enable_FEATURE=no
#   --disable-FEATURE=anything turns into enable_FEATURE=no
AC_ARG_ENABLE(docs,
    AS_HELP_STRING([--disable-docs], [generate some documents]),
        [], [enable_docs=yes])
AC_ARG_ENABLE(doxygen,
    AS_HELP_STRING([--disable-doxy], [allow doxygen-generated documents]),
    [], [enable_doxy=yes])
AC_ARG_ENABLE(sphinx,
    AS_HELP_STRING([--disable-sphinx], [allow sphinx-generated documents]),
    [], [enable_spnx=yes])
AC_ARG_ENABLE(gcov,
    AS_HELP_STRING([--enable-gcov], [compile with coverage active]),
    [], [enable_gcov=no])
# this can disappear as it is now a no-op option
AC_ARG_ENABLE(hack,
    AS_HELP_STRING([--enable-hack], [activate ugly gcov hack on older gcc]),
    [], [enable_hack=no])

# is this an argument to ENABLE for C++ programs?
# HOPS_ENABLE_DEBUG_MSG defined or not

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LEX
AC_PROG_MAKE_SET
# ranlib is obsoleted by lt init

# for when/if we get have to deal with fortran
# AC_PROG_F77
# AC_F77_LIBRARY_LDFLAGS
# AC_PROG_FC

# checks, &c. for C compiler options
mhocppdef="-Wall -Wextra"
AC_ARG_VAR(MHO_CPPFLAGS, [Warning options for C compiler ($mhocppdef)])
if test -z ["$MHO_CPPFLAGS"] ; then
    MHO_CPPFLAGS="-Wall -Wextra"
fi
# these should move to m4 area
# -fkeep-static-functions
mh_cppflags="$CPPFLAGS"
CPPFLAGS="$mh_cppflags -fkeep-static-functions"
AC_PREPROC_IFELSE(
    [AC_LANG_PROGRAM([[const char hw[] = "Hi\n";]],
                     [[fputs (hw, stdout);]])],
    [KEEP_STATIC_FUNCTIONS='-fkeep-static-functions'],
    [KEEP_STATIC_FUNCTIONS=''])
CPPFLAGS="$mh_cppflags"
AC_MSG_NOTICE([KEEP_STATIC_FUNCTIONS is "$KEEP_STATIC_FUNCTIONS"])
AC_SUBST(KEEP_STATIC_FUNCTIONS)

# checks for C++ compiler options
# it seems coding is to the C11 standard, so -std=c++11 (or -std=gnu++11)
# g++ (GCC) 10.3.1 default is -std=gnu++14
mhocxxdef="-Wall -Wextra -std=c++11"
AC_ARG_VAR(MHO_CXXFLAGS, [Warning options for C++ compiler ($mhocxxdef)])
if test -z ["$MHO_CXXFLAGS"] ; then
    MHO_CXXFLAGS="$mhocxxdef"
fi

# less trivial checks for programs
PKG_PROG_PKG_CONFIG
AC_PATH_PROG(SWIG, swig, swig-not-found)
AC_PATH_PROG(PDFLATEX, pdflatex, pdflatex-not-found)
AC_PATH_PROG(DOXYGEN, doxygen, doxygen-not-found)
AC_PATH_PROG(SPHINX, sphinx-build, sphinx-not-found)
AC_PATH_PROG(PERL, perl, perl-not-found)
AC_PATH_PROG(GCOV, gcov, gcov-not-found)
AC_PATH_PROG(HELP2MAN, help2man, help2man-not-found)
AC_PATH_PROG(DOT, dot, dot-not-found)

# simple checks for specific/enable/disable cases
AM_CONDITIONAL(HAVE_SWIG, [test -x $SWIG])
AM_CONDITIONAL(WANT_DOCS, [test x$enable_docs = xyes -a -x $PDFLATEX])
AM_CONDITIONAL(WANT_DOXYGEN, [test x$enable_docs = xyes -a -x $DOXYGEN])
AM_CONDITIONAL(WANT_SPHINX, [test x$enable_docs = xyes -a -x $SPHINX])
AM_CONDITIONAL(DO_COVER, [test x$enable_gcov = xyes -a -x $GCOV])
#M_CONDITIONAL(COVER_HACK, [test x$enable_hack = xyes])
AM_CONDITIONAL(DO_MANPAGES, [test -x $HELP2MAN])
AM_CONDITIONAL(HAVE_DOT, [test -x $DOT])

# and share the conditionals with the summary
AC_MHO_NOTICE([%%% $HAVE_SWIG_TRUE HAVE_SWIG "$SWIG"])
AC_MHO_NOTICE([%%% $WANT_DOCS_TRUE WANT_DOCS])
AC_MHO_NOTICE([%%% $WANT_DOXYGEN_TRUE WANT_DOXYGEN])
AC_MHO_NOTICE([%%% $WANT_SPHINX_TRUE WANT_SPHINX])
AC_MHO_NOTICE([%%% $DO_COVER_TRUE DO_COVER])
#C_MHO_NOTICE([%%% $COVER_HACK_TRUE COVER_HACK])
AC_MHO_NOTICE([%%% $DO_MANPAGES_TRUE DO_MANPAGES "$HELP2MAN"])

# getting our head around Python, see package section below
AC_PATH_PROG([PYTHON2], [python2], [python2-not-found])
AC_PATH_PROG([PYTHON3], [python3], [python3-not-found])
# for the moment, require 3.0
AM_PATH_PYTHON([3.0])
# sets PYTHON, PYTHON_VERSION, PYTHON_PLATFORM
# (PYTHON_PREFIX PYTHON_EXEC_PREFIX)
# pythondir, pkgpythondir, pyexecdir, pkgpyexecdir
# more PYTHON is below

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([inttypes.h stddef.h stdint.h stdlib.h string.h dnl
    sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
# this puts things in the config file
AC_CHECK_FUNCS([clock_gettime floor getcwd gethostname gettimeofday dnl
    isascii memset mkdir modf pow putenv regcomp rint sqrt strcasecmp dnl
    strchr strdup strerror strncasecmp strrchr strspn strstr strtoul ])
# this builds the default LIBS collection
AC_SEARCH_LIBS([floor], [m])

# AC_CHECK_LIBS deprecated relative to AC_SEARCH_LIBS
# don't have code (yet) using GSL
# AC_SEARCH_LIBS([gsl_ran_gaussian], [gsl])
# AC_SEARCH_LIBS([cblas_dgemm], [gslcblas])
# or PKG_CHECK_MODULES(gsl) which defines GSL_CBLAS_LIB=-lgslcblas

# Special variables
#if [test -z "$KPSETEXMFPATH"] ; then [KPSETEXMFPATH=':'] ; fi
KPSETEXMFPATH=`$srcdir/m4/texlive-missing.sh`
AC_ARG_VAR(KPSETEXMFPATH, [normally ':' to find installed TeX files])

#
# package dependency checking; ideally these should be optional
#

# PERL should be optional
AM_CONDITIONAL(HAVE_PERL, [test x$perl != xperl-not-found -a -x "$PERL"])

# PYTHON
# checking for the module allows us to build C extensions
AC_ARG_VAR(PYTHON_EXECUTABLE, [force us to use some python executable])
#PKG_CHECK_MODULES(PYTHON, python3, [has_python=true], [has_python=false])
PKG_CHECK_MODULES(PYTHON, python3)
if [test -n "$PYTHON_VERSION"] ; then
    AC_DEFINE([HAVE_PYTHON], 1, [Define to 1 when PYTHON is available])
    AC_MHO_NOTICE([PYTHON was found via check modules, but...])
    has_python=true
else
    AC_DEFINE([HAVE_PYTHON], 0, [Define to 1 when PYTHON is available])
    AC_MHO_NOTICE([PYTHON not found via check modules, but...])
    has_python=false
fi
# capture for the summary
AC_MHO_NOTICE([python2 is $PYTHON2])
AC_MHO_NOTICE([python3 is $PYTHON3])
AC_MHO_NOTICE([python is $PYTHON])
# this can get much more elaborate
if [test -z "$PYTHON_EXECUTABLE"] ; then
   if [test -n "$PYTHON3"] ; then
       PYTHON_EXECUTABLE=$PYTHON3
   elif [test -n "$PYTHON"] ; then
       PYTHON_EXECUTABLE=$PYTHON
   elif [test -n "$PYTHON2"] ; then
       PYTHON_EXECUTABLE=$PYTHON2
   else
       PYTHON_EXECUTABLE=false
   fi
else
   AC_MHO_NOTICE([PYTHON_EXECUTABLE already set to $PYTHON_EXECUTABLE])
fi
# finally to wrap it up
if [test -x $PYTHON_EXECUTABLE] ; then
    has_python=true
else
    has_python=false
fi
AC_MHO_NOTICE([PYTHON_EXECUTABLE is $PYTHON_EXECUTABLE])
AC_SUBST(PYTHON_CFLAGS)
AC_SUBST(PYTHON_LIBS)
AM_CONDITIONAL(HAVE_PYTHON, $has_python)
AC_MHO_NOTICE([%%% has_python is $has_python (required)])
# finally, for reference
major=[`$PYTHON_EXECUTABLE -c 'import sys; print(sys.version_info[0])'`]
minor=[`$PYTHON_EXECUTABLE -c 'import sys; print(sys.version_info[1])'`]
patch=[`$PYTHON_EXECUTABLE -c 'import sys; print(sys.version_info[2])'`]
AC_SUBST(MODULE_VERSION_MAJOR, $major)
AC_SUBST(MODULE_VERSION_MINOR, $minor)
AC_SUBST(MODULE_VERSION_PATCH, $patch)

# finally numpy includes are not easily found
NUMPY_INCLUDE=[`$PYTHON3 -c'import numpy; print(numpy.get_include())'`]
AC_SUBST(NUMPY_INCLUDE)

# FFTW3
PKG_CHECK_MODULES(FFTW3, fftw3, [has_fftw3=true], [has_fftw3=false])
if [$has_fftw3] ; then
    AC_DEFINE([HAVE_FFTW], 1, [Define to 1 when FFTW is available])
    AC_MHO_NOTICE([FFTW3 was found via check modules])
else
    AC_DEFINE([HAVE_FFTW], 0, [Define to 1 when FFTW is available])
    AC_MHO_NOTICE([FFTW3 not found via check modules])
fi
AC_MHO_NOTICE([%%% has_fftw3 is $has_fftw3 (required)])
AC_SUBST(FFTW3_CFLAGS)
AC_SUBST(FFTW3_LIBS)
AM_CONDITIONAL(HAVE_FFTW3, $has_fftw3)

# PGPLOT -- note that we typically don't need PGPLOT_CFLAGS or PGPLOT_LIBS
PKG_CHECK_MODULES(PGPLOT, pgplot, [has_pgplot=true], [has_pgplot=false])
if [$has_pgplot] ; then
    AC_DEFINE([HAVE_PGPLOT], 1, [Define to 1 when PGPLOT is available])
    AC_MHO_NOTICE([PGPLOT was found via check modules])
else
    AC_DEFINE([HAVE_PGPLOT], 0, [Define to 1 when PGPLOT is available])
    AC_MHO_NOTICE([PGPLOT not found via check modules])
fi
AC_MHO_NOTICE([%%% has_pgplot is $has_pgplot (required)])
AC_SUBST(PGPLOT_CFLAGS)
AC_SUBST(PGPLOT_LIBS)
# CPGPLOT -- as CPGPLOT_LIBS usually includes the pgplot linkage
PKG_CHECK_MODULES(CPGPLOT, cpgplot, [has_cpgplot=true], [has_cpgplot=false])
if [$has_cpgplot] ; then
    AC_DEFINE([HAVE_CPGPLOT], 1, [Define to 1 when CPGPLOT is available])
    AC_MHO_NOTICE([CPGPLOT was found via check modules])
else
    AC_DEFINE([HAVE_CPGPLOT], 0, [Define to 1 when CPGPLOT is available])
    AC_MHO_NOTICE([CPGPLOT not found via check modules])
fi
AC_MHO_NOTICE([%%% has_cpgplot is $has_cpgplot (required)])
AC_SUBST(CPGPLOT_CFLAGS)
AC_SUBST(CPGPLOT_LIBS)
AM_CONDITIONAL(HAVE_PGPLOT, [$has_cpgplot && $has_pgplot])
# finally PGPLOT support -- for the present, hardwire to system location
PGPLOT_FONT=/usr/share/pgplot/grfont.dat
PGPLOT_RGB=/usr/share/pgplot/rgb.txt
AC_SUBST(PGPLOT_FONT)
AC_SUBST(PGPLOT_RGB)

#
# output stage
#

# top-level Makefile and primary directories
AC_CONFIG_FILES(
    Makefile
    doc/Makefile
    source/Makefile
    data/Makefile
)

# source/bash_src makefile
# includes perl scripts as well
AC_CONFIG_FILES(source/bash_src/Makefile)

# source/c_source makefiles, part 1: "sub"
AC_CONFIG_FILES(
    source/c_src/Makefile
    source/c_src/msg/Makefile
    source/c_src/mk4util/Makefile
    source/c_src/dfio/Makefile
    source/c_src/afio/Makefile
    source/c_src/vex/Makefile
)
# source/c_source makefiles, part 2: "fourfit"
AC_CONFIG_FILES(
    source/c_src/fourfit_libs/Makefile
    source/c_src/fourfit_libs/ffcontrol/Makefile
    source/c_src/fourfit_libs/ffcore/Makefile
    source/c_src/fourfit_libs/ffmath/Makefile
    source/c_src/fourfit_libs/ffio/Makefile
    source/c_src/fourfit_libs/ffsearch/Makefile
    source/c_src/fourfit_libs/ffplot/Makefile
    source/c_src/applications/fourfit/Makefile
    source/c_src/applications/fourmer/Makefile
    source/c_src/applications/fplot/Makefile
    source/c_src/applications/fringex/Makefile
)
# source/c_source makefiles, part 3: "aedit family"
AC_CONFIG_FILES(
    source/c_src/applications/Makefile
    source/c_src/applications/aedit/Makefile
    source/c_src/applications/alist/Makefile
    source/c_src/applications/adump/Makefile
)
# source/c_source makefiles, part 4: "mmvlbi apps"
AC_CONFIG_FILES(
    source/c_src/applications/average/Makefile
    source/c_src/applications/cofit/Makefile
    source/c_src/applications/search/Makefile
    source/c_src/applications/snratio/Makefile
)
# source/c_source makefiles, part 5: "legacy apps"
AC_CONFIG_FILES(
    source/c_src/applications/legacy/Makefile
    source/c_src/applications/legacy/CorAsc2/Makefile
)
# source/c_source makefiles, part 6: "test"
AC_CONFIG_FILES(
    source/c_src/applications/test/Makefile
)

# source/cpp_src makefiles, part 1: "core libraries"
AC_CONFIG_FILES(
    source/cpp_src/Makefile
    source/cpp_src/Message/Makefile
)
#   source/cpp_src/Math/Makefile
#   source/cpp_src/Utilities/Makefile
#   source/cpp_src/MK4Interface/Makefile
#   source/cpp_src/Containers/Makefile
#   source/cpp_src/vex2json/Makefile
#)
#   source/cpp_src/Applications/Makefile
#   source/cpp_src/Applications/fourfit/Makefile
#   source/cpp_src/Calibration/Makefile
#   source/cpp_src/Operators/Makefile
#   source/cpp_src/Plugins/Makefile
#   source/cpp_src/Plugins/OpenCL/Makefile
#   source/cpp_src/Plugins/PythonBindings/Makefile

# source/python_src: makefiles
AC_CONFIG_FILES(
    source/python_src/Makefile
    source/python_src/Bindings/Makefile
)
# this is just a test script
AC_CONFIG_FILES(source/python_src/scripts/test.py,
    [chmod +x source/python_src/scripts/test.py])
# need to formulate the plan for this and other pythonic things
    # source/python_src/hops/Makefile

# document makefiles
AC_CONFIG_FILES(
    doc/common/Makefile
    doc/texmiss/Makefile
    doc/covertest/Makefile
    doc/tasking/Makefile
    doc/develplan/Makefile
    doc/geodetic/Makefile
    doc/requirements/Makefile
    doc/specification/Makefile
    doc/testreports/Makefile
    doc/userman/Makefile
    doc/texcheck/Makefile
)
# still to do (perhaps):
    # doc/code_outline/Makefile
    # doc/reference/Makefile
    # doc/specification/code/Makefile # ?

# data and scripting for system tests
#AC_CONFIG_FILES()

#
# Shell setup convenience script
#
AC_CONFIG_FILES(source/bash_src/hops.bash,
    [chmod +x source/bash_src/hops.bash])

# MHO/HOPS compatibility
AC_CONFIG_FILES(hops_config.h, [$srcdir/m4/hops_svn_rev.sh])

# this turns into the generator for config.status
AC_OUTPUT

# finally summarize what happened
AC_MHO_BANNER([Finished with configuration:])

# eof
