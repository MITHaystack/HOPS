#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#
# FIXME: mhops / hops package name collisions; what's the plan?
# FIXME: mailman hops@mit.edu mailing list
#
# NOTE: [ ] are quotes to m4sugar.

AC_PREREQ([2.69])
AC_INIT([mhops], [4.00], [hops@mit.edu])
AC_CONFIG_SRCDIR([README.md])
AC_CONFIG_HEADERS([mho_config.h])

# this is not a GNU project, preserve tree in build dir, check --help/--version
AM_INIT_AUTOMAKE([foreign subdir-objects std-options])

# this macro adds a space-delimited list of additional recursive targets
# coverage allows recursive coverage       (coverage-local: ...)
# hops provides a report on unported code  (hops-local: ...)
# mho provides a report on new code        (mho-local: ...)
# reqs provides a report on requirements   (reqs-local: ...)
AM_EXTRA_RECURSIVE_TARGETS([coverage hops mho reqs])

# user path configuration requires a root directory
AC_ARG_VAR(MHO_ROOT, [Root directory for the MIT HOPS tools installation])
AC_MHO_INIT([Configuration summary for mhops version 4.00])
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

# libtool stuff into m4
AC_CONFIG_MACRO_DIR([m4])
LT_INIT
# for pre-2.69 versions:
#m4_ifset([LT_INIT], [LT_INIT], [AC_PROG_LIBTOOL])
#m4_ifset([LT_WITH_LTDL], [LT_WITH_LTDL], [AC_LIB_LTDL])

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
    [], [enable_doxy=yes])
AC_ARG_ENABLE(gcov,
    AS_HELP_STRING([--enable-gcov], [compile with coverage active]),
    [], [enable_gcov=no])

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

# less trivial checks for programs
PKG_PROG_PKG_CONFIG
AC_PATH_PROG(SWIG, swig, swig-not-found)
AC_PATH_PROG(PDFLATEX, pdflatex, pdflatex-not-found)
AC_PATH_PROG(DOXYGEN, doxygen, doxygen-not-found)
AC_PATH_PROG(SPHINX, sphinx-build, sphinx-not-found)
AC_PATH_PROG(PERL, perl, perl-not-found)
AC_PATH_PROG(GCOV, gcov, gcov-not-found)
AC_PATH_PROG(HELP2MAN, help2man, help2man-not-found)

# simple checks for specific/enable/disable cases
AM_CONDITIONAL(HAVE_SWIG, [test -x $SWIG])
AM_CONDITIONAL(WANT_DOCS, [test x$enable_docs = xyes -a -x $PDFLATEX])
AM_CONDITIONAL(WANT_DOXYGEN, [test x$enable_docs = xyes -a -x $DOXYGEN])
AM_CONDITIONAL(WANT_SPHINX, [test x$enable_docs = xyes -a -x $SPHINX])
AM_CONDITIONAL(DO_COVER, [test x$enable_gcov = xyes -a -x $GCOV])
AM_CONDITIONAL(DO_MANPAGES, [test -x $HELP2MAN])

# getting our head around Python, see package section below
AC_PATH_PROG([PYTHON2], [python2], [python2-not-found])
AC_PATH_PROG([PYTHON3], [python3], [python3-not-found])
# for the moment, require 3.0
AM_PATH_PYTHON([3.0])
# sets PYTHON, PYTHON_VERSION, PYTHON_PLATFORM
# (PYTHON_PREFIX PYTHON_EXEC_PREFIX)
# pythondir, pkgpythondir, pyexecdir, pkgpyexecdir

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
if [test -z "$KPSETEXMFPATH"] ; then [KPSETEXMFPATH=':'] ; fi
AC_ARG_VAR(KPSETEXMFPATH, [normally ':' to find installed TeX files])

#
# package dependency checking; ideally these should be optional
#

# PERL should be optional
AM_CONDITIONAL(HAVE_PERL, [test x$perl != xperl-not-found -a -x "$PERL"])

