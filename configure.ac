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

AC_CONFIG_MACRO_DIRS([m4])
LT_CONFIG_LTDL_DIR(lcld)
LT_INIT

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LEX
AC_PROG_MAKE_SET

# Checks for libraries.

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([inttypes.h stddef.h stdint.h stdlib.h string.h sys/time.h unistd.h])

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
AC_CHECK_FUNCS([clock_gettime floor getcwd gethostname gettimeofday isascii memset mkdir modf pow putenv regcomp rint sqrt strcasecmp strchr strdup strerror strncasecmp strrchr strspn strstr strtoul])

AC_CONFIG_FILES([doc/covertest/Makefile
                 doc/develplan/Makefile
                 doc/geodetic/Makefile
                 doc/requirements/Makefile
                 doc/testreports/Makefile
                 doc/userman/Makefile])
AC_OUTPUT

# eof
