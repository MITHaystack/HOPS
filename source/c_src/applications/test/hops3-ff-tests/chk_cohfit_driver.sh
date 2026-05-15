#!/bin/bash
#
# exercises alist/fringex/average/cohfit using cohfit_driver.sh
#
verb=false
[ -n "$testverb" ] && verb=true
[ -d "$srcdir" ] || { echo srcdir not set; exit 1; }
${HOPS_SETUP-'false'} || {
    echo sourcing chk_env.sh && . $srcdir/chk_env.sh ; }
echo PATH is $PATH
# export DATADIR=`pwd`

rdir="3769"
[ -d $srcdir/testdata/$rdir ] || {
    echo Missing $rdir data; exit 2;
}
td=$srcdir/testdata/$rdir
$verb && echo Using data in $td

driver=`type -p cohfit-driver.sh`
[ -n "$driver" -a -x "$driver" ] || {
    echo executable cohfit-driver.sh not found ; exit 3;
}

expectprods=46
montage=`type -p montage`
[ -n "$montage" -a -x "$montage" ] && havemonty=true || {
    echo have no montage program, will have two fewer products
    expectprods=44
    havemonty=false
}

# do the work in a sub-directory
rm -rf cohdrv
mkdir cohdrv
$verb && echo \
$driver expn=3769 cdir=../$td verb=$verb \\ && echo
    tag=w iarg=2:50:2 wdir=cohdrv exam=dets-%d.data
$driver expn=3769 cdir=../$td verb=$verb \
    tag=w iarg=2:50:2 wdir=cohdrv exam=dets-%d.data
status=$?

products=`ls -1 cohdrv 2>&- | wc -l`
[ "$products" -eq $expectprods ] || {
    echo missing products, have $products, want $expectprods ; exit 5;
}

$verb && echo output products directory cohdrv: && ls -l cohdrv && echo

data="cohdrv/w.alist cohdrv/w.fringex cohdrv/w.coavg cohdrv/w.cohfit"
lines=`wc -l $data | grep total | tr -s ' ' | cut -f2 -d' '`
[ "$lines" -eq 6061 ] || {
    echo missing alines, have $lines, want 6061 ; exit 6;
}

# only if have montage
$havemonty && {
    [ -f cohdrv/w-dets-4.montage.pdf -a -f cohdrv/w-dets-12.montage.pdf ] || {
        echo missing montage PDF files; exit 7;
    }
}

[ -f cohdrv/w-dets-4.summary.txt -a -f cohdrv/w-dets-12.summary.txt ] || {
    echo missing summary text files; exit 8;
}

echo "status 0 from the driver is $status"
exit $?
#
# eof
#
