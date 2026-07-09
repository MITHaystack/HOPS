MHO\_MBDelaySearch
==================

Purpose
-------
``MHO_MBDelaySearch`` performs a coarse three-dimensional search for the
multi-band delay (MBD), single-band delay (SBD), and delay rate (DR) that
maximize the fringe amplitude. It transforms visibility data through delay-rate
space and multi-band delay space using FFTs, then locates the global maximum
in the resulting 2D search surface for each SBD lag. This operator is the core
of the fringe-fitting coarse search (the algorithm is described in greater detail in
:doc:`the broadband fringe-fitting algorithm reference </hops4/data_processing/algorithm_and_output/algorithm>`).

Control File Trigger
--------------------
This operator is an internal utility with no control file keyword.
It is invoked by the fringe-fitting pipeline after delay-rate rotation to perform the initial coarse peak search.

Input Data
----------
This operator acts on the ``visibility_type`` container.
It also requires a ``weight_type`` container provided via ``SetWeights``, and
a reference frequency set via ``SetReferenceFrequency``.

Algorithm
---------
**Initialization (``InitializeImpl``):**

1. Compute a unique frequency grid from the channel frequencies. Duplicate frequencies (e.g., from double-sideband channel pairs sharing a sky frequency) are merged using a tolerance of ``eps = 10^-4`` MHz. The ``MHO_UniformGridPointsCalculator`` derives the grid start, spacing, number of grid points, and average frequency, along with a channel-to-grid-index mapping.
2. Set the number of SBD bins (``N_SBD``) from the FREQ axis dimension, and the number of time bins (``N_DR``) from the TIME axis dimension.
3. Extract the accumulation period ``Delta t_AP`` from the time axis spacing.
4. Resize workspace containers: the MBD workspace (size ``N_grid``) and the SBD-DR workspace (matching the input visibility shape with pol-product and frequency axes collapsed to 1).
5. Initialize the ``MHO_DelayRate`` operator to transform visibility slices into delay-rate space.
6. Initialize the FFT engine for MBD axis label setup and the batched 2D FFT engine for transforming the ``[N_DRSP x N_grid]`` search buffer along the MBD axis.
7. Initialize the cyclic rotator to shift the MBD axis by ``N_grid/2`` bins (standard FFT output reordering).

**Execution (``ExecuteImpl``):**

1. Initialize the global maximum ``f_max = 0``.
2. Loop over each SBD lag ``sbd_idx = 0 .. N_SBD-1``:

   a. Check the SBD window filter: if a window is set, skip SBD values outside the configured range.

   b. Extract the visibility slice for the current SBD lag into the SBD-DR workspace.

   c. Transform the slice into delay-rate space using ``MHO_DelayRate::Execute``.

   d. On the first SBD iteration, set up the delay-rate axis (converting fringe rate to delay rate by dividing by reference frequency) and the MBD axis (via a dummy FFT for axis-label transformation).

   e. Zero the 2D ``[N_DRSP x N_grid]`` search buffer, then scatter-accumulate: for each channel ``ch``, add the delay-rate-transformed visibility to the corresponding MBD bin (determined by the precomputed ``fMBDBinForChannel`` lookup):

       | buffer(dr_idx, MBD_bin[ch]) += sbd_dr_data(0, ch, dr_idx, 0)

   f. Execute the batched FFT along the MBD axis (axis 1) over all DR slices simultaneously.

   g. If coherence time ``t_cohere > 0``, apply incoherent box-car smoothing along the delay-rate dimension:

       | n_half = round(N_DRSP * Delta t_AP / (2 * t_cohere))

       For each DR bin ``dr`` and each MBD bin ``mbd``:

       | buffer(dr, mbd) = mean(\|buffer(j, mbd)\|) for j in [dr-n_half .. dr+n_half]

       (boundary-clamped, not wrap-around).

   h. Search the 2D result for the global maximum. For each (DR, MBD) pair within the configured windows:

       | tmp = norm(buffer(dr_idx, mbd_idx))

       If ``tmp > f_max``, record the maximum value and the corresponding bin indices (applying cyclic-shift correction to the MBD index):

       | MBDMaxBin = (mbd_idx + N_grid/2) % N_grid

3. After the SBD loop, apply the cyclic rotation to properly order the MBD axis.
4. Convert the maximum from squared norm to amplitude: ``f_max = sqrt(f_max)``.
5. Compute coarse physical values from the peak bin indices:

   | CoarseMBD = MBDAxis(MBDMaxBin)
   | CoarseSBD = SBDAxis(SBDMaxBin)
   | CoarseDR = DRAxis(DRMaxBin)

Effect on Data
--------------
This operator does not modify any input data. It produces scalar outputs
accessible via getter methods: the coarse multi-band delay (``GetCoarseMBD``),
coarse single-band delay (``GetCoarseSBD``), coarse delay rate (``GetCoarseDR``),
the corresponding peak bin indices, the search maximum
amplitude (``GetSearchMaximumAmplitude``), and the number of points
searched (``GetNPointsSearched``). The MBD and DR axes are also
accessible for downstream use.
