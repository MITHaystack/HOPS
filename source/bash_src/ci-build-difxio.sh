#!/bin/bash
#
# ci-build-difxio.sh
#
# Build ONLY the difxio library from DiFX 'main', WITHOUT gsl, for use as a
# build-time dependency of HOPS (difxinput2json / difx2hops / difx2mark4, which
# static-link difxio). Intended for the release CI workflows.
#
# Required env:
#   DIFXROOT   - install destination for difxio
# Optional env:
#   DIFX_SRC   - where to clone DiFX (default: $RUNNER_TEMP/difx-src or /tmp/...)
#
# On success, makes the installed difxio.pc discoverable: appends PKG_CONFIG_PATH
# to $GITHUB_ENV when running under GitHub Actions, otherwise prints it to stdout.

set -eo pipefail

: "${DIFXROOT:?DIFXROOT must be set}"
DIFX_SRC="${DIFX_SRC:-${RUNNER_TEMP:-/tmp}/difx-src}"

mkdir -p "$DIFXROOT"
if [ ! -d "$DIFX_SRC/.git" ]; then
    git clone --depth 1 https://github.com/difx/difx.git "$DIFX_SRC"
fi
cd "$DIFX_SRC"

# install-difx blindly appends to these env vars (e.g. ":"+os.environ.get(...)),
# which throws TypeError when they are unset. Pre-seed any that are missing.
export PKG_CONFIG_PATH="${PKG_CONFIG_PATH:-}"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}"
export PYTHONPATH="${PYTHONPATH:-}"
export PERL5LIB="${PERL5LIB:-}"

# difxio auto-enables gsl iff pkg-config finds it (same as the optional fftw3).
# Hand install-difx a pkg-config search path that mirrors the system one minus
# gsl.pc so difxio builds gsl-free, keeping the static-linked HOPS difx tools
# self-contained / relocatable.
NOGSL_PKGDIR="${RUNNER_TEMP:-/tmp}/pkgconfig-nogsl"
mkdir -p "$NOGSL_PKGDIR"
for d in $(pkg-config --variable pc_path pkg-config | tr ':' ' '); do
    [ -d "$d" ] && cp -n "$d"/*.pc "$NOGSL_PKGDIR"/ 2>/dev/null || true
done
rm -f "$NOGSL_PKGDIR"/gsl.pc

PKG_CONFIG_LIBDIR="$NOGSL_PKGDIR" ./install-difx --noipp --doonly=difxio

PC="$DIFXROOT/lib/pkgconfig/difxio.pc"
[ -f "$PC" ] || { echo "ERROR: difxio.pc not found at $PC" >&2; exit 1; }
if grep -qi gsl "$PC"; then
    echo "ERROR: difxio.pc still references gsl (no-gsl build did not take)" >&2
    exit 1
fi

if [ -n "${GITHUB_ENV:-}" ]; then
    echo "PKG_CONFIG_PATH=$DIFXROOT/lib/pkgconfig:${PKG_CONFIG_PATH:-}" >> "$GITHUB_ENV"
else
    echo "PKG_CONFIG_PATH=$DIFXROOT/lib/pkgconfig"
fi
