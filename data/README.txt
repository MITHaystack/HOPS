This directory exists to run system-level (rather than unit/coverage tests)
on potentially large blobs of data.  Generally speaking the data is outside
of the git repository and scripts contained within this area can pull the
required data in, or generate a non-fatal test result if not available.

The general organization is as follows:
    bootstrap       scripts for the transition from HOPS3 (svn) to HOPS4 (git)
    switches        directory of scripts to configure for tests
    tarballs        a directory with source data tarballs
    ff_testdata     captured HOPS3 test data (for testing fourfit, &c)
    ae_testdata     captured HOPS3 test data (for testing aedit)
    vgosdata        captured HOPS3 test data (for testing fourfit, &c)
    difx_hops       captured DIFX/HOPS3 regression (baseband data to ff)
    ...             (other collections are likely to come)

Since the test data is binary, and is in any case too large to include in the
git repository, the tarballs area contains empty files indicating the names
of the tarballs that should exist.  Likewise, the other directories are
empty in the repository except for README and test scripts appropriate to
the particular collection of data.

The scripts expect an environment variable MHO_REGRESSION_DATA set to point
to either this source directory or else to a directory with the appropriate
hierarchy populated with either tarballs or data (i.e. a copy of what is in
the repo, with one or more data tarballs present and/or extracted).

A second environment variable MHO_REGRESSION_EXTRACT ( = true or false)
controls the handling of the tarballs and data directories: true means to
extract any tarballs present into the appropriate data directories (so that
the tests may be run); false means to run only with the data present.

A third environment variable MHO_REGRESSION_TIDY ( = true or false)
controls what to do with the extracted data when the test is complete--leave
the data in place or wipe it.

Note that the automake Makefile "check" tests will walk the hierarchy in the
build tree, but use the data in the $MHO_REGRESSION_DATA heirarchy.  Every
"check" test should test an environment variable MHO_REGRESSION_* named
uniquely for the test which evaluates to true (run the test) or false (skip
the test).  Files that setup up appropriate collections of tests are to be
found in the switches directory.

When adding tests or data, add an appropriate environment variable to
both switches/all_true.sh (i.e. run all tests) and switches/all_false.sh.
Tests are generally named chk_something.sh, so "something" should be unique
and related to the MHO_REGRESSION_something variable.  The automake test
driver drops test results into files with .trs appended (with test output
going to a file with .log appended).  A text file bootstrap/requirements.txt
should contain (for every test) zero or more formal requirements that it
is testing.  Additional scripts (bootstrap/req*) can then be created to
provide reports on tests passed (by date) and fractional completion.

Finally switches/provider.sh contains the common machinery to process the
tarballing manipulations.  Each check script can use it by providing it with
the name of an expected data file and the associated tarball.

The automake test system may be replicated into cmake some day.

In addition to the regression data, the tarballs area includes a legacy
subdirectory which includes tarballs of various still-older bits of HOPS
left over from the HOPS2 to HOPS3 transition.  At that time (2010ish) the
build system was converted to the autotools and many bits of Mark3 or Mark4
software used for correlation were abandoned.  The HOPS3 tree also includes
a number of side projects that were part of the general development effort,
but strictly speaking, not part of HOPS as an analysis project.  These also
turn up in the legacy area as they were never part of the HOPS distribution.

eof
