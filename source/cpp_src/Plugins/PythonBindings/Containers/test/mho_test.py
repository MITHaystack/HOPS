import pyMHO_Containers

import numpy as np
import matplotlib.pyplot as plt
from matplotlib import cm
from matplotlib.ticker import LinearLocator
from mpl_toolkits.mplot3d import Axes3D


def print_mx(mx_obj):
    arr = np.array(mx_obj,copy=False)
    print(arr.shape)
    print(arr.strides)
    print(arr)

def mod_mx(mx_obj):
    arr = np.array(mx_obj,copy=False) #[0,0] = 1000;
    arr[0,0] *= 1000;
    print(arr)

def test_inter(inter_obj):
    vis_arr = inter_obj.GetVisibilityTable().GetNumpyArray() #this is already a numpy array
    axis1 = inter_obj.GetVisibilityTable().GetCoordinateAxis(1);
    axis0 = inter_obj.GetVisibilityTable().GetCoordinateAxis(0);
    axis2 = inter_obj.GetVisibilityTable().GetCoordinateAxis(2);
    axis3 = inter_obj.GetVisibilityTable().GetCoordinateAxis(3);
    print("the visibility dimensions are: ", vis_arr.shape)
    vis_arr[0,0,0,0] = 1000
    vis_arr[0,0,1,1] = 2000
    for x in axis0:
        print(x)
    for x in axis1:
        print(x)
    for x in axis2:
        print(x)
    for x in axis3:
        print(x)


def test_plot_visibilities(inter_obj):
    vis_arr = inter_obj.GetVisibilityTable().GetNumpyArray();
    axis0 = inter_obj.GetVisibilityTable().GetCoordinateAxis(0);
    axis1 = inter_obj.GetVisibilityTable().GetCoordinateAxis(1);
    axis2 = inter_obj.GetVisibilityTable().GetCoordinateAxis(2);
    axis3 = inter_obj.GetVisibilityTable().GetCoordinateAxis(3);

    pp = 0;
    ch = 0;

    print("plotting the visibilities of: pp = ", axis0[pp], " chan = ", axis1[ch]);
    fig, ax = plt.subplots(subplot_kw={"projection": "3d"})

    time, freq = np.meshgrid(axis3, axis2)
    vis = np.array(vis_arr[pp,ch,:,:])
    print("time shape = ", time.shape)
    print("freq shape = ", freq.shape)
    print("vis_00 shape = ", vis.shape)
    vis_mag = np.abs(vis)

    # Plot the surface.
    surf = ax.plot_surface(time, freq, vis_mag, cmap=cm.coolwarm, linewidth=0, antialiased=False)

    # Add a color bar which maps values to colors.
    fig.colorbar(surf, shrink=0.5, aspect=5)
    plt.show()
