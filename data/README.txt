General plan for managing regression test data

This directory exists to run system-level (rather than unit/coverage tests)
on potentially large blobs of data.  Generally speaking the data is outside
of the git repository and scripts contained within this area can pull the
required data in, or generate a non-fatal test result if not available.
However, many unit tests may also require substantial blobs of data as well.

The general organization is as follows:
    bootstrap       scripts for the transition from HOPS3 (svn) to HOPS4 (git)
    switches        directory of scripts to configure for tests
    tarballs        a directory with source data tarballs
        legacy          HOPS SVN tarballs (see legacy_tar.sh)
        LGPL            redistributable source tarballs
        nonLGPL         nonredistributable source tarballs (for reference)
  and these directories (which may be empty except for 'stub'):
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
to a directory with the structure of this source directory, but with some or
all of the tarballs present.  (That is, you can point to the this directory
or a copy of it to conduct a SKIP-all-tests test suite.)  Obviously the more
interesting case is when it points to a directory fully populated with the
data tarballs.  The switches subdirectory contains a number of files that
control what will actually happen in the test suite.  If there is a file
switches/test_config.sh then that script is sourced to configure the suite.
If the file does not exist, the associated tests will have an ERROR.  If
an environment variable MHO_REGRESSION_CONFIG is set, that is used in place
of the file $MHO_REGRESSION_DATA/switches/test_config.sh.

Note that the GNU automake test-driver looks at the return status of any
script (or executable) to determine its result: 0 PASS, 77 SKIP, 99 ERROR
and anything else is a FAIL.  This is recorded in the ":test-result:" line
of the .trs file.  (Note that the GNU folks have a general API to allow other
testrunners; there is considerable documentation at https://www.gnu.org/
software/automake/manual/html_node/API-for-Custom-Test-Drivers.html.)

A variable MHO_REGRESSION_EXTRACT ( = true or false) controls the handling
of the tarballs and data directories: true means to extract any tarballs
present into the appropriate data directories (so that the tests may be run);
false means to run only with the data already present.

A variable MHO_REGRESSION_TIDY ( = true or false) controls what to do with
the extracted data once the test is complete--leave the data in place or
wipe it (to save space).  Generally speaking, a 'nightly' build process
would benefit from MHO_REGRESSION_TIDY=false to speed things up.

Tests are generally named chk_something.sh, so "something" should be unique.
The automake test driver drops test results into files with .trs appended
(with test output going to a file with .log appended--if you turn on verbosity
in the scripts that will turn up in the .log file).

Since the purpose of some of the tests is to track the satisfaction of
our formal requirements for the MSRI project, we need a way to associate
and report the success of tests relative to the requirements.  Thus a
text file bootstrap/requirements.txt will track the association of check
tests and requirements satisfaction.  (I.e. providing the association of
"something" with one or more requirements in the list.)  A reporting script
bootstrap/req_report.sh can then look at the .trs files and provide a
summary report for management purposes.

You may set an environment variable MHO_REGRESSION_REQUIREMENTS to specify
an alternate path for the requirements file.  (You may want to do this when
working on the tree to satisfy one set of tests.)

The CMake version of testing should (eventually) have similar behavior (as
it should be using the same scripts to do the actual tests; both automake
and CMake should be merely arranging for running a list of TESTS and coping
with the output).

For access to required data, a file switches/provider.sh contains the
common machinery to process the tarballing manipulations.  Each check
script can use it by providing it with arguments to indicate the required
tarballs.  The actual mechanics of creating the tarballs is captured in
bootstrap/legacy_tar.sh.

To maintain some MHO_REGRESSION_DATA area, script bootstrap/legacy_update.sh
may be used to mirror what is in the GIT repo with a target data directory.
(That is, the target contains all the scripts of the GIT repo together with
whatever tarballs and/or data that is desired.)

Test scripts

The script switches/template.sh is a template to copy and then modify so
that all of the test scripts have a common behavior.  See the comments in
that script.  The tests themselves are then distributed throughout the
build tree.  Every makefile can provide $srcdir, $abs_top_srcdir or
$abs_top_builddir in the environment for any test in case the script
needs to reference sources.  In the case of the automake system this is
provided by an the TESTS_ENVIRONMENT make variable.  The template script
verifies that if they are set that they do point to directories.

For convenience, this script accepts an argument which is the "something"
for a new test.  It then creates chk_something.sh with a reduced set of
comments so that the tests aren't cluttered with the same set of comments.
Feel free to edit or even rename to suit your needs.  Generally speaking
after creating a new script, you need to also set the $tarballs and
$executable variables and (obviously) give the test something to check.
There are some 'FIXME' tokens left to remind you.

Access to built objects

Historically HOPS3 followed a make all install check order of operations.
This assumes that the install area is not otherwise used (which in the
event was not a very good assumption).  A more proper order is to have
make all check install which means that the install does not happen unless
the check succeeds.  However, since libraries and executables are located
in the build directory until installed, this means that scripts using them
will need to know the relative location of the tools they need.  (This is
necessary also for the linker flags.)  An alternative would be to have a
tool at the toplevel which can do the appropriate lookup.  This can be
handled in the test_config file by setting a variable executables for
the list of built executables, on exit, this variable would be updated
by the full paths to these.

This process can be made easier with a lookup command which defaults
to $MHO_REGRESSION_DATA/switches/lookup.sh but can be overridden by
setting $MHO_REGRESSION_LOOKUP.  The current mechanism requires that
$top_abs_builddir be set if you need to use this machinery.

Related things

In addition to the regression data, the tarballs area includes a legacy
subdirectory which includes tarballs of various still-older bits of HOPS
left over from the HOPS2 to HOPS3 transition.  At that time (2010ish) the
build system was converted to the autotools and many bits of Mark3 or Mark4
software used for correlation were abandoned.  The HOPS3 tree also includes
a number of side projects that were part of the general development effort,
but strictly speaking, not part of HOPS as an analysis project.  These also
turn up in the legacy area as they were never part of the HOPS distribution.

It is expected that the final few point distributions of HOPS3 (3.23 onwards)
should be tarballed in this fashion so that when the SVN (supported from
vault.haystack.mit.edu) is decommissioned we retain history to the start
of the HOPS4 project.

Also in the directory are tarballs of specific tools for reference.  See
the readme.txt files in tarballs/LGPL and tarballs/nonLGPL for more info.

eof
