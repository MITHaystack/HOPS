#!/bin/sh
#
# for the moment, this is the same as legacy unpack
# but eventually, we may have additional data that is not legacy
#
[ -x $MHO_REGRESSION_DATA/bootstrap/legacy_unpack.sh ] &&
exec $MHO_REGRESSION_DATA/bootstrap/legacy_unpack.sh "$@" ||
{ echo configuration error ; exit 99 ; }
# eof
