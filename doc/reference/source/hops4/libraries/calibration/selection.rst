Selection Operators
===================

MHO_MinWeight
-------------

======================= ======================================================
Class                   :hops:`MHO_MinWeight`
Operator Type           Unary
Operator Category       selection
Argument Data Type      :hops:`weight_type`
Priority Value          3.5
Control File Keyword    ``min_weight``
======================= ======================================================

Given a minimum allowed threshold for the data weights, removes data with a 
weight less than this threshold. This is done trivially by zeroing out the value 
for all weights less than the threshold.