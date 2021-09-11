#!/bin/bash
#
# A brutal search
#
[ -n ${HOPS3_SRC_DIR} ] && [ -n ${HOPS4_SRC_DIR} ] || {
    echo "Need to set HOPS3_SRC_DIR and HOPS4_SRC_DIR"
    exit 1;
}
echo searching $HOPS3_SRC_DIR
echo matchesin $HOPS4_SRC_DIR

[ "$#" -eq 1 ] && subdir=/$1 &&
    echo restricted to $HOPS3_SRC_DIR$subdir || subdir=''

for f in `find $HOPS3_SRC_DIR$subdir -name .svn -prune -o -type f`
do
    case $f in
    *CMake*)         continue;;
    *Makefile.*)     continue;;
    *readme*)        continue;;
    *README*)        continue;;
    *cmake*)        continue;;
    *auto*)         continue;;
    *__init__*)     continue;;
    *setup.py*)     continue;;
    esac

    case $dn in
    *attic*)  continue;;
    *swc*)    continue;;
    esac

    bf=`basename $f`
    nf=`find $HOPS4_SRC_DIR -name $bf`
    [ -z "$nf" ] && continue
    for nnn in $nf
    do
        [ -d $f -o -d $nnn ] && continue
        cmp $f $nnn >/dev/null ||
        echo $f $nnn
    done
done |\
sed -e "s+$HOPS3_SRC_DIR+\$H3+" -e "s+$HOPS4_SRC_DIR+\$H4+"

#
# eof
#
