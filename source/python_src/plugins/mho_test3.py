import pyMHO_Containers

import numpy as np
import scipy.stats
from vpal import utility

def generate_pcphases(plot_data):

    #get channel label and phase
    ch_labels = plot_data["PLOT_INFO"]["#Ch"]
    ch_phase = plot_data["PLOT_INFO"]["Phase"]
    # print("ch_labels", ch_labels)
    # print("ch_phase", ch_phase)

    nchan = len(ch_labels) - 1
    #actually stored in the type 210's and what their fourfit names are
    phase_residuals = dict()
    for i in list(range(0, nchan)): #64 is the max number of supported channels in fourfit
        chan_id = ch_labels[i]
        phase_residuals[ chan_id ] = ch_phase[i] #in degrees

    #now compute the phase corrections
    phase_corrections = phase_residuals.copy()
    phase_list_proxy = []
    channel_list = []
    for ch, ch_phase in list(phase_corrections.items()):
        channel_list.append( ch )
        phase_list_proxy.append( -1.0*ch_phase ) #invert to get corrections from residuals

    #invert, unwrap and remove mean phase
    #TODO FIXME -- the next 3 lines effectively do nothing, we have no need to unwrap
    mean_phase = scipy.stats.circmean( np.asarray(phase_list_proxy), high=180.0, low=-180.0) #compute circular mean phase
    #subtract off the mean and limit to [-180,180)
    phase_list_proxy = [ utility.limit_periodic_quantity_to_range( (x - mean_phase), -180.0, 180.0 ) for x in phase_list_proxy]

    #assign the corrections
    for i in list(range(0, len(phase_list_proxy))):
        phase_corrections[ channel_list[i] ] = phase_list_proxy[i]

    chan_names = ""
    phase_list_str = ""
    for elem in "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789":
        if elem in channel_list:
            chan_names += elem
            phase_list_str += str(round(phase_corrections[elem],2) ) + " "

    print("pc_phases ", chan_names, phase_list_str)
