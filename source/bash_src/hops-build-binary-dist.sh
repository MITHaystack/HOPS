#!/bin/bash
#
# hops-build-binary-dist.sh
#
# Build a relocatable HOPS binary distribution (CPack TGZ) on the current host.
# The resulting tarball is renamed to embed the OS name/version and the python
# version the bindings were built against, e.g.:
#
#     hops-4.0.0-x86_64-ubuntu-22.04-python3.10.tar.gz
#     hops-4.0.0-x86_64-rocky-9.3-python3.9.tar.gz
#
# NOTE: the CMake flags below should be kept in sync with the CI release workflow
# (.github/workflows/release-binary-dist.yml). One difference is unavoidable: that
# workflow builds difxio WITHOUT gsl, whereas this script links the difxio from
# $DIFX_BUILD_DIR (a full DiFX install, can be gsl-linked) - so difxinput2json /
# difx2mark4 built here may carry a runtime libgsl dependency the CI ones do not.
# Need to be careful because a libgsl link may cause the distribution to NOT be relocatable
#
# The following environment variables must be defined (e.g. in ~/.bashrc):
#   DIFX_BUILD_DIR        - DiFX install dir containing setup.bash (for difxio)
#   HOPS_CI_DIR           - working dir holding the HOPS source checkout
#   HOPS_CI_LOG_DIR       - directory to write configure/build logs into
#
# Optional:
#   HOPS_DIST_DIR         - where to deposit the finished tarball
#                           (defaults to $HOPS_CI_DIR)
#   HOPS_BUILD_JOBS       - parallel build jobs (defaults to $(nproc))
#   HOPS_CI_MAILER        - mailer command for the report
#   HOPS_CI_MAIL_ADDRESS  - recipient address for the report

die() { echo "ERROR: $*" >&2; exit 1; }

# Emit a "<id>-<version>" tag identifying the host OS (no spaces, lower-case)
detect_os_tag()
{
    local id="" ver=""

    if [ -r /etc/os-release ]; then
        # shellcheck disable=SC1091
        . /etc/os-release
        id="${ID:-unknown}"
        ver="${VERSION_ID:-}"
    elif [ -r /etc/redhat-release ]; then
        # e.g. "Rocky Linux release 9.3 (Blue Onyx)"
        id="$(awk '{print tolower($1)}' /etc/redhat-release)"
        ver="$(grep -oE '[0-9]+(\.[0-9]+)*' /etc/redhat-release | head -n1)"
    elif command -v lsb_release >/dev/null 2>&1; then
        id="$(lsb_release -si)"
        ver="$(lsb_release -sr)"
    else
        id="unknown"
    fi

    # sanitize: lower-case, spaces/dots-in-id -> dashes
    id="$(echo "$id" | tr '[:upper:]' '[:lower:]' | tr ' ' '-')"

    if [ -n "$ver" ]; then
        echo "${id}-${ver}"
    else
        echo "${id}"
    fi
}

# Emit a "python<major>.<minor>" tag for the interpreter the bindings build
# against (pybind11). pyinfo.py prints "lib/python3.10/site-packages"; we pull
# the "python3.10" component out of that so the package name matches the ABI.
detect_python_tag()
{
    local site
    site="$(python3 "$HOPS_CI_DIR/pyinfo.py")" || return 1
    basename "$(dirname "$site")"
}

#set up
source "$HOME/.bashrc"

[ -n "${HOPS_CI_DIR:-}" ]     || die "HOPS_CI_DIR is not set."
[ -n "${HOPS_CI_LOG_DIR:-}" ] || die "HOPS_CI_LOG_DIR is not set."
[ -n "${DIFX_BUILD_DIR:-}" ]  || die "DIFX_BUILD_DIR is not set."

BUILD_JOBS="${HOPS_BUILD_JOBS:-$(nproc)}"
DIST_DIR="${HOPS_DIST_DIR:-$HOPS_CI_DIR}"
BUILD_DIR="$HOPS_CI_DIR/bdist-build"

mkdir -p "$HOPS_CI_LOG_DIR" "$DIST_DIR"

START_TIME="$(date)"

# Use the existing checkout if HOPS_CI_DIR is a git repo, otherwise make a
# fresh clone of master there.
if [ -d "$HOPS_CI_DIR/.git" ]; then
    echo "Using existing HOPS checkout in $HOPS_CI_DIR"
else
    echo "Cloning fresh HOPS master into $HOPS_CI_DIR"
    git clone https://github.com/MITHaystack/HOPS.git "$HOPS_CI_DIR"
fi

cd "$HOPS_CI_DIR"
CURRENT_REV="$(git rev-parse --short HEAD)"

#configure & build

echo "Building binary distribution in: $BUILD_DIR (rev $CURRENT_REV)"

# Always start from a clean build tree for a reproducible distribution.
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Put DiFX (difxio) on our path.
source "$DIFX_BUILD_DIR/setup.bash"

CONFIG_LOG="$HOPS_CI_LOG_DIR/config-${CURRENT_REV}.log"
BUILD_LOG="$HOPS_CI_LOG_DIR/build-${CURRENT_REV}.log"

cd "$BUILD_DIR"
# Keep these flags in sync with .github/workflows/release-binary-dist.yml so the
# locally-built distribution matches the CI-produced one.
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DHOPS_BDIST_LICENSE_COMPAT_ONLY=ON \
    -DHOPS_ENABLE_TEST=OFF \
    -DHOPS_ENABLE_DEBUG_MSG=OFF \
    -DHOPS_USE_DIFXIO=ON \
    -DHOPS_USE_PYBIND11=ON \
    "$HOPS_CI_DIR" 2>&1 | tee "$CONFIG_LOG"

make -j"$BUILD_JOBS" install 2>&1 | tee "$BUILD_LOG"

# CPack produces hops-<version>-<arch>.tar.gz (see CPACK_PACKAGE_FILE_NAME).
cpack -G TGZ

OS_TAG="$(detect_os_tag)"
PY_TAG="$(detect_python_tag)"

# Rename the freshly built tarball(s) to embed the OS and python tags, e.g.
#   hops-4.0.0-x86_64-ubuntu-22.04-python3.10.tar.gz
shopt -s nullglob
PACKAGES=( "$BUILD_DIR"/hops-*.tar.gz )
[ "${#PACKAGES[@]}" -gt 0 ] || die "cpack did not produce a tarball in $BUILD_DIR"

FINAL_PKG=""
for pkg in "${PACKAGES[@]}"; do
    base="$(basename "${pkg%.tar.gz}")"
    FINAL_PKG="$DIST_DIR/${base}-${OS_TAG}-${PY_TAG}.tar.gz"
    mv -f "$pkg" "$FINAL_PKG"
    echo "Created distribution: $FINAL_PKG"
done

END_TIME="$(date)"

#send status
if [ -n "${HOPS_CI_MAILER:-}" ] && [ -n "${HOPS_CI_MAIL_ADDRESS:-}" ]; then
    printf 'HOPS4 binary distribution build\n\n'\
'  rev:     %s\n  os:      %s\n  python:  %s\n  start:   %s\n  end:     %s\n  package: %s\n\n'\
'Logs: %s, %s\n' \
        "$CURRENT_REV" "$OS_TAG" "$PY_TAG" "$START_TIME" "$END_TIME" "$FINAL_PKG" \
        "$CONFIG_LOG" "$BUILD_LOG" \
        | "$HOPS_CI_MAILER" -s "HOPS4 binary dist - ${CURRENT_REV} (${OS_TAG}, ${PY_TAG})" "$HOPS_CI_MAIL_ADDRESS"
fi
