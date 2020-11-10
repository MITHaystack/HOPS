#!/bin/bash
#
# helper script to generate manual pages
# it is invoked by automake with
#   $(top_srcdir)/m4/help2man.sh $(HELP2MAN) 1 $@ ./program $<
#
# You can add verbose to the environment to get some commentary.
#
[ -n "$verbose" ] && verb=true || verb=false
h2m=$1
man=$2
exe=$3
dep=$4
fail=false
[ $# -ge 4 ] || cat <<EOF

Usage: $0 help2man manpage executable dependency

    If help2man is not available (help2man = false) a dummy page is created.
    The manpage argument is the name of the page to produce and dependency is
    the first of any Makefile dependencies (other are ignored).  All these
    arguments are required.

    The path of the dependency is used to find additional information, such
    as h2m files or hand-crafted man pages.

    FIXME: canonical rules for what is where....

    This script always produces a manpage, useful or not.
EOF
[ $# -ge 4 ] || { echo not enough arguments; fail=true; }

[ -x "$h2m" ] || { echo no help2man executable "'$h2m'" ; fail=true; }
[ -n "$man" ] || { echo no manpage "'$man'" to create ; exit 0; }
[ -x "$exe" ] || { echo no executable "'$exe'" to run ; fail=true; }
[ -f "$dep" ] || { echo no dependency "'$dep'" to examine ; fail=true; }
$verb && echo fail is "'$fail'"

# work out section from manpage name provided
sec=`expr "$man" : '.*\.\([0-9].*\)'`
[ -z "$sec" ] && sec=1
tool=`expr "$man" : '\([^.]*\)\..*'`

CMD=DUMMY
des='dunno'
src='MIT Haystack Observatory'
# give up gracefully in failing cases
$fail && cat > ${man-/dev/null} <<EOF
.TH $CMD "$sec" "November 2020" "$src" "User Commands"
.SH NAME
$tool \- $des
.SH SYNOPSIS
.B $tool
[\fI\,options\/\fR]
.SH DESCRIPTION
where the options are:
.TP
\fB\-v\fR
which is not a real option since this is only a $des page.
EOF
$fail && exit 0

# otherwise (for a starter) run help2man
# for included files, [section] can be any of these:
#    NAME
#    SYNOPSIS
#    DESCRIPTION
#    OPTIONS
#    _other_
#    ENVIRONMENT
#    FILES
#    EXAMPLES
#    AUTHOR
# /pattern/ is PERL re syntax.

# the dependency is likely something such as $(srcdir)/src/foo.c
codedir=`dirname $dep`
[ -d "$codedir" ] && srcdir=`dirname $codedir`
[ -d "$srcdir/doc" ] && docdir=$srcdir/doc || docdir=no-such-dir

des=`$exe --description`
[ -f "$docdir/$tool.h2m" ] && inc=$docdir/$tool.h2m || inc=no-such-file

# this should cover everything, with the proviso that an executable
# always needs to be exist in order to generate the basic help page.
$verb && echo \
$h2m \"$exe\" -I \"$inc\" -n \"$des\" -s \"$sec\" -S \"$src\" -N -l \> $man
$h2m  "$exe"  -I  "$inc"  -n  "$des"  -s  "$sec"  -S  "$src"  -N -l  > $man

#
# eof
#
