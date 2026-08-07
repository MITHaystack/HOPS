Frequency Domain Processing
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The frequency domain processing components provide builders for operators that
handle frequency-domain filtering, correction, and processing operations in
VLBI fringe fitting.

:hops:`MHO_NotchesBuilder`
--------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_NotchesBuilder`
Primary Functionality                           Builds a notches (frequency cut) operator
Key Features                                    | Constructs and adds MHO_Notches operator to toolbox
                                                | Implements frequency domain filtering/cutting
                                                | Removes specific frequency ranges from data
=============================================== ====================================================================

The :hops:`MHO_NotchesBuilder` class builds a notches operator that performs frequency
domain filtering by removing specific frequency ranges from the data. This can be used
for eliminating radio frequency interference (RFI) and other unwanted signals that
can contaminate VLBI observations.

:hops:`MHO_PassbandBuilder`
---------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_PassbandBuilder`
Primary Functionality                           Builds a passband (frequency chunk excision) operator
Key Features                                    | Constructs and initializes passband operator
                                                | Handles frequency domain filtering/excision
                                                | Defines usable frequency ranges
=============================================== ====================================================================

The :hops:`MHO_PassbandBuilder` class builds a passband operator that defines the
usable frequency ranges for fringe fitting by excising frequency chunks that should
not be used in the analysis. This is complementary to the notches operator and
helps define the clean frequency ranges for processing.

:hops:`MHO_DCBlockBuilder`
--------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_DCBlockBuilder`
Primary Functionality                           Builds a DC block operator
Key Features                                    | Constructs and adds MHO_DCBlock operator to toolbox
                                                | Inherits from MHO_OperatorBuilder
                                                | Implements DC blocking functionality
                                                | Removes DC component from frequency domain data
=============================================== ====================================================================

The :hops:`MHO_DCBlockBuilder` class builds a DC block operator that removes the
DC component from frequency domain data. This is important for eliminating DC
offsets that can affect the quality of fringe fitting and correlation analysis.

The builder constructs the MHO_DCBlock operator and adds it to the processing
toolbox for frequency domain DC removal.

:hops:`MHO_LSBOffsetBuilder`
----------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_LSBOffsetBuilder`
Primary Functionality                           Builds a LSB (lower side band) offset operator
Key Features                                    | Constructs and initializes MHO_LSBOffset operator
                                                | Uses the inherited GetMatchingStationIdentifiers() mechanism for station identification
                                                | Handles lower side band offset corrections
                                                | Compensates for frequency-dependent effects
=============================================== ====================================================================

The :hops:`MHO_LSBOffsetBuilder` class builds a lower side band (LSB) offset operator
that handles frequency-dependent corrections for the lower side band in heterodyne
receiver systems.

The builder uses station-matching functionality inherited from the shared
`MHO_OperatorBuilder` base class (`GetMatchingStationIdentifiers()`) to ensure
corrections are applied to the appropriate station's data streams.

:hops:`MHO_NotchCombBuilder`
-----------------------------

=============================================== ====================================================================
Class                                           :hops:`MHO_NotchCombBuilder`
Primary Functionality                           Builds a notch comb (repeating frequency cut) operator
Key Features                                    | Constructs and adds MHO_NotchComb operator to toolbox
                                                | Implements a repeating/comb pattern of frequency notches
=============================================== ====================================================================

The :hops:`MHO_NotchCombBuilder` class builds a notch comb operator that applies a
repeating (comb) pattern of frequency notches across the data, complementary to the
single-notch functionality of :hops:`MHO_NotchesBuilder`.
