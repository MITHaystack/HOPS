import pyMHO_Containers

import numpy as np


def test_noema(cstore_interface_obj, param_interface_obj):

    print("RUNNING NOEMA PHASE JUMP CORRECTION")

    #grab the UUID of the visibility object
    vis_uuid = param_interface_obj.get_by_path("/uuid/visibilities");
    visib_obj = cstore_interface_obj.get_object(vis_uuid);

    if visib_obj is None:
        return #bail out

    #figure out if NOEMA is reference or remote station
    stidx = 0
    ref_id = param_interface_obj.get_by_path("/ref_station/site_id");
    rem_id = param_interface_obj.get_by_path("/rem_station/site_id");
    if ref_id == "Nn":
        stidx = 0
    if rem_id == "Nn":
        stidx = 1

    #grab the underlying visibility 4-d array
    vis_arr = visib_obj.get_numpy_array();
    #rank = visib_obj.get_rank()

    #grab the axis information we care about
    axis0 = visib_obj.get_axis(0); #get the polprod axis
    axis1 = visib_obj.get_axis(1); #get the channel axis
    axis3 = visib_obj.get_axis(3); #get the spectral point axis (sub-channel)
    chan_meta_data = visib_obj.get_axis_metadata(1) #channel axis meta data object
    channel_info = chan_meta_data["index_labels"] #channel bin label dict

    #hacky dictionaries of phase jumps for NOEMA channels
    #first number is intra-channel frequency, second is phasor correction
    #roughly estimated from fringe plots
    jumps_l = {
        "a":[25.0, -1.0],
        "b":[30.5, 1.0j],
        "d":[42.0, -1.0],
        "e":[46.0, -1.0],
        "f":[52.0, -1.0j],
        "i":[4.2, -1.0],
        "j":[9.5, -1.0],
        "k":[15.0, -1.0],
        "m":[25.8, -1.0],
        "n":[31.2, -1.0],
        "o":[36.5, 1.0j],
        "p":[42.0, -1.0],
        "q":[47.6, -1.0],
        "r":[53.0, -1.0],
        "u":[5.0, -1.0],
        "v":[10.0, -1.0],
        "w":[16.0, -1.0],
        "x":[21.6, 1.0j],
        "y":[26.5, -1.0j],
        "z":[32.0, -1.0],
        "A":[37.6, -1.0]
    }

    jumps_r = {
        "a":[25.0, -1.0],
        "b":[30.5, 1.0],
        "c":[36.0, -1.0j],
        "d":[42.0, -1.0],
        "e":[46.0, -1.0],
        "f":[52.0, 1.0j],
        "i":[4.2, -1.0],
        "j":[9.5, -1.0],
        "k":[15.0, 1.0j],
        "l":[20.0, -1.0j],
        "m":[25.8, -1.0],
        "n":[31.2, -1.0],
        "o":[36.5, 1.0j],
        # "p":[42.0, 1.0],
        "q":[47.6, -1.0],
        "r":[53.0, -1.0],
        "u":[5.0, -1.0j],
        "v":[10.0, -1.0],
        "w":[16.0, -1.0],
        "x":[21.6, 1.0j],
        "y":[26.5, 1.0],
        "z":[32.0, -1.0],
        "A":[37.6, -1.0]
    }

    #oddly enough when NOEMA is the remote-station in the baseline, some sign flips happend
    #why are these additional jumps/flips needed?
    if stidx == 1:
        #L pol
        jumps_l["b"][1] = -1j #the odd ball
        jumps_l["o"][1] *= -1
        jumps_l["x"][1] *= -1
        jumps_l["y"][1] *= -1
        #R pol
        jumps_r["c"][1] *= -1
        jumps_r["f"][1] *= -1
        jumps_r["k"][1] *= -1
        jumps_r["o"][1] *= -1
        jumps_r["u"][1] *= -1
        jumps_r["x"][1] *= -1

    #stash both pols in one object
    jumps = dict()
    jumps["R"] = jumps_r
    jumps["L"] = jumps_l

    #now apply the corrections to the visibilities, looping over pol-prod and channel
    nspectral = visib_obj.get_dimension(3) # number of spectral points in channel
    for pp in range(0, len(axis0)):
        polprod = axis0[pp]
        pol = polprod[stidx:stidx+1]
        for ch in range(0,len(axis1)):
            chan_key = str(ch) #meta data keys must be strings (e.g. "1", "2") not integers
            fourfit_chan_label = channel_info[chan_key]["channel_label"] #fourfit label
            if pol in jumps and fourfit_chan_label in jumps[pol]:
                jump_freq = jumps[pol][fourfit_chan_label][0]
                jump_phasor = jumps[pol][fourfit_chan_label][1]
                #now apply the phase shift at/after the jump freq
                for sp in range(0,nspectral):
                    freq = axis3[sp]
                    if freq > jump_freq:
                        vis_arr[pp, ch, :, sp] *= jump_phasor
