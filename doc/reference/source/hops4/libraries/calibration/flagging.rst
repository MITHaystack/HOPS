Flagging Operators
==================

MHO_DCBlock
-----------

======================= ======================================================
Class                   :hops:`MHO_DCBlock`
Operator Type           Unary
Operator Category       flagging
Argument Data Type      :hops:`visibility_type`
Priority Value          3.5
Control File Keyword    ``dc_block``
======================= ======================================================

Zero out the (single) DC spectral point for each channels in the visibility data.


MHO_Notches
-----------

======================= ======================================================
Class                   :hops:`MHO_Notches`
Operator Type           Unary
Operator Category       flagging
Argument Data Type      :hops:`visibility_type`
Priority Value          4.5
Control File Keyword    ``notches``
======================= ======================================================

Operator which 'notches' out problematic chunks of visibilities in frequency 
space by filtering out specified frequency ranges. 
Visibility data within a notch is set to zero, and corresponding weights
are adjusted accordingly.


MHO_Passband
------------

======================= ======================================================
Class                   :hops:`MHO_Passband`
Operator Type           Unary
Operator Category       flagging
Argument Data Type      :hops:`visibility_type`
Priority Value          4.5
Control File Keyword    ``passband``
======================= ======================================================

Selects a chunk of frequency space for inclusion or removal based on passband
boundaries (effectively a single notch or window).