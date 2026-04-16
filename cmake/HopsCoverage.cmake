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
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    --coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")

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

        # Strip external and system paths from the report
        COMMAND ${LCOV_EXECUTABLE}
            --remove "${COVERAGE_INFO_FILE}"
            "${CMAKE_SOURCE_DIR}/extern/*"
            "/usr/*"
            "*/test/*"
            --output-file "${COVERAGE_CLEAN_FILE}"
            --rc lcov_branch_coverage=1

        # Produce the HTML report
        COMMAND ${GENHTML_EXECUTABLE}
            "${COVERAGE_CLEAN_FILE}"
            --output-directory "${COVERAGE_OUTPUT_DIR}"
            --branch-coverage
            --legend
            --title "HOPS Coverage"

        COMMAND ${CMAKE_COMMAND} -E echo
            "Coverage report written to ${COVERAGE_OUTPUT_DIR}/index.html"
    )

endif(HOPS_ENABLE_COVERAGE)
