#!/bin/bash
#
# Script to (re)make tarballs of portions of SVN that are
# not part of the distro and perhaps should be saved for
# spelunkers into the history of HOPS at Haystack....
#
[ -z "$save" ] && save=save
[ $# -eq 0 ] && {
    echo Usage: $0 names
    echo This script makes tarballs of directories found
    echo in \$HOPS_ROOT/trunk and places them into the
    echo \$MHO_REGRESSION_DATA/tarballs/legacy area.
    echo The special name 'help' provides a list of tars,
    echo and the special name 'all' does all of them.
    echo
    echo If creating a tarball would overwrite some previously
    echo captured tarball, a copy will be retained with a 'save'
    echo inserted into the name.  See also legacy_nuke.sh.
    echo You can set 'save' in the environment for some other
    echo save name, e.g. a HOPS version number.
    exit 1
}
[ -n "$MHO_REGRESSION_DATA" ] || {
    echo You should set MHO_REGRESSION_DATA to the parent
    echo directory of the data hierarchy where the tarball
    echo will be placed.
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
# existing tarballs will be renamed as $name.$save.gz
# so you have one shot at fixing mistakes.
#
# when adding new data sets, use short names, there is plenty of
# space in the help here or in a <tar-name>.readme.txt captured in
# the source area

# these are the things in "all"
tars="
corr
mk4m
ompi
"

# this is what they contain
thelp="This script manages these legacy tarballs:
misc    -- a (small) test tarball captured in the source area for testing
corr    -- the original /correlator (HOPS 2 era) as copied to SVN
mk4m    -- various migration scripts associated with migration to SVN
ompi    -- some scripts for testing the software correlator with OpenMPI

The script legacy_nuke.sh can be used to wipe old backup copies of
previously generated legacy tarballs.  The default is to remake all
tarballs.  When tarballs are made, the old copies are saved as
<name>.\$save.gz where \$save is taken from the environment and defaults
to save.

Recommended practice might be to save tarballs at every version bump.
"

[ "$1" = all ] && set -- $tars

# some abbreviations to make the case statement below more compact
ffd=data/ff_testdata/testdata
aed=data/ae_testdata/testdata

for t
do
    # set src (found in $trk) and tarball name (usually $src)
    # this list must be synchronized with boostrap/legacy_unpack.sh
    case $t in
    help)   echo "$thelp"       ; exit 0                            ;;
    misc)   src=misc            ; dir=$trk/$ffd         ; name=$src ;;
    corr)   src=correlator      ; dir=$trk              ; name=$src ;;
    mk4m)   src=mk4-migrate     ; dir=$trk              ; name=$src ;;
    ompi)   src=ompi            ; dir=$trk              ; name=$src ;;
    *) echo what is \'$t\' \? it is not defined...      ; exit 4    ;;
    esac
    tarball=$dest/$name.tar.gz
    listing=$dest/$name.tvf.gz
    [ -f $tarball ] &&
        mv $tarball $dest/$name.$save.gz &&
        echo saving $t as $name.$save.gz
    pushd $dir
    tar zcf $tarball ./$src
    tar ztvf $tarball | gzip > $listing
    ls -l $tarball $dest/$name.$save.gz
    popd
done

#
# eof
#
