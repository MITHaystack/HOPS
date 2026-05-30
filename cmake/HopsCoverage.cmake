# Coverage support using gcov/lcov
# Adds HOPS_ENABLE_COVERAGE option and a 'coverage' custom target.
# Usage:
#   cmake -DHOPS_ENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ..
#   make -j$(nproc)
#   ctest
#   make coverage   # produces coverage_html/index.html in the build directory

option(HOPS_ENABLE_COVERAGE "Enable gcov/lcov coverage instrumentation." OFF)

if(HOPS_ENABLE_COVERAGE)

    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(FATAL_ERROR "Coverage instrumentation requires GCC or Clang.")
    endif()

    # Coverage builds must be unoptimized; force Debug if not already set.
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        message(STATUS "HOPS_ENABLE_COVERAGE: overriding build type to Debug (was '${CMAKE_BUILD_TYPE}')")
        set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type forced to Debug for coverage." FORCE)
    endif()

    find_program(LCOV_EXECUTABLE   lcov   REQUIRED)
    find_program(GENHTML_EXECUTABLE genhtml REQUIRED)

    message(STATUS "Coverage enabled: lcov=${LCOV_EXECUTABLE}  genhtml=${GENHTML_EXECUTABLE}")

    set(HOPS_COVERAGE_TEST_EXCLUDE_REGEX "vpal_test_harness[34]_test"
        CACHE STRING "ctest --exclude-regex pattern applied during 'make coverage' runs.")

    set(COVERAGE_FLAGS "--coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${COVERAGE_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_FLAGS}")
    # Only executables need --coverage at link time (pulls in the gcov runtime).
    # Shared libraries must NOT get --coverage here: on many Linux systems libgcov.a
    # is a linker script, and embedding it into a .so causes "file not recognized"
    # errors in downstream link steps. Instrumentation of .so code is handled by
    # the compile flags above; the gcov runtime collects it at load time.
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

    set(COVERAGE_OUTPUT_DIR  "${CMAKE_BINARY_DIR}/coverage_html")
    set(COVERAGE_INFO_FILE   "${CMAKE_BINARY_DIR}/coverage.info")
    set(COVERAGE_CLEAN_FILE  "${CMAKE_BINARY_DIR}/coverage_clean.info")
    set(COVERAGE_INSTALL_DIR "${DOC_INSTALL_DIR}/coverage")

    add_custom_target(coverage
        COMMENT "Collecting and generating lcov coverage report..."

        # Reset counters from any prior run
        COMMAND ${LCOV_EXECUTABLE}
            --zerocounters
            --directory "${CMAKE_BINARY_DIR}"

        # Run the test suite, skipping any tests that match the exclude regex.
        COMMAND ${CMAKE_CTEST_COMMAND}
            --output-on-failure
            --test-dir "${CMAKE_BINARY_DIR}"
            --exclude-regex "${HOPS_COVERAGE_TEST_EXCLUDE_REGEX}"

        # Capture coverage data
        COMMAND ${LCOV_EXECUTABLE}
            --capture
            --directory "${CMAKE_BINARY_DIR}"
            --output-file "${COVERAGE_INFO_FILE}"
            --rc lcov_branch_coverage=0

        # Strip external and system paths. Leading wildcards make these match
        # regardless of how lcov stores absolute vs. relative paths in the tracefile.
        COMMAND ${LCOV_EXECUTABLE}
            --remove "${COVERAGE_INFO_FILE}"
            "*/extern/*"
            "/usr/*"
            --output-file "${COVERAGE_CLEAN_FILE}"
            --rc lcov_branch_coverage=0

        # Produce the combined HTML report
        COMMAND ${GENHTML_EXECUTABLE}
            "${COVERAGE_CLEAN_FILE}"
            --output-directory "${COVERAGE_OUTPUT_DIR}"
            --legend
            --title "HOPS Coverage"

        # --- Split the combined tracefile into HOPS3 / HOPS4 reports -----------
        # --extract keeps only matching files and recomputes totals, so each
        # report carries its own independent percentages.

        # HOPS3 — C sources (source/c_src)
        COMMAND ${LCOV_EXECUTABLE}
            --extract "${COVERAGE_CLEAN_FILE}"
            "*/source/c_src/*"
            --output-file "${CMAKE_BINARY_DIR}/coverage_hops3.info"
            --rc lcov_branch_coverage=0
        COMMAND ${GENHTML_EXECUTABLE}
            "${CMAKE_BINARY_DIR}/coverage_hops3.info"
            --output-directory "${COVERAGE_OUTPUT_DIR}/hops3"
            --legend
            --title "HOPS3 (c_src) Coverage"

        # HOPS4 — C++ sources (source/cpp_src)
        COMMAND ${LCOV_EXECUTABLE}
            --extract "${COVERAGE_CLEAN_FILE}"
            "*/source/cpp_src/*"
            --output-file "${CMAKE_BINARY_DIR}/coverage_hops4.info"
            --rc lcov_branch_coverage=0
        COMMAND ${GENHTML_EXECUTABLE}
            "${CMAKE_BINARY_DIR}/coverage_hops4.info"
            --output-directory "${COVERAGE_OUTPUT_DIR}/hops4"
            --legend
            --title "HOPS4 (cpp_src) Coverage"

        # Install the HTML reports into the doc directory
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${COVERAGE_OUTPUT_DIR}"
            "${COVERAGE_INSTALL_DIR}"

        COMMAND ${CMAKE_COMMAND} -E echo
            "Combined report:  ${COVERAGE_INSTALL_DIR}/index.html"
        COMMAND ${CMAKE_COMMAND} -E echo
            "HOPS3 report:     ${COVERAGE_INSTALL_DIR}/hops3/index.html"
        COMMAND ${CMAKE_COMMAND} -E echo
            "HOPS4 report:     ${COVERAGE_INSTALL_DIR}/hops4/index.html"

        VERBATIM
    )

    # --- Per-library coverage targets -----------------------------------------
    # Generates a `coverage-<lib>` target that runs ONLY the tests under that
    # library's build subdir and captures ONLY that subtree's instrumentation,
    # producing coverage_html/<lib>/index.html.  Scoping both ctest and lcov to
    # the per-library build dir means no test labels are required and a per-lib
    # run does not disturb the counters of any other target.
    function(hops_add_library_coverage NAME SUBDIR)
        add_custom_target(coverage-${NAME}
            COMMENT "Coverage for ${NAME} (${SUBDIR})"
            COMMAND ${LCOV_EXECUTABLE} --zerocounters
                --directory "${CMAKE_BINARY_DIR}/${SUBDIR}"
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
                --test-dir "${CMAKE_BINARY_DIR}/${SUBDIR}"
                --exclude-regex "${HOPS_COVERAGE_TEST_EXCLUDE_REGEX}"
            COMMAND ${LCOV_EXECUTABLE} --capture
                --directory "${CMAKE_BINARY_DIR}/${SUBDIR}"
                --output-file "${CMAKE_BINARY_DIR}/coverage_${NAME}.info"
                --rc lcov_branch_coverage=0
            COMMAND ${LCOV_EXECUTABLE} --remove
                "${CMAKE_BINARY_DIR}/coverage_${NAME}.info" "*/extern/*" "/usr/*"
                --output-file "${CMAKE_BINARY_DIR}/coverage_${NAME}_clean.info"
                --rc lcov_branch_coverage=0
            COMMAND ${GENHTML_EXECUTABLE}
                "${CMAKE_BINARY_DIR}/coverage_${NAME}_clean.info"
                --output-directory "${COVERAGE_OUTPUT_DIR}/${NAME}"
                --legend
                --title "HOPS Coverage: ${NAME}"
            VERBATIM)
    endfunction()

    # Auto-discover libraries: any immediate subdir of c_src/cpp_src that has a
    # test/ subdir gets a coverage-<name> target.  On a name collision between
    # the two trees, the c_src target is suffixed to keep target names unique.
    set(_hops_cov_seen "")
    foreach(_tree c_src cpp_src)
        file(GLOB _libdirs RELATIVE "${CMAKE_SOURCE_DIR}/source/${_tree}"
             "${CMAKE_SOURCE_DIR}/source/${_tree}/*")
        foreach(_lib ${_libdirs})
            set(_libpath "${CMAKE_SOURCE_DIR}/source/${_tree}/${_lib}")
            if(IS_DIRECTORY "${_libpath}" AND IS_DIRECTORY "${_libpath}/test")
                set(_name "${_lib}")
                if("${_name}" IN_LIST _hops_cov_seen)
                    set(_name "${_lib}_${_tree}")
                endif()
                hops_add_library_coverage("${_name}" "source/${_tree}/${_lib}")
                list(APPEND _hops_cov_seen "${_name}")
            endif()
        endforeach()
    endforeach()

endif(HOPS_ENABLE_COVERAGE)
