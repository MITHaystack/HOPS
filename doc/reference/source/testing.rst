========================
Testing and Verification
========================

In order to run the test suite after installation, you must first download the test data with the ``testdata_download_all.sh`` script:

.. code-block:: bash

   cd <hops-git>
   cd ./build
   source <hops-install>/bin/hops.bash
   testdata_download_all.sh
   make test

Note, that the vast majority of the test-suite currently requires HOPS3, so its dependencies should be
installed if you plan on running the tests.
A terminal width of at least 95 characters is needed to avoid line wrap in the test output.

Building this Documentation
----------------------------

HOPS can automatically build this reference documentation using ``doxygen`` and ``sphinx``. To do
so, ensure that ``doxygen``, ``sphinx``, and the Python packages ``breathe`` and ``myst_parser`` are
installed, and that the CMake option ``HOPS_BUILD_DOCS`` is set to ``ON``. Then, from the build
directory, run:

.. code-block:: bash

   make reference

The resulting HTML documentation will be installed to ``<hops-install>/doc/reference``, with the
master index file at ``<hops-install>/doc/reference/index.html``.
