#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#
# FIXME: mhops / hops package name collisions?
# FIXME: mailman hops@mit.edu mailing list

AC_PREREQ([2.69])
AC_INIT([mhops], [4.00], [hops@mit.edu])
AC_CONFIG_SRCDIR([README.md])
AC_CONFIG_HEADERS([mho_config.h])
AM_INIT_AUTOMAKE([foreign subdir-objects])

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
AC_CONFIG_MACRO_DIRS([m4])
LT_INIT
# for pre-2.69 versions:
#m4_ifset([LT_INIT], [LT_INIT], [AC_PROG_LIBTOOL])
#m4_ifset([LT_WITH_LTDL], [LT_WITH_LTDL], [AC_LIB_LTDL])

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

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LEX
AC_PROG_MAKE_SET

# checks for specific/enable/disable cases
AC_PATH_PROG([PDFLATEX], [pdflatex], [no-such-code])
AC_PATH_PROG([DOXYGEN], [doxygen], [doxygen-not-found])
AC_PATH_PROG([SPHINX], [sphinx], [sphinx-not-found])
AM_CONDITIONAL(WANT_DOCS, [test $enable_docs = yes -a -x $PDFLATEX])
AM_CONDITIONAL(WANT_DOXYGEN, [test $enable_docs = yes -a -x $DOXYGEN])
AM_CONDITIONAL(WANT_SPHINX, [test $enable_docs = yes -a -x $SPHINX])

# Checks for libraries.

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
AC_CHECK_FUNCS([clock_gettime floor getcwd gethostname gettimeofday dnl
    isascii memset mkdir modf pow putenv regcomp rint sqrt strcasecmp dnl
    strchr strdup strerror strncasecmp strrchr strspn strstr strtoul ])

# Special variables
if [test -z "$KPSETEXMFPATH"] ; then [KPSETEXMFPATH=':'] ; fi
AC_ARG_VAR(KPSETEXMFPATH, [normally ':' to find installed TeX files])

#
# package dependency checking; ideally these should be optional
#

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
AM_CONDITIONAL(HAVE_FFTW, $has_fftw3)

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
    data/Makefile
    source/Makefile
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
)
# still to do:
    # source/c_src/applications/fourfit/Makefile

# source/cpp_src makefiles
AC_CONFIG_FILES(source/cpp_src/Makefile)
# still to do:
    # source/cpp_src/Messaging/Makefile
    # source/cpp_src/Messaging/src/Makefile
    # source/cpp_src/Messaging/include/Makefile
    # source/cpp_src/MK4Interface/Makefile
    # source/cpp_src/MK4Interface/src/Makefile
    # source/cpp_src/MK4Interface/include/Makefile
    # source/cpp_src/Bindings/Makefile
    # source/cpp_src/Utilities/Makefile
    # source/cpp_src/Utilities/src/Makefile
    # source/cpp_src/Utilities/include/Makefile
    # source/cpp_src/Test/Makefile
    # source/cpp_src/Containers/Makefile
    # source/cpp_src/Containers/include/Makefile

# source/python_src makefiles
AC_CONFIG_FILES(source/python_src/Makefile)
# still to do:
    # source/python_src/Makefile
    # source/python_src/scripts/Makefile
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

# data for system tests
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
