import numpy as np
import matplotlib as mpl
#mpl.rcParams['toolbar'] = 'None'

import matplotlib.pyplot as plt



def plot_alist_data(fignum, x_data, y_data, x_label, y_label):


    fig, ax = plt.subplots()
    ax.set_title('AEDIT Demo plot')

    line, = ax.plot(x_data, y_data, 'o', markerfacecolor='none', markersize=5, picker=5)
    
    ax.set_xlabel(x_label)
    ax.set_ylabel(y_label)

    fig.canvas.mpl_connect('pick_event', pick_action)
    
    return fig



def pick_action(event):

    n = len(event.ind)
    if not n:
        return

    fig, axs = plt.subplots(n, squeeze=False)

    print(event.ind)
    
    for dataind, ax in zip(event.ind, axs.flat):

        # read in the kludged fourfit plot and plot it
        image = plt.imread('KZ_3C279_LR.jpg')        
        ax.imshow(image)
        ax.axis('off')
        ax.margins(0,0)
        
    fig.show()
    return True



"""
def key_action(event):
    global curr_pos

    if event.key == "right":
        curr_pos = curr_pos + 1
    elif event.key == "left":
        curr_pos = curr_pos - 1
    else:
        return
    curr_pos = curr_pos % len(plots)

    ax.cla()
    ax.plot(plots[curr_pos][0], plots[curr_pos][1])
    fig.canvas.draw()
"""
    
