#!/bin/sh
HOPS_GIT_REV=$(command -v git >/dev/null 2>&1 && (git rev-parse --is-inside-work-tree >/dev/null 2>&1 && git log -1 --format="%H" | cut -c1-6 || echo "*") || echo "*")
export HOPS_LAST_GIT_REVISION=$HOPS_GIT_REV
echo $HOPS_LAST_GIT_REVISION
