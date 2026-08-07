.. _InitializationLibrary:

Initialization
==============

The Initialization library provides a the primary mechanism for building and configuring
operators used in VLBI fringe fitting operations. This library implements the builder pattern
to create various correction operators, data labeling/processing components, and
other management utilities required for VLBI data processing in the HOPS4 framework.
Each builder provides a Build() method that constructs and configures the associated operator object
and returns a boolean indicating the success of the construction process.

.. toctree::
   :maxdepth: 2

   data_labeling_organization
   phase_delay_corrections
   polarization_processing
   frequency_domain_processing
   data_selection_filtering
   system_management_configuration
