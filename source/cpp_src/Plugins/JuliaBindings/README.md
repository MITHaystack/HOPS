Prerequisites for julia bindings

0. To configure the build system to compile the julia bindings ensure the cmake option HOPS_USE_JULIA is ON, e.g. from build:

    cmake -DHOPS_USE_JULIA=ON ...

1. Install the julia runtime (see https://julialang.org/)
2. Install the CxxWrap.jl Julia package (needed to build the bindings). Install it in Julia default environment with:

    julia -e 'import Pkg; Pkg.add("CxxWrap")'

3. Runtime Prerequisites (required to run the installed .jl example scripts)

    julia -e 'import Pkg; Pkg.add(["CxxWrap", "JSON3", "Plots"])'
