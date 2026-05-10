#!/bin/bash
#
# A script to do the alist, fringex, average, cohfit dance in
# order to estimate the coherence of some fringe data.
#
# The presumption is that correlator output and fringes exist
# in some directory and one wants to process one or more fringes.
#
USAGE="Usage: $0 [key=value options]

where the options are:

    alist=<file>    input alist (optional)
    cdir=<dir>      correlator experiment directory or part (required)
    wdir=<dir>      directory in which to work (defaults to current directory)
    expn=<int>      HOPS experiment number (column 8 of alists)
    iarg=<...>      the fringex -i argument, see below (defaults to all)
    tag=<string>    some token to put in working filenames (required)
    exam=<string>   tag-exam used for -e argument of cohfit products
    grep=<string>   grep -E <string> will be invoked on the alist
    rnc=RxC         the -g option of cohfit
    msglev=<int>    the -m option of cohfit
    labs=true|false show the additional plot labels
    nuke=true|false nuke tag/exam names if found (true by default)
    verb=true|false be chatty or not (true by default)
    nmsg=true|false if true, msg output goes to tag.msgs (true by default)
    old=true|false  if true use cofit, not cohfit

The fringex segmentation should either be the 'all' argument which
results in 1,2,4,8... second accumulation periods, an explicit list
of up to 50 (integer) durations (in seconds) or a min:max:step construct.

If no alist is provided, one is generated from the 'cdir' which is linked
to the current directory as 'expn' and the DATADIR set appropriately.
Then the fringex, average and cohfit are executed in order.

The cdir must be specified as from the wdir (if relative).
The expn defaults to 16383 (the ILLEGAL value) and is used only to create
a symbolic link to the correlator experiment directory (or scan therein).
"
while [ $# -gt 0 ] ; do case $1 in
--help)     echo "$USAGE" ; exit 0 ;;
--version)  echo "no version information" ; exit 0 ;;
alist=*)    eval "$1" ;;
cdir=*)     eval "$1" ;;
wdir=*)     eval "$1" ;;
expn=*)     eval "$1" ;;
iarg=*)     eval "$1" ;;
tag=*)      eval "$1" ;;
rnc=*)      eval "$1" ;;
msglev=*)   eval "$1" ;;
exam=*)     eval "$1" ;;
grep=*)     eval "$1" ;;
labs=*)     eval "$1" ;;
nuke=*)     eval "$1" ;;
verb=*)     eval "$1" ;;
old=*)      eval "$1" ;;
nmsg=*)     eval "$1" ;;
*)          echo Erroneous argument "$1" ; echo "$USAGE" ; exit 1 ;;
esac ; shift ; done

# zeroth, sine qua non
alistexec=`type -p alist`
[ -n "$alistexec" -a -x "$alistexec" ] || {
    echo no alist executable--set up HOPS ; exit 1 ;
}
fringex=`type -p fringex`
[ -n "$fringex" -a -x "$fringex" ] || {
    echo no fringex executable--set up HOPS ; exit 1 ;
}
average=`type -p average`
[ -n "$average" -a -x "$average" ] || {
    echo no average executable--set up HOPS ; exit 1 ;
}

# first the booleans
[ -z "$labs" ] && labs=false
[ "$labs" = "true" -o "$labs" = "false" ] || {
    echo labs must be true or false, not $labs
    exit 2
}
[ -z "$nuke" ] && nuke=true
[ "$nuke" = "true" -o "$nuke" = "false" ] || {
    echo nuke must be true or false, not $nuke
    exit 2
}
[ -z "$verb" ] && verb=true
[ "$verb" = "true" -o "$verb" = "false" ] || {
    echo verb must be true or false, not $verb
    exit 2
}
[ -z "$old" ] && old=false
[ "$old" = "true" -o "$old" = "false" ] || {
    echo old must be true or false, not $old
    exit 2
}
[ -z "$nmsg" ] && nmsg=true
[ "$nmsg" = "true" -o "$nmsg" = "false" ] || {
    echo nmsg must be true or false, not $nmsg
    exit 2
}

$old && cohfit=`type -p cofit` || cohfit=`type -p cohfit`
[ -n "$cohfit" -a -x "$cohfit" ] || {
    echo no cohfit executable--set up HOPS ; exit 3 ;
}

# next iarg (the tricky one)
[ -z "$iarg" ] && iarg='all'
min=0 max=0 step=0
[ "$iarg" = 'all' ] || {
  [ `expr "$iarg" : '.*,.*'` -gt 1 ] || {
    min=`expr "$iarg" : '\([^:]*\):[^:]*:[^:]*'`
    max=`expr "$iarg" : '[^:]*:\([^:]*\):[^:]*'`
    step=`expr "$iarg" : '[^:]*:[^:]*:\([^:]*\)'`
    # $verb && echo min=$min max=$max step=$step
    ii=$min
    iarg=$ii
    ii=$(($ii + $step))
    while [ $ii -le $max ] ; do
        iarg="$iarg,$ii"
        ii=$(($ii + $step))
    done
  }
}
# $verb && echo iarg=$iarg

