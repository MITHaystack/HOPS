#!/bin/sh
#
# $Id: check_itime.sh 2707 2008-09-04 19:24:27Z gbc $
#
# simple script to look for self inconsistency of ibex_time.
# not as thorough as it needs to be.
# skip the L and H cases as seconds of the minute are dropped.
#

verb=false
[ "$testverb" ] && verb=true

rm -f queries.*

now=`date -u`
gps=`./ibex_time -G "$now" | cut -d. -f1`
$verb && echo Using GPS time $gps

$verb && echo ./ibex_time -v -e -A \"$now\" \| grep -v '^-L' \> queries.orig
./ibex_time -v -e -A "$now" | grep -v '^-[LH]' > queries.orig

for cmd in `cut -c2 queries.orig`
do
	cat queries.orig | cut -d= -f2 | cut -c2- |\
	./ibex_time -R $cmd - > queries.$cmd
done

$verb && {
	for q in queries.?
	do
		echo "" ${q}: `sort $q | uniq -c`
	done
}

error_percent=`./ibex_time -R G queries.? |\
	awk '{x=$1-'"$gps"';c++;}(x>-.001)&&(x<.001){o++}\
	     END{print int(1000.-1000.*o/c);}'`
$verb && echo error_percent is $error_percent

#
# for the most part no loss of information with the transformations,
# but a few discard fractional seconds and rounding errors accumulate.
#
[ $# -eq 1 ] && ./ibex_time -R G queries.? |\
	awk '{x=$1-'"$gps"';c++;print x}' | sort -n | uniq -c

[ $# -eq 0 -a "$error_percent" -le 1 ] && {
	$verb || rm queries.? queries.orig
}

#
# there are 121 cases, so .1% is pretty good, considering....
#
[ "$error_percent" -le 1 ] || {
	echo Failure rate ${error_percent}%.
	./ibex_time -R G queries.? |\
	awk '{x=$1-'"$gps"';c++;print x}' | sort -n | uniq -c
}
[ "$error_percent" -le 1 ]

#
# eof
#
