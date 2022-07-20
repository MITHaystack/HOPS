#!/bin/sh
#
# $Id: check_itime.sh 2707 2008-09-04 19:24:27Z gbc $
#
# simple script to look for self inconsistency of hops_time.
# It is not as thorough as perhaps it could be made to be.
# A few cases are skipped due to precision errors.
#
# Derived from previous check_itime and modifed for changed arguments.
#
export TZ=UTC
export HOPS_JULIAN_EPOCH=UTC
yr=`date +%Y`
hve=`expr 2 \* \( $yr - 2000 \)`
export HOPS_VDIF_EPOCH=$hve

verb=true
veryverb=true
[ -z "$testverb" ] && testverb=-1
[ "$testverb" -gt 0 ] && verb=true
[ "$testverb" -gt 1 ] && veryverb=true

rm -f queries.*

# The linux weenies strike again
# in newer releases:
#   Sat 14 Dec 2019 09:29:00 PM UTC
# versus the older:
#   Sat Dec 14 21:36:08 UTC 2019

# Now is an even second.
#[ -z "$now" ] && now=`date -u`
uyr=`date -u +"UTC %Y"`
[ -z "$now" ] && now=`./hops_time -q D +0 | cut -d. -f1`".0 $uyr"
gps=`./hops_time -q G "$now" | cut -d. -f1`.0
#ori=`./hops_time -q G -- "-1d" | cut -d. -f1`.0
$verb && echo Using GPS time $gps from $now

$verb && echo ./hops_time -v -e -a \"$gps\" ... \> queries.orig
# greps missing...
./hops_time -v -e -a "$gps" |\
grep -v ' H ' |\
grep -v ' HOPS ' |\
grep -v ' F ' |\
grep -v ' File' |\
grep -v ' R ' |\
cat - > queries.orig
# even 4-mins required for the HOPS forms
# even minutes required for the File forms,
# it is unclear what dance is required for -R since
# the test execution time affects the answers...

# i.e. cmd is each output format
for cmd in `cut -c1-11 queries.orig | tr -d ' '`
do
    cat queries.orig | cut -d= -f3 | cut -c2- |\
    ./hops_time -q $cmd STDIN > queries.$cmd
done

$veryverb && {
    for q in queries.*
    do
	[ "${q}" = queries.orig ] && continue
	sort $q | uniq -c |\
	awk '{printf("%20s %s\n","'$q'",$0);}'
    done
}

eval `./hops_time -q G queries.? |\
	awk '{x=$1-'"$gps"';c++;}(x>-.001)&&(x<.001){o++}\
	     END{print "ep=" int(10000.-10000.*o/c), "tc=" c;}'`
$verb && echo 100x error_percentage is $ep based on $tc time evaluations

[ "$ep" -eq 0 ] && exit 0 || exit 1

#
# eof
#