# PYTHON
# checking for the module allows us to build C extensions
AC_ARG_VAR(PYTHON_EXECUTABLE, [force us to use some python executable])
PKG_CHECK_MODULES(PYTHON, python3, [has_python=true], [has_python=false])
if [$has_python] ; then
    AC_DEFINE([HAVE_PYTHON], 1, [Define to 1 when PYTHON is available])
    AC_MHO_NOTICE([PYTHON was found via check modules])
else
    AC_DEFINE([HAVE_PYTHON], 0, [Define to 1 when PYTHON is available])
    AC_MHO_NOTICE([PYTHON not found via check modules])
fi
# capture for the summary
AC_MHO_NOTICE([python2 is $PYTHON2])
AC_MHO_NOTICE([python3 is $PYTHON3])
AC_MHO_NOTICE([python is $PYTHON])
AC_MHO_NOTICE([%%% has_python is $has_python (required)])
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
AC_MHO_NOTICE([PYTHON_EXECUTABLE is $PYTHON_EXECUTABLE])
AC_SUBST(PYTHON_CFLAGS)
AC_SUBST(PYTHON_LIBS)
AM_CONDITIONAL(HAVE_PYTHON, $has_python)
# finally, for reference
major=[`$PYTHON_EXECUTABLE -c 'import sys; print(sys.version_info[0])'`]
minor=[`$PYTHON_EXECUTABLE -c 'import sys; print(sys.version_info[1])'`]
patch=[`$PYTHON_EXECUTABLE -c 'import sys; print(sys.version_info[2])'`]
AC_SUBST(MODULE_VERSION_MAJOR, $major)
AC_SUBST(MODULE_VERSION_MINOR, $minor)
AC_SUBST(MODULE_VERSION_PATCH, $patch)

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

# top-level Makefile and primary directories
AC_CONFIG_FILES(
    Makefile
    doc/Makefile
    source/Makefile
    data/Makefile
)

### FIXME: revert the 'b' names to the normal names

# source/bash_src makefile
# includes perl scripts as well
AC_CONFIG_FILES(source/bash_src/Makefile)

# source/c_source makefiles, part 1: "sub" and applications
AC_CONFIG_FILES(
    source/c_src/Makefile
    source/c_src/mk4util/Makefile
    source/c_src/dfio/Makefile
    source/c_src/afio/Makefile
    source/c_src/vex/Makefile
    source/c_src/applications/Makefile
    source/c_src/applications/alist/Makefile
    source/c_src/applications/aedit/Makefile
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
)

# source/cpp_src makefiles
AC_CONFIG_FILES(
    source/cpp_src/Makefile
    source/cpp_src/Message/Makefile
    source/cpp_src/Math/Makefile
    source/cpp_src/Utilities/Makefile
    source/cpp_src/MK4Interface/Makefile
    source/cpp_src/Containers/Makefile
)

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
    doc/covertest/Makefile
    doc/develplan/Makefile
    doc/geodetic/Makefile
    doc/requirements/Makefile
    doc/specification/Makefile
    doc/testreports/Makefile
    doc/userman/Makefile
)
# still to do:
    # doc/code_outline/Makefile
    # doc/reference/Makefile
    # doc/specification/code/Makefile # ?
    # doc/developer/Makefile

# data and scripting for system tests
#AC_CONFIG_FILES()

# captive copy of hops3.x
#AC_CONFIG_FILES()

#
# Shell setup convenience script
#
AC_CONFIG_FILES(source/bash_src/hops.bash,
    [chmod +x source/bash_src/hops.bash])

# MHO/HOPS compatibility (temporary)
AC_CONFIG_FILES(hops_config.h,
    [echo '#define HOPS_SVN_REV 3089' >> hops_config.h;
     echo "#define PACKAGE_VERSION $VERSION" >> hops_config.h])

# this turns into the generator for config.status
AC_OUTPUT

AC_MHO_BANNER([Finished with configuration:])

# eof
