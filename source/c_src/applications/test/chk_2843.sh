#!/bin/sh
#
# check script 2843
#
# Exercises the basic fringe search and reduction, and verifies that the SNR
# (parsed from the postscript file) is within expected bounds.
# This is a Mk4/hdw dataset.
#

# set final exit status as an ERROR in case you forget to set it
passfail=99
# setups for test $something; exit=echo to disable exits 1..4
something=2843
[ -x "$MHO_REGRESSION_DATA/switches/test_envchk.sh" ] &&
    . "$MHO_REGRESSION_DATA/switches/test_envchk.sh"

# declare the tarballs that are needed and make those arrangements
tarballs='2843'
# declare the (built) executables that might not be in your path
executables='fourfit'
# finally, acquire a list of directories that may need tidying
nukables='ff_2843.ps'
# now run test_config (or an alternative config script) to unpack the data
[ -n "$MHO_REGRESSION_CONFIG" ] && . $MHO_REGRESSION_CONFIG ||
    . "$MHO_REGRESSION_DATA/switches/test_config.sh"
[ -n "$MHO_REGRESSION_REQ" ] || { echo requirement not set ; exit 99; }

# FIXME: this is just for some debugging of test_config.sh
echo verb: $verb
echo data: $MHO_REGRESSION_DATA
echo config: $MHO_REGRESSION_CONFIG
echo extract: $MHO_REGRESSION_EXTRACT
echo tidy: $MHO_REGRESSION_TIDY
echo nukables: $nukables

# first check that everything needed is actually present
data=$MHO_REGRESSION_DATA/ff_testdata/2843
[ -d $data ] || { echo unpacked data directory is not present; exit 5;}
[ -f "$data/321-1701_0552+398/0552+398.oifhak" ] || { echo correlator file missing; exit 6; }

# since we rely on this for our test, make sure it is generated
rm -f ff-2836.ps

# FIXME: these lines should go away eventually
export DEF_CONTROL=/dev/null
export TEXT=$abs_top_srcdir/source/c_src/vex/text

# second execute some tests and set $passfail appropriately
# the $fourfit variable is set by lookup.sh, which is run in test_config.sh
# note that we set teh start time, this is important to match the SNR
$verb && echo \
$fourfit -t -d diskfile:ff-2843.ps -b AI:S \\ && echo \
    $data/321-1701_0552+398/0552+398.oifhak \
    set start -3

$fourfit -t -d diskfile:ff-2843.ps -b AI:S \
    $data/321-1701_0552+398/0552+398.oifhak \
    set start -3

# does the output file exist?
[ -f ./ff-2843.ps ] || { echo ./ff-2843.ps missing && exit 7 ; }

# FIXME: grab amp as well

# pluck out line containing the snr and parse it
line=$(grep '7570 9653' ./ff-2843.ps)

IFS='()'
read a snr b <<<"$line"

# snr bounds
low=47.8
high=48.6
aok=$(echo "$snr>$low && $snr<$high" | bc)
$verb && echo aok is $aok and "$low < $snr < $high" is expected from: $line
[ "$aok" -gt 0 ] && passfail=0 || passfail=8

eval set -- $nukables ; for dir ; do echo Nuking $dir ; rm -rf $dir ; done
[ "$MHO_REGRESSION_REQ" = ok ] || echo $MHO_REGRESSION_REQ $passfail
exit $passfail
#
# eof
#
