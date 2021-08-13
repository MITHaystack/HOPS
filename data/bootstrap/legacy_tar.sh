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
    echo in "(HOPS3)" \$HOPS_ROOT/trunk and places them into the
    echo \$MHO_REGRESSION_DATA/tarballs/legacy area.
    echo The special name 'help' provides a list of tars,
    echo and the special name 'all' does all of them.
    echo
    echo If creating a tarball would overwrite some previously
    echo captured tarball, a copy will be retained with a 'save'
    echo inserted into the name.  See also legacy_nuke.sh.
    echo You can set 'save' in the environment for some other
    echo save name, e.g. a HOPS version number.
    echo
    echo New HOPS4 data is to be managed in some TBD script.
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
    echo \$MHO_REGRESSION_DATA/tarballs/legacy $dest is missing, go fix that
    exit 4
}

trk=$HOPS_ROOT/trunk
[ -d "$trk" ] || {
    echo \$HOPS_ROOT/trunk $trk is missing, go fix that
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
thelp="This script manages legacy tarballs; -s- refers to source
areas and -d- refers to data areas

corr    -s- the original /correlator (HOPS 2 era) as copied to SVN
mk4m    -s- various migration scripts associated with migration to SVN
ompi    -s- some scripts for testing the software correlator with OpenMPI

misc    -d- a (small) test tarball captured in the source area for testing
fftest  -d- there are individual HOPS experiments that have been captured.
           use fftest to get a listing.  They are tarballed individually.

v2xsrc  -s- vex2xml sources
v2xtst  -d- vex2xml testcases

The script legacy_nuke.sh can be used to wipe old backup copies of
previously generated legacy tarballs.  The default is to remake all
tarballs.  When tarballs are made, the old copies are saved as
<name>.\$save.gz where \$save is taken from the environment and defaults
to save.

Recommended practice might be to save tarballs at every version bump.
"
fftest="Tarballs for parts of the following experiments are supported:
2491    -d- scan 363-200000
2611    -d- scan 062-094600
2836    -d- scan scan001 (one of the basic tests)
2843    -d- scan 321-1701_0552+398 (one of the basic tests)
2849    -d- scan 297-0311_RCAS
2912    -d- scan 253-1907
3064    -d- scan 113-0256
3262    -d- scan 049-0600
3365    -d- scans 094-0644_HIGH and 094-0644_LOW (fourmer test)
3372    -d- scan 193-1757 (one of the basic tests)
3413    -d- scan 278-1758 (one of the basic tests)
3562    -d- scan 141-0002
3571    -d- scan 244-1717
"

[ "$1" = all ] && set -- $tars

# some abbreviations to make the case statement below more compact
ffd=data/ff_testdata/testdata
aed=data/ae_testdata

for t
do
    # set src (found in $trk) and tarball name (usually $src)
    # this list must be synchronized with boostrap/legacy_unpack.sh
    name="$t" exc='' src='' dir=''
    case $t in
    \#*)        continue                                                ;;
    *help)      echo "$thelp"       ; exit 0                            ;;
    corr)       src=correlator      ; dir=$trk                          ;;
    mk4m)       src=mk4-migrate     ; dir=$trk                          ;;
    ompi)       src=ompi            ; dir=$trk                          ;;
    # ff_testdata subdirs
    fftest)     echo "$fftest"      ; exit 0                            ;;
    misc)       src=misc            ; dir=$trk/$ffd                     ;;
    2491)       src=$t              ; dir=$trk/$ffd                     ;;
    2611)       src=$t              ; dir=$trk/$ffd                     ;;
    2836)       src=$t              ; dir=$trk/$ffd                     ;;
    2843)       src=$t              ; dir=$trk/$ffd                     ;;
    2849)       src=$t              ; dir=$trk/$ffd                     ;;
    2912)       src=$t              ; dir=$trk/$ffd                     ;;
    3064)       src=$t              ; dir=$trk/$ffd                     ;;
    3262)       src=$t              ; dir=$trk/$ffd                     ;;
    3365)       src=$t              ; dir=$trk/$ffd                     ;;
    3372)       src=$t              ; dir=$trk/$ffd                     ;;
    3413)       src=$t              ; dir=$trk/$ffd                     ;;
    3562)       src=$t              ; dir=$trk/$ffd                     ;;
    3571)       src=$t              ; dir=$trk/$ffd                     ;;
    average)    src=$t              ; dir=$trk/$ffd                     ;;
    # ae_testdata subdirs
    aetest)     src=testdata        ; dir=$trk/$aed                     ;;

    # vex2xml
    v2xsrc)     src=vex2xml         ; dir=$trk      ; exc=testcases     ;;
    v2xtst)     src=vex2xml         ; dir=$trk      ; exc="??? ???4"    ;;

    *) echo what is \'$t\' \? it is not defined...      ; exit 4    ;;
    esac

set -x
    # ok, now do the work...
    [ -z "$name" -o -z "$dir" ] && { echo $t undefined ; continue ; }
    tarball=$dest/$name.tar.gz
    listing=$dest/$name.tvf.gz
    [ -f $tarball ] &&
        mv $tarball $dest/$name.$save.gz &&
        echo saving $t as $name.$save.gz
    pushd $dir
    excludes=''
    [ -z "$exc" ] || {
        for e in $exc ; do excludes="--exclude=$e $excludes" ; done
    }
    tar $excludes zcf  $tarball ./$src
    tar $excludes ztvf $tarball | gzip > $listing
    [ -f $dest/$name.$save.gz ] && ls -1 $dest/$name.$save.gz
    ls -l $tarball
set +x
    popd
done

#
# eof
#
