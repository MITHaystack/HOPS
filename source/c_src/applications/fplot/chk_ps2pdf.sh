#!/bin/sh
#
# check script chk_ps2pdf.sh
#
# Check that fplot dependencies are going to be satisfied
#

# standard setup follows; comment out what is not needed
[ -z "$testverb" ] && testverb=0
verb=false ; [ "$testverb" -gt 0 ] && verb=true
very=false ; [ "$testverb" -gt 1 ] && very=true && verb=true
passfail=99
something=chk_ps2pdf.sh
[ -z "$srcdir" -o -d "$srcdir" ] || {
    echo srcdir "$srcdir" not set correctly; exit 1; }
#[ -z "$abs_top_srcdir" -o -d "$abs_top_srcdir" ] || {
#    echo abs_top_srcdir "$abs_top_srcdir" not set correctly; exit 2; }
#[ -z "$abs_top_builddir" -o -d "$abs_top_builddir" ] || {
#    echo abs_top_builddir "$abs_top_builddir" not set correctly; exit 3; }
[ $# -gt 0 ] && { echo $something takes no arguments; exit 4; }
#[ -z "$MHO_REGRESSION_DATA" ] && { echo MHO_REGRESSION_DATA not set; exit 77; }
#[ -x "$MHO_REGRESSION_DATA/switches/test_config.sh" ] || {
#    echo "$MHO_REGRESSION_DATA/switches/test_config.sh" not found ; exit 99; }

# declare the tarballs that are needed and make those arrangements
#tarballs='FIXME'
# declare the (built) executables that might not be in your path
#executables='FIXME'
# finally, acquire a list of directories that may need tidying
#nukables=''
#[ -n "$MHO_REGRESSION_CONFIG" ] && . $MHO_REGRESSION_CONFIG ||
#    . "$MHO_REGRESSION_DATA/switches/test_config.sh"
#[ -n "$MHO_REGRESSION_REQ" ] || { echo requirement not set ; exit 99; }

# first check that everything needed is actually present
[ -n $ps2pdf ] || { echo ps2pdf not in the env; ps2pdf=`type -p false`; }
[ "$ps2pdf" = "@PS2PDF@" ] && { echo fix configuration ; exit 77 ; }
[ -x $ps2pdf ] || { echo ps2pdf at $ps2pdf is not executable; exit 2; }
echo the HOPS3 code linkes to X11 and PGPLOT ... is that necesary, too?
passfail=77

#eval set -- $nukables ; for dir ; do echo Nuking $dir ; rm -rf $dir ; done
#[ "$MHO_REGRESSION_REQ" = ok ] || echo $MHO_REGRESSION_REQ $passfail
exit $passfail
#
# eof
#
