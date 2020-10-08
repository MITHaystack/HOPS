dnl
dnl Some useful configure rules.
dnl
dnl AC_MHO_INIT([comment])
dnl -----------------------
AC_DEFUN([AC_MHO_INIT], [dnl
rm -f config.summary ; echo [$1] > config.summary
])

dnl
dnl AC_MHO_NOTICE([normal notice arguments])
dnl-----------------------------------------
AC_DEFUN([AC_MHO_NOTICE], [dnl
echo "[$1]" >> config.summary
AC_MSG_NOTICE([$1])dnl
])

dnl
dnl AC_MHO_BANNER([comment])
dnl-------------------------
AC_DEFUN([AC_MHO_BANNER], [dnl
    echo ''
    echo '  '[$1]
    echo ''
    grep '%%%' config.summary | sed 's/^.../    /'
    echo ''
])

dnl
dnl AC_MHO_LIBDEP([name],[dir],[lib])
dnl -------------------------
AC_DEFUN([AC_MHO_LIBDEP], [dnl
$1_LIB=['-L$(top_builddir)/$2 -l$3']
$1_DEP=['$(top_builddir)/$2/lib$3.a']
AC_SUBST($1_LIB)
AC_SUBST($1_DEP)
])

dnl
dnl eof
dnl
