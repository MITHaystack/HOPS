#!/bin/sh
#
# check script 2491
#
# This is a very old dataset captured by kad back in '94,
# in fact it is the oldest data captured, perhaps... Last seen on
# gemini:/var/ftp/pub/hops/oldhops/hops/sample_data/2491/363-200000
#

# set final exit status as an ERROR in case you forget to set it
passfail=99
# setups for test $something; exit=echo to disable exits 1..4
something=2491
[ -x "$MHO_REGRESSION_DATA/switches/test_envchk.sh" ] &&
    . "$MHO_REGRESSION_DATA/switches/test_envchk.sh"

# declare the tarballs that are needed and make those arrangements
tarballs='2491'
# declare the (built) executables that might not be in your path
executables='fplot'
# finally, acquire a list of directories that may need tidying
nukables=''
[ -n "$MHO_REGRESSION_CONFIG" ] && . $MHO_REGRESSION_CONFIG ||
    . "$MHO_REGRESSION_DATA/switches/test_config.sh"
[ -n "$MHO_REGRESSION_REQ" ] || { echo requirement not set ; exit 99; }

# first check that everything needed is actually present
data=$MHO_REGRESSION_DATA/ff_testdata/2491
[ -d "$data" ] || { echo data not present when it should be ; exit 5; }
[ -f "$data/ff_control.2491" ] || { echo config file missing ; exit 6; }

# FIXME: these lines should go away eventually
export DEF_CONTROL=/dev/null
export TEXT=$abs_top_srcdir/source/c_src/vex/text

# since we rely on this for our test, make sure it is generated
rm -f ff-2491-?-??.ps

# second execute some tests and set $passfail appropriately
$fplot -p fplot-2491-A-%02d.ps $data/363-200000/??.*jznfbg
$fplot -p fplot-2491-B-%02d.ps $data/363-200000/??.*jznfqu
$fplot -p fplot-2491-C-%02d.ps $data/363-200000/??.*jzqvrk

pscount=`ls -1 ff-2491-?-??.ps | wc -l`
[ "$pscount" -eq 46 ] && passfail=0 || passfail=7

eval set -- $nukables ; for dir ; do echo Nuking $dir ; rm -rf $dir ; done
[ "$MHO_REGRESSION_REQ" = ok ] || echo $MHO_REGRESSION_REQ $passfail
exit $passfail
#
# eof
#
