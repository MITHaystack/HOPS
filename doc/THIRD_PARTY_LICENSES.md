# Third-Party Licenses
HOPS is MIT-licensed (see `LICENSE.md`). At install time the license text of
every redistributed sub-component/dependency is collected under
`<install-prefix>/doc/third_party_licenses/<library>/`
and also dumped into the CPack TGZ bundle.

A full list follows:

## Bundled / redistributed

- **nlohmann/json** - MIT
- **pybind11_json** - BSD-3-Clause
- **CLI11** - BSD-3-Clause
- **Howard Hinnant `date`** - MIT
- **pybind11** - BSD-3-Clause (`HOPS_USE_PYBIND11`)
- **Matplot++** - MIT (`HOPS_USE_MATPLOTPP`)
  - **CImg** - dual CeCILL-C / CeCILL v2.1; **we elect CeCILL-C** (LGPL-like, weak copyleft)
  - **nodesoup** - Unlicense (public domain)
- **Eigen** - MPL-2.0; built with `EIGEN_MPL2_ONLY` no LGPL modules used
- **ANTLR** 3.5.2 / 4.8 - BSD-3-Clause (`vex2xml`, `HOPS_USE_JAVA`)
- **Apache Commons CLI** 1.2 / 1.4 - Apache-2.0 (`vex2xml`, ships its NOTICE)

## Optionally linked at build or used at runtime (not redistributed)

- **FFTW3** - GPL-2.0+ - excluded from MIT-license only builds (HOPS4 has a built-in FFT)
- **PGPLOT** - non-free - excluded from MIT-license only builds
- **GSL** - GPL-3.0+ - excluded from MIT-license only builds
- **difxio** (DiFX) - GPL, used only by `difxinput2json` and `difx2mark4`, linked statically
- **libpng / zlib / X11 / MPI** - permissive
- **libgfortran / libgomp** - GPL + GCC Runtime Library Exception, ok
- **gnuplot** - invoked as a subprocess, not linked, ok
- **Python: numpy / scipy / matplotlib** - BSD (pip-installed, not redistributed)

## Exceptions

- **difxinput2json** - GPLv3 (isolated, statically-linked exe)
- **difx2mark4** - GPLv3 (isolated, statically-linked exe; treated exactly like `difxinput2json`)
- **cohfit** - not distributable in binary form (GSL + PGPLOT conflict); source only
- **legacy PGPLOT dependent apps** (`aedit`, `search`, `fourfit3`) - not MIT-redistributable
- **various python scripts** Various python scripts may have their own (GPL) licenses, declared in source.

## Generating a clean MIT-licensed binary distribution

Configure with `-DHOPS_BDIST_LICENSE_COMPAT_ONLY=ON`: forces off FFTW3, PGPLOT, and GSL,
and blocks CImg's transitively included GPL backends. The GPLv3 executables
`difxinput2json` and `difx2mark4` are the only non-MIT components it leaves enabled (difxio).
