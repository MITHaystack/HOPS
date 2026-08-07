MHO\_FringeFitter
==================

Purpose
-------

``MHO_FringeFitter`` is the abstract base class that orchestrates the entire
fringe-fitting pipeline in HOPS4. It provides a structured, step-driven
lifecycle for configuring, initializing, executing, and finalizing a fringe-fit
operation. All concrete fringe-fitting algorithms (such as the standard
three-dimensional fringe search and the ionospheric fringe fitter) derive
from this class and implement the pure virtual methods to define their specific
search strategy. The class also supports the Visitor pattern, allowing external
extensions (e.g., Python bindings, profiling hooks) to observe or modify the
fitter's behavior at each step.

Control File Trigger
--------------------

This class is not directly triggered by a control-file keyword.
It is instantiated and driven by the fringe-fitting application
(e.g., the ``fourfit4`` binary), which reads the control file,
constructs the appropriate derived fitter, and invokes the lifecycle methods
in sequence. The overall execution follows the pattern:

.. code-block:: none

   Configure -> Initialize -> [PreRun, Run, PostRun] (repeated) -> Finalize

Input Data
----------

The fitter holds pointers to three core data objects, all obtained from
an ``MHO_FringeData`` container passed at construction:

- **MHO_ParameterStore\*** -- stores configuration parameters, fit results, and metadata indexed by string keys (e.g., ``/config/*``, ``/fringe/*``, ``/vex/*``)
- **MHO_ScanDataStore\*** -- provides access to the scan's raw data files (Mark4/HOPS binary data) and the root VEX/JSON header describing the observation
- **MHO_ContainerStore\*** -- holds in-memory data containers such as visibility and weight objects

The fitter also maintains an ``MHO_OperatorToolbox``, which stores and manages
all calibration, flagging, and pre-fit data-operator objects. Operators are
organized by category and priority, allowing the fitter to retrieve and execute
subsets of operators in a defined order.

Algorithm
---------

The fringe-fitter lifecycle consists of seven phases, each implemented as a pure virtual method in the base class and concretely realized by derived classes:

1. **Configure**: Sets up the fitter's internal state from the parameter store. This includes reading control-file parameters, determining the search space (delay, delay-rate, phase), and building the operator pipeline. The ``MHO_OperatorBuilderManager`` is typically constructed during this phase.

2. **Initialize**: Prepares all data objects and operators for the first iteration. This may include loading visibility data, building fringe-search tables (e.g., the SBD -- search-by-delay -- tables), and performing any one-time data transformations.

3. **PreRun**: Executed before each search iteration. Derived classes may use this phase to reset temporary state, prepare data views, or apply pre-fit operators (such as flagging, passband correction, or polarization products).

4. **Run**: The core search phase. For the standard three-dimensional fringe search, this iterates over delay, delay-rate, and phase, computing the fringe amplitude at each grid point (or sub-grid peak). The operator toolbox is invoked within this phase to apply calibration corrections to the visibility data before the fringe search.

5. **PostRun**: Executed after each search iteration. Derived classes may use this phase to cache intermediate results, evaluate convergence criteria, or update fit parameters.

6. **IsFinished**: Returns a boolean indicating whether the search is complete. For multi-iteration strategies (e.g., hierarchical coarse-to-fine search), this method controls the loop termination.

7. **Finalize**: Cleans up resources, writes the final fit results to the parameter store (e.g., ``/fringe/delay``, ``/fringe/delayrate``, ``/fringe/phase``), and prepares output data structures for downstream processing.

The base class also provides two optional protected methods, ``Cache()`` and ``Refresh()``, which allow derived classes to store and restore visibility/weight data before and after in-place modifications by flagging or calibration operators.

The ``Accept()`` method implements the Visitor pattern: a derived fitter calls
this method to allow a ``MHO_FringeFitterVisitor`` to inspect or modify the
fitter's state. This is used by extension frameworks (such as the Python
bindings) to hook into the fitter's lifecycle without modifying the core class
hierarchy.

Effect on Data
--------------

The ``MHO_FringeFitter`` base class itself does not modify data; it is the
derived classes that perform the actual fringe search and store results.
Over the course of a complete fringe-fit operation, the fitter's lifecycle
reads visibility and weight data from the container store, applies calibration
and flagging operators in-place, performs the fringe search, and writes the
final fit parameters (delay, delay-rate, phase, amplitude, SNR, etc.) to the
parameter store. The operator toolbox may be reconfigured between iterations,
allowing the fitter to adapt its calibration pipeline based on intermediate
results.
