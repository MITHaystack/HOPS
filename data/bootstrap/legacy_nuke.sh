#!/bin/bash
#
# Script to nuke saved tarballs, see legacy_tar.sh
#
[ $# -eq 0 ] && {
    echo 'need exactly one of "list" or "nuke" as argument'
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

[ "$1" = list -o "$1" = nuke ] || {
    echo 'need exactly one of "list" or "nuke" as argument'
    exit 4
}

[ "$1" = nuke ] && find $dest -name \*.save.gz -exec rm '{}' \;
[ "$1" = list ] && find $dest -name \*.save.gz -print

#
# eof
#
