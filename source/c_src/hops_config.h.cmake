#ifndef HOPS_CONFIG_H__
#define HOPS_CONFIG_H__

/******************************************************************************/
/* The following values are required to compile hops. */

/* HOPS package rev number */
#define HOPS_REVISION @HOPS_REVISION_VALUE@

/* Haystack SVN rev number */
#define HOPS_SVN_REV @HOPS_SVN_REV_VALUE@

#cmakedefine WORDS_BIGENDIAN

/* Worry about the byte order in data files */
#ifdef WORDS_BIGENDIAN
/* Data are fine as they are */
#else /* WORDS_BIGENDIAN */
    #define BYTFLP
#endif /* WORDS_BIGENDIAN */

/*Everything below this line was essentially copied wholesale from hops_config.h.in */
/*However its not clear what values are actually used in the code. */
/******************************************************************************/

/* hops_config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
#cmakedefine AC_APPLE_UNIVERSAL_BUILD

/* Define to 1 if the `closedir' function returns void instead of `int'. */
#cmakedefine CLOSEDIR_VOID

/* Define to one of `_getb67', `GETB67', `getb67' for Cray-2 and Cray-YMP
   systems. This function is required for `alloca.c' support on those systems.
   */
#cmakedefine CRAY_STACKSEG_END

/* Define to 1 if using `alloca.c'. */
#cmakedefine C_ALLOCA

/* Define to 1 if you have `alloca', as a function or macro. */
#cmakedefine HAVE_ALLOCA

/* Define to 1 if you have <alloca.h> and it should be used (not on Ultrix).
   */
#cmakedefine HAVE_ALLOCA_H

/* Define to 1 if you have the `argz_add' function. */
#cmakedefine HAVE_ARGZ_ADD

/* Define to 1 if you have the `argz_append' function. */
#cmakedefine HAVE_ARGZ_APPEND

/* Define to 1 if you have the `argz_count' function. */
#cmakedefine HAVE_ARGZ_COUNT

/* Define to 1 if you have the `argz_create_sep' function. */
#cmakedefine HAVE_ARGZ_CREATE_SEP

/* Define to 1 if you have the <argz.h> header file. */
#cmakedefine HAVE_ARGZ_H

/* Define to 1 if you have the `argz_insert' function. */
#cmakedefine HAVE_ARGZ_INSERT

/* Define to 1 if you have the `argz_next' function. */
#cmakedefine HAVE_ARGZ_NEXT

/* Define to 1 if you have the `argz_stringify' function. */
#cmakedefine HAVE_ARGZ_STRINGIFY

/* Define to 1 if you have the <arpa/inet.h> header file. */
#cmakedefine HAVE_ARPA_INET_H

/* Define to 1 if you have the `closedir' function. */
#cmakedefine HAVE_CLOSEDIR

/* Define to 1 if you have the declaration of `cygwin_conv_path', and to 0 if
   you don't. */
#cmakedefine HAVE_DECL_CYGWIN_CONV_PATH

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_DIRENT_H

/* Define if you have the GNU dld library. */
#cmakedefine HAVE_DLD

/* Define to 1 if you have the <dld.h> header file. */
#cmakedefine HAVE_DLD_H

/* Define to 1 if you have the `dlerror' function. */
#cmakedefine HAVE_DLERROR

/* Define to 1 if you have the <dlfcn.h> header file. */
#cmakedefine HAVE_DLFCN_H

/* Define to 1 if you have the <dl.h> header file. */
#cmakedefine HAVE_DL_H

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
#cmakedefine HAVE_DOPRNT

/* Define if you have the _dyld_func_lookup function. */
#cmakedefine HAVE_DYLD

/* Define to 1 if the system has the type `error_t'. */
#cmakedefine HAVE_ERROR_T

/* Define to 1 if you have the <fcntl.h> header file. */
#cmakedefine HAVE_FCNTL_H

/* Define to 1 if FFTW is available */
#cmakedefine HAVE_FFTW

