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

    set(COVERAGE_FLAGS "--coverage -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   ${COVERAGE_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_FLAGS}")
    # Only executables need --coverage at link time (pulls in the gcov runtime).
    # Shared libraries must NOT get --coverage here: on many Linux systems libgcov.a
    # is a linker script, and embedding it into a .so causes "file not recognized"
    # errors in downstream link steps. Instrumentation of .so code is handled by
    # the compile flags above; the gcov runtime collects it at load time.
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

    set(COVERAGE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/coverage_html")
    set(COVERAGE_INFO_FILE  "${CMAKE_BINARY_DIR}/coverage.info")
    set(COVERAGE_CLEAN_FILE "${CMAKE_BINARY_DIR}/coverage_clean.info")

    add_custom_target(coverage
        COMMENT "Collecting and generating lcov coverage report..."

        # Reset counters from any prior run
        COMMAND ${LCOV_EXECUTABLE}
            --zerocounters
            --directory "${CMAKE_BINARY_DIR}"

        # Run the test suite
        COMMAND ${CMAKE_CTEST_COMMAND}
            --output-on-failure
            --test-dir "${CMAKE_BINARY_DIR}"

        # Capture coverage data
        COMMAND ${LCOV_EXECUTABLE}
            --capture
            --directory "${CMAKE_BINARY_DIR}"
            --output-file "${COVERAGE_INFO_FILE}"
            --rc lcov_branch_coverage=1

        # Keep only files under source/ — this excludes extern/, /usr/, and
        # everything else outside the project source tree in one step.
        COMMAND ${LCOV_EXECUTABLE}
            --extract "${COVERAGE_INFO_FILE}"
            "${CMAKE_SOURCE_DIR}/source/*"
            --output-file "${COVERAGE_CLEAN_FILE}"
            --rc lcov_branch_coverage=1

        # Produce the HTML report
        COMMAND ${GENHTML_EXECUTABLE}
            "${COVERAGE_CLEAN_FILE}"
            --output-directory "${COVERAGE_OUTPUT_DIR}"
            --branch-coverage
            --legend
            --title "HOPS Coverage"

        # Install the HTML report into the doc directory
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${COVERAGE_OUTPUT_DIR}"
            "${DOC_INSTALL_DIR}/coverage"

        COMMAND ${CMAKE_COMMAND} -E echo
            "Coverage report installed to ${DOC_INSTALL_DIR}/coverage/index.html"
    )

endif(HOPS_ENABLE_COVERAGE)
