#!/bin/bash
#
# Script to nuke saved tarballs, see legacy_tar.sh
#
[ $# -eq 0 ] && {
    echo 'You need exactly one of "list" or "nuke" as argument'
    echo 'or you can use save:label to rename saved tarballs'
    exit 1
}
[ -n "$MHO_REGRESSION_DATA" ] || {
    echo You should set MHO_REGRESSION_DATA to the parent
    echo directory of the data hierarchy
    exit 2
}

dest=$MHO_REGRESSION_DATA/tarballs/legacy
[ -d "$dest" ] || {
    echo $dest is missing, go fix that
    exit 3
}

label=`expr $1 : 'save:\(.*\)'`
[ -n "$label" ] && arg=save || arg=$1

[ "$arg" = list -o "$arg" = nuke -o "$arg" = save ] || {
    echo 'Exactly one of "list", "nuke" or "save:label" as argument, please.'
    echo "arg was '$arg'"
    exit 4
}

# this may work...
cmd="x={}; mv \$x \${x/save/$label}"
echo "'$cmd'"
[ -n "$label" ] && find $dest -name \*.save.gz -exec sh -c "$cmd" \;

[ "$arg" = nuke ] && find $dest -name \*.save.gz -exec rm '{}' \;
[ "$arg" = list ] && find $dest -name \*.save.gz -print

#
# eof
#
