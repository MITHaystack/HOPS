#!/usr/bin/env python3
'''
Test script to exercise the SWIG numpy python interface
'''
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#
import argparse
import sys

# add the current (build) directory to the PYTHONPATH to find the wrappers
sys.path.append('.')
# and then add the staging .libs area to the PYTHONPATH as well
sys.path.append('.libs')
import mhops

def parseOptions():
    '''
    Build a parser for the command-line options
    '''
    des = 'This is unit test script for the SWIG numpy interface to HOPS'
    des += ''
    epi = 'There is not much more to say about the options'
    epi += ''
    use = '%(prog)s [options]\n'
    parser = argparse.ArgumentParser(epilog=epi, description=des, usage=use)
    parser.add_argument('--version', action='version', version='%(prog)s 0.0')
    parser.add_argument('--description', action='store_true')
    parser.add_argument('-v', '--verbose', dest='verb',
        action='store_true', default=False,
        help='provide some commentary')
    parser.add_argument_group('real options', 'useful options')
    #
    opts = parser.parse_args()
    if opts.description: print(des); sys.exit(0)
    return opts
    

# main entry point with some obvious processing steps
if __name__ == '__main__':
  error = 0
  o = parseOptions()
  if o.verb: print('Hello World')
# print('%d < verbosity < %d' % (mhops.MIN_VERBOSITY, mhops.MAX_VERBOSITY))
# print('%d < severity < %d' % (mhops.MIN_SEVERITY, mhops.MAX_VERBOSITY))
# print('default wrap_progname %s' % mhops.get_wrap_progname())
# print('default wrap_msglevel %d' % mhops.get_wrap_msglevel())
# mhops.wrap_message(1,2,'hello cruel world')
  sys.exit(error)
#
# eof
#
