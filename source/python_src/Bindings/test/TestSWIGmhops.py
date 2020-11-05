#!/usr/bin/env python3
#
# Simple test script to exercise the SWIG python interface
#
import sys
# add the current (build) directory to the PYTHONPATH to find the wrappers
sys.path.append('.')
# and then add the staging .libs area to the PYTHONPATH as well
sys.path.append('.libs')
import mhops

# main entry point with some obvious processing steps
if __name__ == '__main__':
  error = 0
  print('Hello World')
  print('verbosity < %d' % (mhops.MAX_VERBOSITY))
  print('%d < severity < %d' % (mhops.MIN_SEVERITY, mhops.MAX_VERBOSITY))
  print('default wrap_progname %s' % mhops.get_wrap_progname())
  print('default wrap_msglevel %d' % mhops.get_wrap_msglevel())
  mhops.wrap_message(1,2,'hello cruel world')
  sys.exit(error)
#
# eof
#
