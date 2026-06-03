#!/bin/sh
# hops_buildenv.sh
#
# Set up the environment needed to *compile/link* downstream code against an
# installed HOPS:
#   CMAKE_PREFIX_PATH : lets `find_package(Hops)` locate the installed config package
#   PKG_CONFIG_PATH   : lets `pkg-config` locate hops3.pc / hops4.pc by name
#
# These are ONLY needed when building against HOPS. The installed HOPS
# command-line tools do NOT depend on them, which is why this is kept separate
# from hops.bash and must be sourced explicitly, e.g.
#   . "$HOPS_INSTALL/bin/hops_buildenv.sh"
# It relies on HOPS_INSTALL (and the OLD_HOPS_* caches) set up by hops.bash.

if [ -z "${HOPS_INSTALL:-}" ]; then
    echo "hops_buildenv.sh: HOPS_INSTALL is not set; source hops.bash first." >&2
    return 0 2>/dev/null || exit 0
fi

# Strip any entries from a previously-sourced HOPS instance. hops.bash exports
# OLD_HOPS_INSTALL / OLD_HOPS_SYS pointing at the prior prefix (if any).
HOPS_OLD_PREFIX="${OLD_HOPS_INSTALL:-${OLD_HOPS_SYS:-NULL}}"

# CMAKE_PREFIX_PATH: lets find_package(Hops) locate the installed config package
HOPS_NEW_CMAKE_PREFIX_PATH=$(printf '%s\n' "${CMAKE_PREFIX_PATH:-}" | sed "s|${HOPS_OLD_PREFIX}:||g")
HOPS_NEW_CMAKE_PREFIX_PATH="$HOPS_INSTALL:$HOPS_NEW_CMAKE_PREFIX_PATH"
export CMAKE_PREFIX_PATH="$HOPS_NEW_CMAKE_PREFIX_PATH"

# PKG_CONFIG_PATH: lets pkg-config locate hops3.pc / hops4.pc by name
HOPS_NEW_PKG_CONFIG_PATH=$(printf '%s\n' "${PKG_CONFIG_PATH:-}" | sed "s|${HOPS_OLD_PREFIX}/lib/pkgconfig:||g")
HOPS_NEW_PKG_CONFIG_PATH="$HOPS_INSTALL/lib/pkgconfig:$HOPS_NEW_PKG_CONFIG_PATH"
export PKG_CONFIG_PATH="$HOPS_NEW_PKG_CONFIG_PATH"

unset HOPS_OLD_PREFIX HOPS_NEW_CMAKE_PREFIX_PATH HOPS_NEW_PKG_CONFIG_PATH
