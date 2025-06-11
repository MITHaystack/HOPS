..  _msg:

msg
===

The msg library controls the command line output of various programs. The only 
parameter accessible to the user is the message level. The message level for hops3 
programs is an integer, where the lower the integet the more verbose the output will be.
For example a level of '-1' will print a large variety of debug information, whereas
'5' will be silent.

.. doxygenfunction:: msg
  :project: hops

.. doxygenfunction:: set_progname
   :project: hops

.. doxygenfunction:: get_progname
  :project: hops

.. doxygenfunction:: set_msglev
  :project: hops
