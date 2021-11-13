#!/bin/bash
#
# Script to (re)make tarballs of portions of SVN that are
# not part of the distro and perhaps should be saved for
# spelunkers into the history of HOPS at Haystack....
#
[ -z "$save" ] && save=save
[ $# -eq 0 -o "$1" = '--help' ] && {
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

#
# list of all tar nicknames -- generated with 'all' as an argument
# via the 'tars' variable
#
# each portion has a $dir(ectory) in $HOPS_ROOT/trunk
# from which to generate a tar of as $name(.tar.gz)
#
# existing tarballs will be renamed as $name.$save.gz
# so you have one shot at fixing mistakes.
#
# when adding new data sets, use short names, there is plenty of
# space in the help here or in a <tar-name>.readme.txt captured in
# the source area to explain it.  A comment #.... (no spaces!) will
# be skipped and should refer to the source area of the HOPS3 SVN tree.
#
# keep the tars, the help and the case statement similarly ordered
#
fftest=" 2491 2611 2836 2843 2849 2912 3064 3262 "
fftest="$fftest 3365 3372 3413 3562 3571 3727 3756 3772"
tars="
corr    #correlator
mk4m    #mk4-migrate
ompi    #ompi
difx    #difx
mark5   #mark5
h3msrc  #misc-sources
h3mtst  #misc-vlb2data
readme  #top-level-files
swcmsc  #swc-misc
swcreg  #swc-regression
tarpub  #historical-public
tarprv  #historical-private

doc     #doc
h3help  #help
include #include
pphops3 #postproc-losers
pphops4 #postproc-winners
scripts #scripts
sublib  #sub

misc    #data/ff_testdata/testdata/misc
$fftest #data/ff_testdata/testdata/....
average #data/ff_testdata/testdata/average
aetest  #data/ae_testdata/testdata

v2xsrc  #vex2xml-sources
v2xtst  #vex2xml-testcases
"

# this is the help on what they contain
thelp="This script manages legacy tarballs; -s- refers to source
areas and -d- refers to data areas; -p- refers to areas that should
not be publically distributed.

corr    -s- the original /correlator (HOPS 2 era) as copied to SVN
mk4m    -s- various migration scripts associated with migration to SVN
ompi    -s- some scripts for testing the software correlator with OpenMPI
difx    -s- some utilities that could be given to DiFX eventually
mark5   -s- some code for working with Mark5 units
h3msrc  -s- misc codes for bit states, planning, and the original vlbi?
h3mtst  -d- test data (.m5b) data for testing vlbi2
readme  -s- various readme and other toplevel files
swcmsc  -p- scripts for Haystack correlator -- miscellaneous
swcreg  -p- regression test scripts for Haystack correlator
tarpub  -s- various public historical HOPS3 tarballs from the tarpit
tarprv  -p- various private historical HOPS3 tarballs from the tarpit

doc     -s- the original HOPS3 document directory
h3help  -s- the original HOPS3 help directory
include -s- the original HOPS3 include directory
pphops3 -p- various HOPS3 sources likley not propagating into HOPS4
pphops4 -p- various HOPS4 sources that are propagating into HOPS4
scripts -s- the original HOPS3 scripts directory
sublib  -s- the original HOPS3 sub (library) directory

misc    -d- a (small) test tarball captured in the source area for testing
fftest  -d- there are individual HOPS experiments that have been captured.
            Using fftest gives a listing.  They are tarballed individually
            using the associated 4-digit experiment names.
average -d- samples of the use of 'average'
aetest  -d- sample of aedit data

v2xsrc  -s- vex2xml sources
v2xtst  -d- vex2xml testcases

The script legacy_nuke.sh can be used to wipe old backup copies of
previously generated legacy tarballs.  The default is to remake all
tarballs.  When tarballs are made, the old copies are saved as
<name>.\$save.gz where \$save is taken from the environment and defaults
to save.

Recommended practice might be to save tarballs at every version bump.
"
ffhelp="Tarballs for parts of the following experiments are supported:
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
3727    -d- scan 026-1123 e20z26 ALMA-APEX test
3756    -d- scan 328-1800
3772    -d- scan No0001 of c211d
"

[ "$1" = all ] && set -- $tars

# some abbreviations to make the case statement below more compact
ffd='data/ff_testdata/testdata'
aed='data/ae_testdata'
vxc='doc src lib ???4'
vb2='misc/vlbi2/data'
reg='swc/scripts/regression'
pp3='attic bispec calamp coterp fearfit hfold pratio'
pp4='adump aedit alist average cofit fourfit fourmer'
pp4="$pp4 fplot fringex search snratio"

# this is a catch-all for random files
readmefiles="apt-packages.txt autogen.sh capture-log.py ChangeLog.txt
configure.ac Copyright Fink.txt hops.bash.in hops_config.h.in
MacPorts.txt pgplot-drivers.list README.difx.txt README.pgplot.txt
README.svn.txt README.txt.in README.vendor.txt attic/README.haystack.txt"
tarpubfiles=''
tarprvfiles=''

for t
do
    # set src (found in $trk) and tarball name (usually $src)
    # this list must be synchronized with boostrap/legacy_unpack.sh
    name="$t" exc='' src='' dir=''
    case $t in
    \#*)        continue                                            ;;

    # various sources not likely to be duplicated in HOPS4
    corr)       src=correlator  ; dir=$trk                          ;;
    mk4m)       src=mk4-migrate ; dir=$trk                          ;;
    ompi)       src=ompi        ; dir=$trk                          ;;
    difx)       src=difx        ; dir=$trk                          ;;
    mark5)      src=mark5       ; dir=$trk                          ;;
    h3msrc)     src=misc        ; dir=$trk      ; exc=data          ;;
    h3mtst)     src=$vb2        ; dir=$trk                          ;;
    readme)     src=src-unused  ; dir=$trk                          ;;
    swcmsc)     src=swc         ; dir=$trk      ; exc=regression    ;;
    swcreg)     src=$reg        ; dir=$trk                          ;;
    tarpub)     src=src-unused  ; dir=/swc/tarpit                   ;;
    tarprv)     src=src-unused  ; dir=/swc/tarpit                   ;;

    # sources that are re-worked in HOPS4
    doc)        src=doc         ; dir=$trk                          ;;
    h3help)     src=help        ; dir=$trk                          ;;
    include)    src=include     ; dir=$trk                          ;;
    pphops3)    src=postproc    ; dir=$trk      ; exc="$pp4"        ;;
    pphops4)    src=postproc    ; dir=$trk      ; exc="$pp3"        ;;
    scripts)    src=scripts     ; dir=$trk                          ;;
    sublib)     src=sub         ; dir=$trk                          ;;

    # found in the data directory
    misc)       src=misc        ; dir=$trk/$ffd                     ;;
    2491)       src=$t          ; dir=$trk/$ffd                     ;;
    2611)       src=$t          ; dir=$trk/$ffd                     ;;
    2836)       src=$t          ; dir=$trk/$ffd                     ;;
    2843)       src=$t          ; dir=$trk/$ffd                     ;;
    2849)       src=$t          ; dir=$trk/$ffd                     ;;
    2912)       src=$t          ; dir=$trk/$ffd                     ;;
    3064)       src=$t          ; dir=$trk/$ffd                     ;;
    3262)       src=$t          ; dir=$trk/$ffd                     ;;
    3365)       src=$t          ; dir=$trk/$ffd                     ;;
    3372)       src=$t          ; dir=$trk/$ffd                     ;;
    3413)       src=$t          ; dir=$trk/$ffd                     ;;
    3562)       src=$t          ; dir=$trk/$ffd                     ;;
    3571)       src=$t          ; dir=$trk/$ffd                     ;;
    3727)       src=$t          ; dir=$trk/$ffd                     ;;
    3756)       src=$t          ; dir=$trk/$ffd                     ;;
    3772)       src=$t          ; dir=$trk/$ffd                     ;;
    average)    src=$t          ; dir=$trk/$ffd                     ;;
    # ae_testdata subdirs
    aetest)     src=testdata    ; dir=$trk/$aed                     ;;

    # vex2xml
    v2xsrc)     src=vex2xml     ; dir=$trk      ; exc=testcases     ;;
    v2xtst)     src=vex2xml     ; dir=$trk      ; exc="$vxc"        ;;

    # help targets -- last due to wildcards
    fftest)     echo "$ffhelp"  ; exit 0                            ;;
    *help)      echo "$thelp"   ; exit 0                            ;;
    *) echo what is \'$t\' \? it is not defined...      ; exit 4    ;;
    esac

    # some checks -- continue to allow 'all' case to function
    [ -z "$name" -o -z "$dir" ] && { echo $t undefined ; continue ; }
    [ -d $dir ] || {  echo no $dir to work on, skipping...$name ; continue ; }

    # ok, now do the work...starting with setting variables
    tarball=$dest/$name.tar.gz
    listing=$dest/$name.tvf.gz
    excludes=''
    [ -z "$exc" ] || {
        for e in $exc ; do excludes="--exclude=$e $excludes" ; done
    }

    # save the tarball and listing
    [ -f $tarball ] &&
        mv $tarball $dest/$name.$save.tar.gz &&
        echo saving $t tarball as $name.$save.tar.gz
    [ -f $listing ] &&
        mv $listing $dest/$name.$save.tvf.gz &&
        echo saving $t listing as $name.$save.tvf.gz

    pushd $dir

    created=false
    # created by src dir
    [ -d "./$src" ] &&
        tar zcf  $tarball $excludes ./$src && created=true
    # special cases
    [ "$name" = 'readme' ] &&
        tar zcf  $tarball $readmefiles && created=true
    [ "$name" = 'tarpub' ] &&
        tar zcf  $tarball $readmefiles && created=true
    [ "$name" = 'tarprv' ] &&
        tar zcf  $tarball $readmefiles && created=true
    $created && [ -f $tarball ] && {
        tar ztvf $tarball $excludes | gzip > $listing
        [ -f $dest/$name.$save.gz ] && ls -1 $dest/$name.$save.gz
        ls -l $tarball
        ls -l $listing
    } || {
        echo Unable to manufacture $tarball
        [ -f $dest/$name.$save.tar.gz ] &&
            mv $dest/$name.$save.tar.gz $dest/$name.tar.gz
            echo restored $dest/$name.$save.gz
    }

    popd
done

#
# eof
#