/* Define to 1 if you have the <float.h> header file. */
#cmakedefine HAVE_FLOAT_H

/* Define to 1 if you have the `floor' function. */
#cmakedefine HAVE_FLOOR

/* Define to 1 if you have the `getcwd' function. */
#cmakedefine HAVE_GETCWD

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H

/* Define to 1 if you have the `isascii' function. */
#cmakedefine HAVE_ISASCII

/* Define if you have the libdl library or equivalent. */
#cmakedefine HAVE_LIBDL

/* Define if libdlloader will be built on this platform */
#cmakedefine HAVE_LIBDLLOADER

/* Define to 1 if you have the <limits.h> header file. */
#cmakedefine HAVE_LIMITS_H

/* Define to 1 if you have the <locale.h> header file. */
#cmakedefine HAVE_LOCALE_H

/* Define to 1 if `lstat' has the bug that it succeeds when given the
   zero-length file name argument. */
#cmakedefine HAVE_LSTAT_EMPTY_STRING_BUG

/* Define this if a modern libltdl is already installed */
#cmakedefine HAVE_LTDL

/* Define to 1 if you have the <mach-o/dyld.h> header file. */
#cmakedefine HAVE_MACH_O_DYLD_H

/* Define to 1 if your system has a GNU libc compatible `malloc' function, and
   to 0 otherwise. */
#cmakedefine HAVE_MALLOC

/* Define to 1 if you have the <memory.h> header file. */
#cmakedefine HAVE_MEMORY_H

/* Define to 1 if you have the `memset' function. */
#cmakedefine HAVE_MEMSET

/* Define to 1 if you have the `mkdir' function. */
#cmakedefine HAVE_MKDIR

/* Define to 1 if you have the `modf' function. */
#cmakedefine HAVE_MODF

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
#cmakedefine HAVE_NDIR_H

/* Define to 1 if you have the <netdb.h> header file. */
#cmakedefine HAVE_NETDB_H

/* Define to 1 if you have the <netinet/in.h> header file. */
#cmakedefine HAVE_NETINET_IN_H

/* Define to 1 if you have the `opendir' function. */
#cmakedefine HAVE_OPENDIR

/* Define to 1 if you have the `pow' function. */
#cmakedefine HAVE_POW

/* Define if libtool can extract symbol lists from object files. */
#cmakedefine HAVE_PRELOADED_SYMBOLS

/* Define to 1 if you have the `readdir' function. */
#cmakedefine HAVE_READDIR

/* Define to 1 if your system has a GNU libc compatible `realloc' function,
   and to 0 otherwise. */
#cmakedefine HAVE_REALLOC

/* Define to 1 if you have the `rint' function. */
#cmakedefine HAVE_RINT

/* Define if you have the shl_load function. */
#cmakedefine HAVE_SHL_LOAD

/* Define to 1 if you have the `sqrt' function. */
#cmakedefine HAVE_SQRT

/* Define to 1 if `stat' has the bug that it succeeds when given the
   zero-length file name argument. */
#cmakedefine HAVE_STAT_EMPTY_STRING_BUG

/* Define to 1 if you have the <stddef.h> header file. */
#cmakedefine HAVE_STDDEF_H

/* Define to 1 if you have the <stdint.h> header file. */
#cmakedefine HAVE_STDINT_H

/* Define to 1 if you have the <stdlib.h> header file. */
#cmakedefine HAVE_STDLIB_H

/* Define to 1 if you have the `strcasecmp' function. */
#cmakedefine HAVE_STRCASECMP

/* Define to 1 if you have the `strchr' function. */
#cmakedefine HAVE_STRCHR

/* Define to 1 if you have the `strdup' function. */
#cmakedefine HAVE_STRDUP

/* Define to 1 if you have the `strerror' function. */
#cmakedefine HAVE_STRERROR

/* Define to 1 if you have the <strings.h> header file. */
#cmakedefine HAVE_STRINGS_H

/* Define to 1 if you have the <string.h> header file. */
#cmakedefine HAVE_STRING_H

