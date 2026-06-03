#!/bin/bash
# hops-install-deps.sh
#
# Install the system packages HOPS needs to build from source and to run,
# on Debian/Ubuntu (apt) and RHEL/Rocky/Alma/Fedora (dnf/yum) systems.
#
# The package lists are primarily:
#   - build tools          (compiler, cmake, python dev headers, wget, jq)
#   - native runtime libs   (FFTW3, PGPLOT + gfortran, X11/Xpm, GSL)
#   - runtime executables   (gnuplot, ghostscript, imagemagick)
#   - python packages       (numpy, scipy, matplotlib) (optional), skip with --no-python
#                           these can be installed locally by the build system with: -DHOPS_PYPI_MANAGE_DEPS=ON
#                           or managed externally by the user
#
# Usage:
#   ./hops-install-deps.sh [--yes] [--dry-run] [--no-python] [--help]
#
#   --yes        pass the package manager's assume-yes flag (non-interactive)
#   --dry-run    print the install command without running it
#   --no-python  skip numpy/scipy/matplotlib (let the build system's pip calls or user handle it)
#   --help       show this message
#
# NOTE (RHEL/Rocky/Alma/Fedora): PGPLOT is not in the base repos. It is provided
# by RPM Fusion (nonfree). This script installs everything else, then attempts
# pgplot-devel separately...in case it fails.

set -euo pipefail

ASSUME_YES=""
DRY_RUN="no"
WITH_PYTHON="yes"

usage() {
    # print the leading comment block (skip the shebang, stop at the first
    # non-comment line) with the leading "# " stripped.
    awk 'NR==1 {next} /^#/ {sub(/^# ?/, ""); print; next} {exit}' "$0"
}

for arg in "$@"; do
    case "$arg" in
        --yes|-y)      ASSUME_YES="yes" ;;
        --dry-run|-n)  DRY_RUN="yes" ;;
        --no-python)   WITH_PYTHON="no" ;;
        --help|-h)     usage; exit 0 ;;
        *) echo "Unknown option: $arg" >&2; usage; exit 1 ;;
    esac
done

# Run package-manager commands with sudo when we are not already root.
SUDO=""
if [ "$(id -u)" -ne 0 ]; then
    if command -v sudo >/dev/null 2>&1; then
        SUDO="sudo"
    else
        echo "Warning: not root and 'sudo' not found; commands may fail." >&2
    fi
fi

# run "$@", honoring --dry-run (print only) and prefixing sudo as needed.
run() {
    if [ "$DRY_RUN" = "yes" ]; then
        echo "+ $SUDO $*"
    else
        echo "+ $SUDO $*"
        # shellcheck disable=SC2086
        $SUDO "$@"
    fi
}

install_apt() {
    local build_pkgs=(build-essential cmake python3-dev python3-pip wget jq)
    local lib_pkgs=(libfftw3-dev pgplot5 libgfortran5 libx11-dev libxpm-dev \
                    libgsl-dev gsl-bin libgslcblas0 gnuplot ghostscript \
                    ghostscript-x imagemagick binutils)
    local py_pkgs=(python3-numpy python3-scipy python3-matplotlib)

    local pkgs=("${build_pkgs[@]}" "${lib_pkgs[@]}")
    [ "$WITH_PYTHON" = "yes" ] && pkgs+=("${py_pkgs[@]}")

    local yflag=""
    [ "$ASSUME_YES" = "yes" ] && yflag="-y"

    run apt-get update
    run apt-get install $yflag "${pkgs[@]}"
}

install_dnf() {
    local pm="$1"   # dnf or yum
    local build_pkgs=(gcc gcc-c++ gcc-gfortran make cmake python3-devel \
                      python3-pip wget jq)
    # NOTE: pgplot-devel is handled separately
    local lib_pkgs=(fftw-devel libgfortran libX11-devel libXpm-devel gsl-devel \
                    gnuplot ghostscript ImageMagick binutils)
    local py_pkgs=(python3-numpy python3-scipy python3-matplotlib)

    local pkgs=("${build_pkgs[@]}" "${lib_pkgs[@]}")
    [ "$WITH_PYTHON" = "yes" ] && pkgs+=("${py_pkgs[@]}")

    local yflag=""
    [ "$ASSUME_YES" = "yes" ] && yflag="-y"

    # EPEL hosts some of the supporting packages on RHEL/Rocky/Alma; best-effort only
    if [ "$DRY_RUN" = "yes" ]; then
        run "$pm" install $yflag epel-release
    else
        run "$pm" install $yflag epel-release || \
            echo "Note: could not install epel-release; some packages may be unavailable." >&2
    fi

    run "$pm" install $yflag "${pkgs[@]}"

    # PGPLOT is not in EPEL/base; it ships via RPM Fusion (nonfree). Attempt it
    # on its own so a missing repo doesn't abort the rest of the install.
    if [ "$DRY_RUN" = "yes" ]; then
        run "$pm" install $yflag pgplot pgplot-devel
    elif ! run "$pm" install $yflag pgplot pgplot-devel; then
        echo "Note: pgplot/pgplot-devel could not be installed (needed for HOPS3)." >&2
        echo "      Enable the RPM Fusion nonfree repository, then re-run" >&2
        echo "      See https://rpmfusion.org/Configuration" >&2
    fi
}

if command -v apt-get >/dev/null 2>&1; then
    echo "Detected apt (Debian/Ubuntu)."
    install_apt
elif command -v dnf >/dev/null 2>&1; then
    echo "Detected dnf (RHEL/Rocky/Alma/Fedora)."
    install_dnf dnf
elif command -v yum >/dev/null 2>&1; then
    echo "Detected yum (older RHEL/CentOS)."
    install_dnf yum
else
    echo "Error: no supported package manager (apt-get, dnf, yum) found." >&2
    echo "Install the HOPS dependencies manually; see the package lists in this script." >&2
    exit 1
fi

echo "Done."
