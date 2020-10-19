#!/bin/bash
#
svn=trunk/chops
git=hops-git
tg='/'
#
USAGE="$0 [svn=$svn] [git=$git] [force=true|false] [tg=$tg] [rsync options] [.]

This utility script will import files from the source area of version 3
into a matching version 4 area.  This intended to be used only as the
bootstrap from HOPS3 to HOPS4, as the sources are expected to diverge
after that.  It is just a thin wrapper around rsync.  Used with the -n
option, it will echo the command to be used, rather than doing it.

It is intended to work ONLY from the destination directory, and then,
only if there is a matching source directory.  It will refuse to run
if a local Makefile.am exists.  If desperate set force to true.

The tg variable allows you to limit the rsync to a specific file or
directory.  It should begin with a '/' directory delimiter.

If you just want to do the rsync from the svn area to the git area use
'.' as a final argument
"
[ $# -eq 0 -o ${1-'--help'} = '--help' ] && { echo "Usage: $USAGE"; exit 0; }

force=false
while [ $# -gt 0 ]
do
    case $1 in
    svn=*)   eval "$1" ; shift ;;
    git=*)   eval "$1" ; shift ;;
    force=*) eval "$1" ; shift ;;
    tg=*)    eval "$1" ; shift ;;
    .)       break ;;
    *=)      echo "Usage: $USAGE"; exit 1;;
    esac
done
[ "$force" = true -o "$force" = false ] || { echo "Usage: $USAGE"; exit 1; }
$force || {
    [ -f 'Makefile.am' ] && { echo "Usage: $USAGE"; exit 2; }
}

cwd=`pwd`
mhopath=`expr "$cwd" : "\(/.*\)/$git/.*"`
gitpath=`expr "$cwd" : "/.*/$git/\(.*\)"`
svnpath=$mhopath/$svn/$gitpath
destone=`dirname $tg`
destiny=`basename $destone`
[ "$destiny" = '' -o "$destiny" = '/' ] && destiny='.'

echo curpath is $cwd
echo mhopath is $mhopath
echo gitpath is $gitpath
echo svnpath is $svnpath
echo destiny is $destiny

echo rsync -av "$@" $svnpath$tg $destiny | head
     rsync -av "$@" $svnpath$tg $destiny

exit 0
#
# eof
#
