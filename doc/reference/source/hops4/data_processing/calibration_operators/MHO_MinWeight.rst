MHO\_MinWeight
==============

Purpose
-------
``MHO_MinWeight`` implements a weight-based selection filter that zeroes out
all weight values below a user-specified minimum threshold. This operator
effectively removes low-quality visibility data points from downstream
processing by driving their weights to zero.

Control File Trigger
--------------------
- **Keyword:** ``min_weight``
- **Category:** selection
- **Priority:** 3.5

.. list-table:: Parameters for ``min_weight``
   :header-rows: 1

   * - Parameter
     - Type
     - Description
   * - value
     - real
     - Minimum weight threshold; all weights below this value are set to zero.

Input Data
----------
This operator acts on the ``weight_type`` container.

Algorithm
---------
``MHO_MinWeight`` has no ``Initialize`` method; all work occurs in ``ExecuteInPlace``.

The builder (``MHO_MinWeightBuilder``) retrieves the weight container from the
container store, and skips construction entirely if the configured value is zero.

**Execution (``ExecuteInPlace``):**

1. Iterate over every element in the weight array.
2. For each weight ``w``, compare against the threshold: ``if w < w_min, w = 0``.

Effect on Data
--------------
The operator modifies the ``weight_type`` container in place. All weight values
strictly less than the configured minimum threshold are set to zero. Zeroed
weights cause downstream operators to exclude the corresponding visibility data
points from fringe-fitting, averaging, or any weighted computation.
