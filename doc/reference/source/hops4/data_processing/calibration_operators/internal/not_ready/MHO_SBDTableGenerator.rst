MHO\_SBDTableGenerator
======================

Purpose
-------
``MHO_SBDTableGenerator`` is an internal utility operator that prepares a
single-band-delay (SBD) table container for use by the NormFX fringe-fitting
pipeline. It creates an output ``sbd_type`` container sized appropriately for
FFT-based delay search by padding the frequency axis. It generates axis labels
for the SBD output and copies metadata from the input visibility container.

Control File Trigger
--------------------
This operator is internal and has no dedicated control-file keyword.
It is instantiated automatically as part of the NormFX operator chain
(both single-sideband and mixed-sideband variants) when visibility data is
being prepared for single-band-delay transformation.

Input Data
----------
The operator inherits from ``MHO_TransformingOperator`` and takes two arguments: an input ``visibility_type`` container (4D axis pack: polarization-product x channel x time x frequency) and an output ``sbd_type`` container (which is typedef'd to the same ``visibility_type`` structure).

Algorithm
---------
``MHO_SBDTableGenerator`` is a transforming operator with a meaningful ``InitializeImpl`` but a no-op ``ExecuteImpl``.

**Initialization (``InitializeImpl``):**

1. Verify both input and output pointers are non-null.
2. Call ``ConditionallyResizeOutput`` to check whether the output ``sbd_type`` container needs to be resized.
3. Set the internal ``fInitialized`` flag to ``true``.

**Conditional resize (``ConditionallyResizeOutput``):**

1. Retrieve the dimensions of both the input visibility and output SBD containers.
2. Compare axis-by-axis. The frequency axis (``FREQ_AXIS``, axis index 3) is expected to be padded by a factor of 4 (defined by ``PADDING_FACTOR``). All other axes must match exactly:

   .. math::

       \begin{aligned}
       \text{expected: } & \text{sbd\_dim}[FREQ\_AXIS] = 4 \times \text{vis\_dim}[FREQ\_AXIS] \\
       \text{for all other axes: } & \text{sbd\_dim}[i] = \text{vis\_dim}[i]
       \end{aligned}

3. If any axis mismatch is detected, the output container is resized:

   a. Copy the input dimensions into the output dimension array.
   b. Set ``sbd_dim[FREQ_AXIS]`` to $4 \times \text{vis\_dim}[FREQ\_AXIS]$ (required by the NormFX FFT implementation).
   c. Call ``Resize`` on the output container and zero all elements.
   d. Copy the polarization-product axis (``POLPROD_AXIS``), channel axis (``CHANNEL_AXIS``), and time axis (``TIME_AXIS``) from the input container to the output container.

4. The frequency axis (``FREQ_AXIS``) is *not* copied from the input; it is left to be populated by the zero-padder and FFT stages of the NormFX pipeline.

**Execution (``ExecuteImpl``):**

This is a no-op. The operator returns ``true`` if ``fInitialized`` is ``true``, and ``false`` otherwise. No data transformation occurs during execution.

Effect on Data
--------------
The operator does not transform visibility values. It ensures the output
``sbd_type`` container has the correct dimensions for downstream NormFX
processing: the frequency axis is padded by a factor of 4 relative to the input,
while all other axes are unchanged. The polarization-product, channel, and
time axes are copied from the input container, preserving their labels and
metadata. The frequency axis labels are not copied; they will be generated
by the subsequent zero-padding and FFT stages. If the output container 
already has the correct dimensions, no resize or copy is performed.
