#!/bin/bash
#
# A brutal search
#
[ -n ${HOPS3_SRC_DIR} ] && [ -n ${HOPS4_SRC_DIR} ] || {
    echo "Need to set HOPS3_SRC_DIR and HOPS4_SRC_DIR"
    exit 1;
}
cat <<EOF

searching $HOPS3_SRC_DIR
matchesin $HOPS4_SRC_DIR

NB--skipping files that are known to be found in many places
(build machinery, &c.) and this including some sources with
commonly used names:

    fplot.c parse_cmdline.c read_data.c sort_data.c sorter.c update.c

Also skipped are some HOPS3 applications and areas not retained.
Anything that turns up on the list is probably something to address.

EOF

[ "$#" -eq 1 ] && subdir=/$1 &&
    echo restricted to $HOPS3_SRC_DIR$subdir || subdir=''

for f3 in `find $HOPS3_SRC_DIR$subdir -name .svn -prune -o -type f`
do
    case $f3 in
    *CMake*)            continue;;
    *Makefile.*)        continue;;
    *configure*)        continue;;
    *aclocal*)          continue;;
    *readme*)           continue;;
    *README*)           continue;;
    *cmake*)            continue;;
    *auto*)             continue;;
    *__init__*)         continue;;
    *setup.py*)         continue;;
    *import*sh)         continue;;
    *tex)               continue;;
    *.hh)               continue;;
    *.cc)               continue;;
    *hops_config.h*)    continue;;
    *pypath.py)         continue;;
    *.gitignore)        continue;;
    *hops.bash*)        continue;;
    *chk_env*)          continue;;

    */hfold/*)          continue;;
    */coterp/*)         continue;;

    *fplot.c)           continue;;
    *parse_cmdline.c)   continue;;
    *read_data.c)       continue;;
    *sort_data.c)       continue;;
    *sorter.c)          continue;;
    *update.c)          continue;;
    esac

    case $dn in
    *attic*)  continue;;
    *swc*)    continue;;
    esac

    bf3=`basename $f3`
    nf3=`find $HOPS4_SRC_DIR -name $bf3`
    [ -z "$nf3" ] && continue
    for bf4 in $nf3
    do
        # skip directories
        [ -d $f3 -o -d $bf4 ] && continue
        nn=`diff $f3 $bf4 | wc -l`
        cmp $f3 $bf4 >/dev/null || {
            echo $f3 $bf4 \# diff $nn lines; }
    done
done |\
sed -e "s+$HOPS3_SRC_DIR+\$H3+" -e "s+$HOPS4_SRC_DIR+\$H4+"
echo found $count files to worry about

echo
echo H3=$HOPS3_SRC_DIR H4=$HOPS4_SRC_DIR

# not a test
exit 0
#
# eof
#