/* Define to 1 if you have the `strlcat' function. */
#cmakedefine HAVE_STRLCAT

/* Define to 1 if you have the `strlcpy' function. */
#cmakedefine HAVE_STRLCPY

/* Define to 1 if you have the `strncasecmp' function. */
#cmakedefine HAVE_STRNCASECMP

/* Define to 1 if you have the `strrchr' function. */
#cmakedefine HAVE_STRRCHR

/* Define to 1 if you have the `strspn' function. */
#cmakedefine HAVE_STRSPN

/* Define to 1 if you have the `strstr' function. */
#cmakedefine HAVE_STRSTR

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_DIR_H

/* Define to 1 if you have the <sys/dl.h> header file. */
#cmakedefine HAVE_SYS_DL_H

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
#cmakedefine HAVE_SYS_NDIR_H

/* Define to 1 if you have the <sys/socket.h> header file. */
#cmakedefine HAVE_SYS_SOCKET_H

/* Define to 1 if you have the <sys/stat.h> header file. */
#cmakedefine HAVE_SYS_STAT_H

/* Define to 1 if you have the <sys/types.h> header file. */
#cmakedefine HAVE_SYS_TYPES_H

/* Define to 1 if you have the <unistd.h> header file. */
#cmakedefine HAVE_UNISTD_H

/* Define to 1 if you have the `vprintf' function. */
#cmakedefine HAVE_VPRINTF

/* This value is set to 1 to indicate that the system argz facility works */
#cmakedefine HAVE_WORKING_ARGZ

/* HOPS package rev number */
#cmakedefine HOPS_REVISION

/* Haystack SVN rev number */
#cmakedefine HOPS_SVN_REV

/* Define to 1 if `lstat' dereferences a symlink specified with a trailing
   slash. */
#cmakedefine LSTAT_FOLLOWS_SLASHED_SYMLINK

/* Define if the OS needs help to load dependent libraries for dlopen(). */
#cmakedefine LTDL_DLOPEN_DEPLIBS

/* Define to the system default library search path. */
#cmakedefine LT_DLSEARCH_PATH

/* The archive extension */
#cmakedefine LT_LIBEXT

/* The archive prefix */
#cmakedefine LT_LIBPREFIX

/* Define to the extension used for runtime loadable modules, say, ".so". */
#cmakedefine LT_MODULE_EXT

/* Define to the name of the environment variable that determines the run-time
   module search path. */
#cmakedefine LT_MODULE_PATH_VAR

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#cmakedefine LT_OBJDIR

/* Define to the shared library suffix, say, ".dylib". */
#cmakedefine LT_SHARED_EXT

/* Define if dlsym() requires a leading underscore in symbol names. */
#cmakedefine NEED_USCORE

/* Name of package */
#cmakedefine PACKAGE

/* Define to the address where bug reports for this package should be sent. */
#cmakedefine PACKAGE_BUGREPORT

/* Define to the full name of this package. */
#cmakedefine PACKAGE_NAME

/* Define to the full name and version of this package. */
#cmakedefine PACKAGE_STRING

/* Define to the one symbol short name of this package. */
#cmakedefine PACKAGE_TARNAME

/* Define to the home page for this package. */
#cmakedefine PACKAGE_URL

/* Define to the version of this package. */
#define PACKAGE_VERSION @HOPS_SVN_REV_VALUE@


/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at runtime.
	STACK_DIRECTION > 0 => grows toward higher addresses
	STACK_DIRECTION < 0 => grows toward lower addresses
	STACK_DIRECTION = 0 => direction of growth unknown */
#cmakedefine STACK_DIRECTION

/* Define to 1 if you have the ANSI C header files. */
#cmakedefine STDC_HEADERS

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
#cmakedefine TM_IN_SYS_TIME

/* Version number of package */
#cmakedefine VERSION

/* Define to 1 if the X Window System is missing or not being used. */
#cmakedefine X_DISPLAY_MISSING

/******************************************************************************/

#endif
