# HopsPkgConfig.cmake
#
# Generates pkg-config (.pc) files and a CMake package-config
# (HopsConfig.cmake / HopsConfigVersion.cmake) for downstream projects.
#
# Exposes:
#   hops_generate_pkgconfig(NAME <name>
#                          DESCRIPTION <text>
#                          LIBS <link-line>
#                          [CFLAGS <flags>]
#                          [REQUIRES <pkgs>]
#                          [REQUIRES_PRIVATE <pkgs>]
#                          [LIBS_PRIVATE <link-line>])
#
# Builds <prefix>/lib/pkgconfig/<name>.pc from cmake/hops.pc.in.

function(hops_generate_pkgconfig)
    set(_oneValueArgs NAME DESCRIPTION)
    set(_multiValueArgs LIBS CFLAGS REQUIRES REQUIRES_PRIVATE LIBS_PRIVATE)
    cmake_parse_arguments(PCARG "" "${_oneValueArgs}" "${_multiValueArgs}" ${ARGN})

    if(NOT PCARG_NAME)
        message(FATAL_ERROR "hops_generate_pkgconfig: NAME is required")
    endif()

    # Local variables for configure_file substitution.
    set(PC_NAME             "${PCARG_NAME}")
    set(PC_DESCRIPTION      "${PCARG_DESCRIPTION}")
    string(REPLACE ";" " " PC_LIBS             "${PCARG_LIBS}")
    string(REPLACE ";" " " PC_CFLAGS           "${PCARG_CFLAGS}")
    string(REPLACE ";" " " PC_REQUIRES         "${PCARG_REQUIRES}")
    string(REPLACE ";" " " PC_REQUIRES_PRIVATE "${PCARG_REQUIRES_PRIVATE}")
    string(REPLACE ";" " " PC_LIBS_PRIVATE     "${PCARG_LIBS_PRIVATE}")

    set(_pc_out "${CMAKE_CURRENT_BINARY_DIR}/${PCARG_NAME}.pc")
    configure_file(
        "${PROJECT_SOURCE_DIR}/cmake/hops.pc.in"
        "${_pc_out}"
        @ONLY
    )
    install(FILES "${_pc_out}"
            DESTINATION "${LIB_INSTALL_DIR}/pkgconfig")
endfunction()


