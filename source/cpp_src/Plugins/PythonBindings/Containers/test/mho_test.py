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

def test_inter(cstore_interface_obj, param_interface_obj):

    n_obj =  cstore_interface_obj.GetNObjects();
    print("n objects present = ", n_obj)
    uuid_string = param_interface_obj.Get("vis_uuid");
    print("expecting an object with uuid: ", uuid_string)
    print("object with that uuid is present? ", cstore_interface_obj.IsObjectPresent(uuid_string) )

    visib_obj = cstore_interface_obj.GetVisibilityObject(uuid_string)

    print("vis object class name = ", visib_obj.GetClassName() )

    vis_arr = visib_obj.GetNumpyArray() #this is already a numpy array
    print("vis array shape = ", vis_arr.shape)
    print("vis array strides = ", vis_arr.strides)

    print("vis rank = ", visib_obj.GetRank() )

    axis1 = visib_obj.GetCoordinateAxis(1);
    axis0 = visib_obj.GetCoordinateAxis(0);
    axis2 = visib_obj.GetCoordinateAxis(2);
    axis3 = visib_obj.GetCoordinateAxis(3);
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

    print("now we are going to modify the labels of the first axis.")
    print("we have to do this one label at a time via the visib_obj with the current interface")

    #set the polarization labels to some nonsense
    visib_obj.SetCoordinateLabel(0, 0, "RR")
    visib_obj.SetCoordinateLabel(0, 1, "LR")



def test_plot_visibilities(cstore_interface_obj, param_interface_obj):

    n_obj =  cstore_interface_obj.GetNObjects();
    print("n objects present = ", n_obj)
    vis_uuid = param_interface_obj.Get("/uuid/visibilities");
    print("expecting an object with uuid: ", vis_uuid)
    print("object with that uuid is present? ", cstore_interface_obj.IsObjectPresent(vis_uuid) )

    if not cstore_interface_obj.IsObjectPresent(vis_uuid):
        return

    visib_obj = cstore_interface_obj.GetVisibilityObject(vis_uuid);

    vis_arr = visib_obj.GetNumpyArray();
    axis0 = visib_obj.GetCoordinateAxis(0);
    axis1 = visib_obj.GetCoordinateAxis(1);
    axis2 = visib_obj.GetCoordinateAxis(2);
    axis3 = visib_obj.GetCoordinateAxis(3);

    #modifying all values for APs 12-15
    # vis_arr[:,:,12:15,:] = 200.0

    #lets dump the tags 
    tags = visib_obj.GetTags()
    print("tags = ", tags)


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
