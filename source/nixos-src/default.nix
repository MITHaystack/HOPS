# Pin the specific version of nix packages to work with hops so it will always be reproducible.
{ pkgs ? (import (builtins.fetchGit {
  url = "https://github.com/NixOS/nixpkgs-channels.git";
  ref = "nixos-20.03";
  rev = "13a15f262a2b348d6aa976017f2cd88e3a18405d";
}) { }) }:

let
  # Nix package information.
  pgplot = pkgs.stdenv.mkDerivation rec {
    pname = "pgplot";
    version = "5.2.2";

    # Source pgplot from California Technical Institute website.
    src = builtins.fetchTarball {
      url = "ftp://ftp.astro.caltech.edu/pub/pgplot/pgplot5.2.tar.gz";
      sha256 = "01hwa7fmyb3n542lvmxq4x1jbcm4pnc4p7cqak1i33azxbpzlays";
    };

    nativeBuildInputs = with pkgs; [ gfortran ];
    buildInputs = with pkgs; [ 
    x11 
 ];

    preConfigure = ''
      # We have to select drivers,
      # before continuing.
      # See 'install-unix.txt' part 4.
      # See pgplot-drivers.list for the complete list of drivers.
      echo '
      NUDRIV 0 /NULL      Null device (no output)				Std F77
      XWDRIV 1 /XWINDOW   Workstations running X Window System		C
      XWDRIV 2 /XSERVE    Persistent window on X Window System		C
      ' > drivers.list

      # TODO: use actual operating system for second argument,
      # instead of hard-coded `linux`.
      ./makemake . linux g77_gcc
    '';
    postBuild = ''
      make cpg
    '';

    installPhase = ''
      mkdir -p $out/lib
      mv cpgplot.h libcpgplot.a libpgplot.a libpgplot.so grfont.dat pgxwin_server rgb.txt $out/lib
    '';

    # pgplot information.
    meta = with pkgs.stdenv.lib; {
      description = "A Fortran- or C-callable, device-independent graphics package for making simple scientific graphs";
      homepage = "https://sites.astro.caltech.edu/~tjp/pgplot/";
      license = {
        fullName = "PGPLOT License";
        url = "https://sites.astro.caltech.edu/~tjp/pgplot/";
        free = true;
      };
      maintainers = [ ];
    };
  };
in pkgs.stdenv.mkDerivation rec {
  pname = "hops";
  version = "3.21-2936";
  
  # Source HOPS locally.
  src = ./.;

  # Source hops from latest production build.
  #src = builtins.fetchTarball {
    #url = "ftp://gemini.haystack.mit.edu/pub/${pname}/${pname}-${version}.tar.gz";
    #sha256 = "1vbqr1h67w7k613332mwp4ql5iacr65l1q5gd1zm9gg78s0rhqgy";
  #};

  nativeBuildInputs = with pkgs; [ gfortran pkg-config ];
  buildInputs = with pkgs; [ 
    fftw 
    ghostscript 
    libpng12 
    pgplot 
    x11 
    texlive.combined.scheme-full
    doxygen
    graphviz
    clang
    libtool
    automake
    autoconf
  ];

  # Point to the correct directory where pgplot is. Note: this is different from the default. See README.txt for more information.
  PGPLOT_DIR = "${pgplot}/lib";

  preConfigure = ''
    patchShebangs autogen.sh
    ./autogen.sh
    # Create build directory.
  '';

  configureFlags = [
    "--enable-devel"
    "--enable-docs"
    "CC=clang"
    "--enable-doxy"
  ];

  # Do equivalent of make check.
  doCheck = true;

  # Ignore security warnings.
  NIX_CFLAGS_COMPILE = "-Wno-error=format-security";

  # Hops information.
  meta = with pkgs.stdenv.lib; {
    description = "The Haystack Observatory Post-processing System software";
    homepage = "https://www.haystack.mit.edu/haystack-observatory-postprocessing-system-hops/";
    license = licenses.mit;
    # maintainers = ["Geoff Crew"];
  };
}
