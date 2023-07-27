#
# This is an example of python helper code
# (c) Massachusetts Institute of Technology, 2020
# The contents of the package Copyright statement apply here.
#
# The SWIG interfaces tend not to be Pythonic, but writing some
# simple wrappers makes them more useful.  These Python fragments
# will be appended so that callers have less obscure calls.  It
# is also a place for checking, since SWIG errors are likely not
# sensible to the typical user.
#
# Per the include/npary_demo.i file, these fragments turn up after
# the generated python wrapper definitions.
#

def var_get_np_amps(vis):
    '''
    Extract amplitudes from a numpy array of visibilities.
    '''
    try: # to create a numpy object for the answer
        amps = np.ones(vis.shape[0], dtype=np.double)
    except:
        raise Exception('argument is not a numpy array')
    if type(vis[0]) != np.complex128:
        raise Exception('argument is not complex numpy array')
    get_np_amps(vis, amps)
    return amps

def var_get_np_phases(vis):
    '''
    Extract phases from a numpy array of visibilities.
    '''
    try: # to create a numpy object for the answer
        phases = np.ones(vis.shape[0], dtype=np.double)
    except:
        raise Exception('argument is not a numpy array')
    if type(vis[0]) != np.complex128:
        raise Exception('argument is not complex numpy array')
    get_np_phases(vis, phases)
    return phases

# these duplicate SWIG methods with similar names
def get_vis_amps(vis):
    return(doubleArray_frompointer(vis.amps))
setattr(MyVis, 'get_amps', get_vis_amps)

def get_vis_phas(vis):
    return(doubleArray_frompointer(vis.phas))
setattr(MyVis, 'get_phas', get_vis_phas)

def describe_vis(vis):
    return(describe_one_vis(vis))
setattr(MyVis, '__str__', describe_vis)

# cannot do this with the MyVisArray, as the length is not available
# i.e. MyVisArray is essentially a pointer to the first MyVis

#
# eof
#
