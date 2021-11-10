"""
Demonstration of 'picking', or interacting with datapoints with a mouse click
"""

import numpy as np
import matplotlib as mpl
mpl.rcParams['toolbar'] = 'None'

import matplotlib.pyplot as plt
from read_alist import *

# for now, hard code the location of the alist file and the figures we want to call
# from datapoints in the alist file
fname, amp, t = get_amp_time('3765/3765.alist.check', baseline='KZ')

fig, ax = plt.subplots()
ax.set_title('AEDIT Demo plot - Expt 3765, Freq B')

# the 'picker' property of an artist (like a line) is None by default
# if it is nonzero, the artist will fire a pick event if the mouse event is over the artist
# if it is a function, that function must determine whether the artist was hit by a mouse event
# NB: the 'pickradius' property is buggy; if picker is a float, pickradius is set to that value
# since keyword arguments are assigned in reverse order (!) it makes no sense to set picker and pickradius separately
line, = ax.plot(t, amp, 'o', markerfacecolor='none', markersize=5, picker=5)

ax.set_ylabel('amplitude (e-4)')
ax.set_xlabel('Time (UT on day 121-105')


# This is the function that is called when a pick_event happens (see the mpl_connect line below)
# The argument is a matplotlib.backend_bases.PickEvent class, which has two attributes:
# the mouse event that generated the pick (position, button pressed, etc), and the artist that
# generated the pick event
def pick_action(event):

    # check that we picked an element from the data we plotted
    if event.artist != line:
        return

    # get the index of the datapoint that was picked in the artist (might be more than one if points overlap)
    n = len(event.ind)
    if not n:
        return

    # generate a new figure, with enough subplots to accomodate the number of datapoints that were picked
    fig, axs = plt.subplots(n, squeeze=False)

    # associate each datapoint with a panel in the new figure
    for dataind, ax in zip(event.ind, axs.flat):

        # read in the kludged fourfit plot and plot it
        image = plt.imread('3765/'+fname[dataind]+'.jpg')        
        ax.imshow(image)
        ax.axis('off')
        ax.margins(0,0)
        
    #fig.tight_layout()    
    #fig.subplots_adjust(left=0, bottom=0, right=1, top=1, wspace=None, hspace=None)

    # show the new figure
    fig.show()
    return True

# this connects our callback function to the event manager
fig.canvas.mpl_connect('pick_event', pick_action)
plt.show()
