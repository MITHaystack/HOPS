################################################################################
# HopsReproducible.cmake
#
# Settings that make the compiled output independent of *where* it was built,
# so that two checkouts in different directories produce identical binaries.
#
# This module strips absolute build/source directory paths out of the emitted
# objects. Those paths otherwise leak in via:
#   - DWARF debug info (paths to source files; only with -g builds)
#   - the __FILE__ macro and assert()/exception strings
# The path-mapping flags collapse both ${CMAKE_SOURCE_DIR} and ${CMAKE_BINARY_DIR}
# to ".", which is safe here: the only runtime consumer of __FILE__ (MHO_Profiler)
# already reduces it to a basename at compile time.
#
# NOTE: this must be included BEFORE the add_subdirectory() calls (and before the
# extern subdirectory in particular) so that every target -- including the bundled
# externs (matplot++/CImg, hops3 C) -- inherits the flags.
################################################################################

include(CheckCXXCompilerFlag)

option(HOPS_REPRODUCIBLE_PATHS "Strip absolute build/source paths from compiled binaries (reproducible builds)." ON)

if(HOPS_REPRODUCIBLE_PATHS)

    set(HOPS_PREFIX_MAP_FLAGS "")

    # -ffile-prefix-map is the superset (covers both debug info and __FILE__/macros);
    # available on gcc >= 8 and clang >= 10. Fall back to the two narrower flags on
    # older compilers that we still support (gcc 6/7 only have -fdebug-prefix-map).
    check_cxx_compiler_flag("-ffile-prefix-map=a=b" HOPS_HAS_FILE_PREFIX_MAP)
    if(HOPS_HAS_FILE_PREFIX_MAP)
        set(HOPS_PREFIX_MAP_FLAGS
            "-ffile-prefix-map=${CMAKE_SOURCE_DIR}=. -ffile-prefix-map=${CMAKE_BINARY_DIR}=.")
    else()
        check_cxx_compiler_flag("-fdebug-prefix-map=a=b" HOPS_HAS_DEBUG_PREFIX_MAP)
        check_cxx_compiler_flag("-fmacro-prefix-map=a=b" HOPS_HAS_MACRO_PREFIX_MAP)
        if(HOPS_HAS_DEBUG_PREFIX_MAP)
            string(APPEND HOPS_PREFIX_MAP_FLAGS
                " -fdebug-prefix-map=${CMAKE_SOURCE_DIR}=. -fdebug-prefix-map=${CMAKE_BINARY_DIR}=.")
        endif()
        if(HOPS_HAS_MACRO_PREFIX_MAP)
            string(APPEND HOPS_PREFIX_MAP_FLAGS
                " -fmacro-prefix-map=${CMAKE_SOURCE_DIR}=. -fmacro-prefix-map=${CMAKE_BINARY_DIR}=.")
        endif()
    endif()

    string(STRIP "${HOPS_PREFIX_MAP_FLAGS}" HOPS_PREFIX_MAP_FLAGS)

    if(HOPS_PREFIX_MAP_FLAGS)
        # apply to both C (hops3, externs) and C++ (hops4)
        set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} ${HOPS_PREFIX_MAP_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${HOPS_PREFIX_MAP_FLAGS}")
        message(STATUS "Reproducible path mapping enabled: ${HOPS_PREFIX_MAP_FLAGS}")
    else()
        message(STATUS "Reproducible path mapping requested but no supported "
                       "prefix-map flags were found for this compiler.")
    endif()

endif()

################################################################################
# Relocatable, path-independent install RPATH (Linux/ELF).
#
# By default CMake bakes the *absolute* install lib directories into the
# installed binaries' DT_RUNPATH. That makes the install both non-relocatable
# and non-reproducible (the RPATH depends on CMAKE_INSTALL_PREFIX). We replace
# it with a fixed set of $ORIGIN-relative entries so the loader finds the HOPS
# libraries relative to the binary's own location, regardless of where the tree
# is installed or moved to.
#
# The install layout (see top-level CMakeLists.txt) places:
#   executables in   bin/   and   bin/test/
#   libraries   in   lib/ , lib/hops/   (and lib64/ for the bundled tz library)
# A single global CMAKE_INSTALL_RPATH is applied to every target, so we list all
# the $ORIGIN-relative offsets needed to reach the lib dirs from any of those
# locations. Entries that don't resolve for a given binary are simply ignored by
# the loader, so the superset is harmless.
#
# NOTE: must run before the add_subdirectory() calls so targets pick this up.
################################################################################

option(HOPS_REPRODUCIBLE_RPATH "Use \$ORIGIN-relative install RPATH (relocatable, reproducible installs)." ON)

if(HOPS_REPRODUCIBLE_RPATH AND NOT APPLE)

    # Do NOT append the absolute link-time library dirs to the install RPATH;
    # those are build-host paths and would defeat both relocatability and
    # reproducibility. (This is CMake's default, but pin it for clarity.)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH OFF)

    # Make the *build-tree* RPATH $ORIGIN-relative as well (CMake >= 3.14).
    #
    # This is essential for reproducibility, not just relocatability: at link
    # time CMake bakes the absolute build-tree library dirs into .dynstr and
    # sizes that section to fit. At install time it overwrites the RPATH with the
    # (shorter) $ORIGIN install RPATH in place, but cannot shrink the allocated section,
    # it just zero-pads it. So the *length* of the absolute build path leaks into
    # the installed binary's .dynstr size (and hence every following section
    # offset and the GNU build-id), making two builds in different directories
    # differ even though the final RPATH string is identical. Forcing the build
    # RPATH to be $ORIGIN-relative removes the absolute path entirely, so the
    # section is sized identically regardless of where the build happened. Unlike
    # CMAKE_BUILD_WITH_INSTALL_RPATH this keeps the build tree runnable (ctest).
    if(NOT CMAKE_VERSION VERSION_LESS "3.14")
        set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)
    else()
        message(WARNING "CMake < 3.14: CMAKE_BUILD_RPATH_USE_ORIGIN unavailable; "
                        "installed binaries' .dynstr size may depend on the build "
                        "path, breaking bit-for-bit reproducibility.")
    endif()

    # Reach lib/, lib/hops/, lib64/ from bin/, bin/test/, lib/, lib/hops/, and
    # from the HOPS python module dir (lib/python3/site-packages, three levels
    # deep) so the installed pybind11 modules locate the HOPS libs via RPATH
    # without needing LD_LIBRARY_PATH.
    set(CMAKE_INSTALL_RPATH
        "$ORIGIN"
        "$ORIGIN/hops"
        "$ORIGIN/lib"
        "$ORIGIN/lib/hops"
        "$ORIGIN/lib64"
        "$ORIGIN/.."
        "$ORIGIN/../lib"
        "$ORIGIN/../lib/hops"
        "$ORIGIN/../lib64"
        "$ORIGIN/../../lib"
        "$ORIGIN/../../lib/hops"
        "$ORIGIN/../../lib64"
        "$ORIGIN/../../../lib"
        "$ORIGIN/../../../lib/hops"
        "$ORIGIN/../../../lib64")

    message(STATUS "Reproducible install RPATH enabled (\$ORIGIN-relative).")

endif()
