#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
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

# source makefile
AC_CONFIG_FILES(source/Makefile)

# source/bash_src makefile
AC_CONFIG_FILES(source/bash_src/Makefile)

# source/c_source makefiles; still to do:
    # source/c_src
    # source/c_src/mk4util
    # source/c_src/mk4util/src
    # source/c_src/mk4util/include
    # source/c_src/afio
    # source/c_src/afio/src
    # source/c_src/afio/include
    # source/c_src/applications
    # source/c_src/applications/alist
    # source/c_src/applications/aedit
    # source/c_src/applications/aedit/src
    # source/c_src/applications/aedit/include
    # source/c_src/applications/fourfit
    # source/c_src/dfio
    # source/c_src/dfio/src
    # source/c_src/dfio/include
    # source/c_src/vex
    # source/c_src/vex/text
    # source/c_src/vex/src
    # source/c_src/vex/include
    # source/c_src/fourfit_libs
    # source/c_src/fourfit_libs/ffplot
    # source/c_src/fourfit_libs/ffplot/src
    # source/c_src/fourfit_libs/ffplot/include
    # source/c_src/fourfit_libs/ffsearch
    # source/c_src/fourfit_libs/ffsearch/src
    # source/c_src/fourfit_libs/ffsearch/include
    # source/c_src/fourfit_libs/ffcontrol
    # source/c_src/fourfit_libs/ffcontrol/src
    # source/c_src/fourfit_libs/ffcontrol/include
    # source/c_src/fourfit_libs/ffcore
    # source/c_src/fourfit_libs/ffcore/src
    # source/c_src/fourfit_libs/ffcore/include
    # source/c_src/fourfit_libs/ffio
    # source/c_src/fourfit_libs/ffio/src
    # source/c_src/fourfit_libs/ffmath
    # source/c_src/fourfit_libs/ffmath/src

# source/python_src makefiles; still to do:
    # source/python_src/Makefile
    # source/python_src/scripts/Makefile
    # source/python_src/hops/Makefile

# source/cpp_src makefiles; still to do:
    # source/cpp_src/Makefile
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

# document makefiles; still to do:
    # doc/code_outline/Makefile
    # doc/reference/Makefile
    # doc/specification/code/Makefile # ?
    # doc/developer/Makefile
AC_CONFIG_FILES(Makefile
    doc/Makefile
    doc/common/Makefile
    doc/covertest/Makefile
    doc/develplan/Makefile
    doc/geodetic/Makefile
    doc/requirements/Makefile
    doc/specification/Makefile
    doc/testreports/Makefile
    doc/userman/Makefile
)

#
# Shell setup convenience script
#
AC_CONFIG_FILES(source/bash_src/hops.bash,
    [chmod +x source/bash_src/hops.bash])

# this turns into the generator for config.status
AC_OUTPUT

AC_MHO_BANNER([Finished with configuration:])

# eof
