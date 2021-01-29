#!/bin/bash
#
# Script to (re)make tarballs of portions of SVN that are
# not part of the distro and perhaps should be saved for
# spelunkers into the history of HOPS at Haystack....
#
[ $# -eq 0 ] && {
    echo read the script for help
    exit 1
}
[ -n "$MHO_REGRESSION_DATA" ] || {
    echo You should set MHO_REGRESSION_DATA to the parent
    echo directory of the data hierarchy
    exit 2
}
[ -n "$HOPS_ROOT" ] || {
    echo You should set HOPS_ROOT to the parent
    echo directory of the source hierarchy
    echo files are expected at trunk/...
    exit 3
}

dest=$MHO_REGRESSION_DATA/tarballs/legacy
[ -d "$dest" ] || {
    echo $dest is missing, go fix that
    exit 4
}

trk=$HOPS_ROOT/trunk
[ -d "$trk" ] || {
    echo $trk is missing, go fix that
    exit 5
}

# list of portions that are preserved
# each portion has a $dir(ectory) in $HOPS_ROOT/trunk
# from which to generate a tar of as $name(.tar.gz)
#
# existing tarballs will be renamed as $name.save.gz
# so you have one shot at fixing mistakes

tars="
corr
"

[ "$1" = all ] || set -- $tars

for t in $tars
do
    # set source and dest
    case $t in
    corr)   src=correlator ; dir=$trk    ; name=$src ;;
    *) echo what is $t \?  ; exit 4                  ;;
    esac
    tarball=$dest/$name.tar.gz
    listing=$dest/$name.tvf.gz
    [ -f $tarball ] &&
        mv $tarball $dest/$name.save.gz &&
        echo saving $t as $name.save.gz
    pushd $dir
    tar zcf $tarball ./$src
    tar ztvf $tarball | gzip > $listing
    ls -l $tarball
    popd
done

#
# eof
#
