#!/usr/bin/env python3
'''
Test script to exercise the SWIG numpy python interface
'''
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#
import argparse
import sys
import numpy as np

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
    
def first_batch(o):
    if o.verb: print('First batch of testing')
    error = 0
    if o.verb: print('Hello World: %s' % np.version.full_version)
    vis = np.ones(17,dtype=np.complex128)
    if o.verb: print('input vis:    ',vis)
    amp = mhops.var_get_np_amps(vis)
    ok = np.any(np.array(list(map(lambda x: x==1.0, amp))))
    if not ok: error += 1
    if o.verb: print('output amp:   ',amp,ok)
    phs = mhops.var_get_np_phases(vis)
    ok = np.any(np.array(list(map(lambda x: x==0.0, phs))))
    if not ok: error += 1
    if o.verb: print('output phase: ',phs,ok)
    return error

def second_batch(o):
    if o.verb: print('Second batch of testing')
    error = 0
    a = mhops.doubleArray(16)
    p = mhops.doubleArray(16)
    m1 = mhops.MyVis("1st set", a, p, 16)
    m2 = mhops.MyVis("2nd set", a, p, 16)
    d1 = mhops.describe_one_vis(m1)
    d2 = mhops.describe_one_vis(m2)
    if len(d1) < 10: error += 1
    if len(d2) < 10: error += 1
    if o.verb: print('one',d1,'\ntwo',d2)
    ma = mhops.MyVisArray(2)
    ma[0] = m1
    ma[1] = m2
    da = mhops.describe_all_vis(ma.cast(), 2)
    if len(da) < 10: error += 1
    if o.verb: print('all:\n' + da)
    db = str(m1)
    if len(db) < 10: error += 1
    if o.verb: print('attr.one',db)
    return error

# main entry point with some obvious processing steps
if __name__ == '__main__':
  error = 0
  o = parseOptions()
  np.set_printoptions(linewidth=80,precision=4)
  error += first_batch(o)
  error += second_batch(o)
  sys.exit(error)
#
# eof
#