# finally the pedestrian stuff
[ -z "$expn" ] && expn=16383
[ -z "$rnc" ] && dashg="" || dashg="-g $rnc"
[ -z "$msglev" ] && dashm="" || dashm="-m $msglev"
[ -z "$exam" -o $old = true ] && dashe="" || dashe="-e $tag-$exam"
[ -z "$tag" ] && {
    echo tag not set, this is a problem
    exit 4
}

# now start doing things
[ -z "$wdir" ] && wdir=`pwd`
[ -n "$wdir" ] && cd $wdir || {
    echo unable to change to work directory $wdir
    exit 5
}
export DATADIR=`pwd`

# remove planned outputs
[ -f $tag.cofit ] && {
    $nuke || { echo have $tag.cofit and nuke is false ; exit 6; }
}
[ -f $tag.cofit.txt ] && {
    $nuke || { echo have $tag.cofit.txt and nuke is false ; exit 6; }
}
[ -f $tag.coavg ] && {
    $nuke || { echo have $tag.coavg and nuke is false ; exit 6; }
}
[ -f $tag.fringex ] && {
    $nuke || { echo have $tag.fringex and nuke is false ; exit 6; }
}
[ -f $tag.alist ] && {
    $nuke || { echo have $tag.alist and nuke is false ; exit 6; }
}
[ -f $tag.tmp.alist ] && {
    $nuke || { echo have $tag.tmp.alist and nuke is false ; exit 6; }
}
[ -f $tag.msgs ] && {
    $nuke || { echo have $tag.msgs and nuke is false ; exit 6; }
}
rm -f $tag.alist $tag.tmp.alist $tag.fringex $tag.coavg $tag.cofit
rm -f $tag.cofit.txt $tag.cofit.ps $tag.cofit.pdf $tag-$exam.* $tag.msgs

# link the cdir in to cwd
[ -z "$cdir" ] && {
    echo no correlator experiment specified with cdir=
    exit 7
}
[ -d "$cdir" ] || {
    echo correlator experiment directory does not exist
    exit 7
}
$nuke && [ -h "$expn" ] && rm $expn
[ -h "$expn" ] && {
    echo Symlink $expn exists or nuke is false ; exit 7
}
welink=false
ln -s $cdir $expn && welink=true || {
    echo Unable to link $cdir as $expn ; exit 7
}

$nmsg && exec 2>$tag.msgs

# link or create the alist
[ -n "$alist" -a -f $alist ] && ln -s $alist $tag.alist || {
    $verb && echo \
    $alistexec -v6 -o $tag.alist $expn
    $alistexec -v6 -o $tag.alist $expn
    $verb && set -- `wc -l $tag.alist` && echo $1 lines in $2
}

# grep on the alist if required
[ -n "$grep" ] && {
    mv $tag.alist $tag.tmp.alist
    $verb && echo \
    grep -E "$grep" $tag.tmp.alist \> $tag.alist
    grep -E "$grep" $tag.tmp.alist  > $tag.alist
    $verb && set -- `wc -l $tag.alist` && echo $1 lines in $2
}

# now fringex
$verb && echo \
$fringex -i $iarg -r $tag.alist \> $tag.fringex
$fringex -i $iarg -r $tag.alist  > $tag.fringex
$verb && set -- `wc -l $tag.fringex` && echo $1 lines in $2

# now average
$verb && echo \
$average -c -o $tag.coavg \< $tag.fringex
$average -c -o $tag.coavg  < $tag.fringex
$verb && set -- `wc -l $tag.coavg` && echo $1 lines in $2

# finally cofit; the old code sprays printf chatter
$labs && dashl='-l' || dashl=''
$verb && echo \
$cohfit $dashg $dashe $dashl $dashm -d $tag.cofit.ps/vcps/pdf -o $tag.cofit \\ &&
    echo \< $tag.coavg \> $tag.cofit.txt
$cohfit $dashg $dashe $dashl $dashm -d $tag.cofit.ps/vcps/pdf -o $tag.cofit \
    < $tag.coavg > $tag.cofit.txt
$verb && set -- `wc -l $tag.cofit $tag.cofit.pdf 2>&-`
$verb && echo $1 lines in $2 && [ $# -ge 4 ] && echo $3 lines in $4
$verb && echo commentary in $tag.msgs

$welink && rm -f $expn
[ -s $tag.cofit.txt ] || rm $tag.cofit.txt
[ -s $tag.cofit.pdf ] && echo see `pwd`/$tag.cofit.pdf
#
# eof vim:nospell
#