# hops_generate_all_pkgconfig()
#
# Writes hops3.pc and hops4.pc using the option state available at the
# top level. Call AFTER all add_subdirectory()s so HOPS_USE_* values
# have been finalized.
function(hops_generate_all_pkgconfig)

    # ---- hops3: legacy C libraries ----
    set(_hops3_libs
        -lafio -ldfio -lffcontrol -lffcore -lffio -lffmath
        -lffplot -lffsearch -lmk4util -lmsg -lvex)

    set(_hops3_cflags "")
    set(_hops3_requires "")

    if(HOPS_USE_FFTW3)
        list(APPEND _hops3_requires "fftw3")
        list(APPEND _hops3_cflags "-DHOPS_USE_FFTW3")
    endif()

    hops_generate_pkgconfig(
        NAME hops3
        DESCRIPTION "HOPS3 legacy C libraries (mk4 / vex / fourfit support)"
        LIBS ${_hops3_libs}
        CFLAGS ${_hops3_cflags}
        REQUIRES ${_hops3_requires}
    )

    # ---- hops4: modern C++ libraries ----
    set(_hops4_libs
        -lMHO_Calibration
        -lMHO_Containers
        -lMHO_Control
        -lMHO_Fringe
        -lMHO_Initialization
        -lMHO_Math
        -lMHO_Message
        -lMHO_MK4Interface
        -lMHO_Operators
        -lMHO_Utilities
        -lMHO_Vex2JSON
        -ldate-tz
    )

    # Mirror the -D defines injected by hops_add_cxxflag() in the top-level
    # build so downstream consumers see the same gates as our public headers.
    set(_hops4_cflags
        -std=c++${HOPS_CXX_STANDARD}
        -I\${includedir}/hops_extern/eigen
        -I\${includedir}/hops_extern/json
        -I\${includedir}/hops_extern/cli11
        -DHAVE_CONFIG_H
    )

    if(HOPS_USE_FFTW3)
        list(APPEND _hops4_cflags "-DHOPS_USE_FFTW3")
    endif()
    if(HOPS_USE_OPENMP)
        list(APPEND _hops4_cflags "-DHOPS_USE_OPENMP")
        if(OpenMP_CXX_FLAGS)
            list(APPEND _hops4_cflags "${OpenMP_CXX_FLAGS}")
            list(APPEND _hops4_libs "${OpenMP_CXX_FLAGS}")
        endif()
    endif()
    if(HOPS_USE_MATPLOTPP)
        list(APPEND _hops4_cflags "-DUSE_MATPLOTPP")
        list(APPEND _hops4_libs "-lmatplot" "-lMHO_MatplotPlugin")
    endif()
    if(HOPS_USE_PYBIND11)
        list(APPEND _hops4_libs "-lMHO_PluginManagement" "-lMHO_pyVisitors")
    endif()
    if(HOPS_USE_CUDA)
        list(APPEND _hops4_cflags "-DHOPS_USE_CUDA")
    endif()
    if(HOPS_USE_MPI)
        list(APPEND _hops4_cflags "-DHOPS_USE_MPI")
    endif()
    if(HOPS_USE_HDF5)
        list(APPEND _hops4_cflags "-DHOPS_USE_HDF5")
    endif()

    set(_hops4_requires "hops3")
    if(HOPS_USE_DIFXIO)
        list(APPEND _hops4_requires "difxio")
    endif()

    hops_generate_pkgconfig(
        NAME hops4
        DESCRIPTION "HOPS4 C++ libraries (fourfit4 / VLBI fringe fitting)"
        LIBS ${_hops4_libs}
        CFLAGS ${_hops4_cflags}
        REQUIRES ${_hops4_requires}
    )

endfunction()


# hops_generate_cmake_config()
#
# Writes HopsConfig.cmake / HopsConfigVersion.cmake into
# <prefix>/lib/cmake/Hops. The config file defines imported targets
# manually (from the lists below) rather than using install(EXPORT ...).
# This avoids the export-set consistency requirements that the current
# plugin-INTERFACE layout doesn't satisfy.
function(hops_generate_cmake_config)
    include(CMakePackageConfigHelpers)

    set(_cmake_install_dir "${LIB_INSTALL_DIR}/cmake/Hops")

    # Library lists baked into the generated HopsConfig.cmake.
    set(HOPS3_LIBS
        afio dfio ffcontrol ffcore ffio ffmath ffplot ffsearch mk4util msg vex)
    set(HOPS4_LIBS
        MHO_Calibration MHO_Containers MHO_Control MHO_Fringe MHO_Initialization
        MHO_Math MHO_Message MHO_MK4Interface MHO_Operators MHO_Utilities
        MHO_Vex2JSON date-tz)
    if(HOPS_USE_MATPLOTPP)
        list(APPEND HOPS4_LIBS matplot MHO_MatplotPlugin)
    endif()
    if(HOPS_USE_PYBIND11)
        list(APPEND HOPS4_LIBS MHO_PluginManagement MHO_pyVisitors)
    endif()
    if(HOPS_USE_OPENMP)
        list(APPEND HOPS4_LIBS MHO_OpenMPPlugin)
    endif()

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/HopsConfigVersion.cmake"
        VERSION "${HOPS_VERSION_NUMBER}"
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/HopsConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/HopsConfig.cmake"
        INSTALL_DESTINATION "${_cmake_install_dir}"
        PATH_VARS INCLUDE_INSTALL_DIR LIB_INSTALL_DIR BIN_INSTALL_DIR
    )

    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/HopsConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/HopsConfigVersion.cmake"
        DESTINATION "${_cmake_install_dir}"
    )
endfunction()
