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

    n_obj =  cstore_interface_obj.get_nobjects();
    print("n objects present = ", n_obj)
    uuid_string = param_interface_obj.get_by_path("vis_uuid");
    print("expecting an object with uuid: ", uuid_string)
    print("object with that uuid is present? ", cstore_interface_obj.is_object_present(uuid_string) )

    visib_obj = cstore_interface_obj.get_object(uuid_string)

    print("vis object class name = ", visib_obj.get_classname() )

    vis_arr = visib_obj.get_numpy_array() #this is already a numpy array
    print("vis array shape = ", vis_arr.shape)
    print("vis array strides = ", vis_arr.strides)

    print("vis rank = ", visib_obj.get_rank() )

    axis1 = visib_obj.get_axis(1);
    axis0 = visib_obj.get_axis(0);
    axis2 = visib_obj.get_axis(2);
    axis3 = visib_obj.get_axis(3);
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
    visib_obj.set_axis_label(0, 0, "RR")
    visib_obj.set_axis_label(0, 1, "LR")



def test_plot_visibilities(cstore_interface_obj, param_interface_obj):

    n_obj =  cstore_interface_obj.get_nobjects();
    print("n objects present = ", n_obj)
    vis_uuid = param_interface_obj.get_by_path("/uuid/visibilities");
    print("expecting an object with uuid: ", vis_uuid)
    print("object with that uuid is present? ", cstore_interface_obj.is_object_present(vis_uuid) )

    if not cstore_interface_obj.is_object_present(vis_uuid):
        return

    visib_obj = cstore_interface_obj.get_object(vis_uuid);

    vis_arr = visib_obj.get_numpy_array();
    axis0 = visib_obj.get_axis(0);
    axis1 = visib_obj.get_axis(1);
    axis2 = visib_obj.get_axis(2);
    axis3 = visib_obj.get_axis(3);

    #modifying all values for APs 12-15
    # vis_arr[:,:,12:15,:] = 200.0

    #lets dump the tags
    tags = visib_obj.get_metadata()
    print("tags = ", tags)

    tags["python_int_tag"] = 33
    tags["python_float_tag"] = 3.14159
    tags["python_string_tag"] = "a_new_string"
    tags["python vector tag"] = [1.0, 2.0, 3.0, 12.0]
    tags["python_bool_tag"] = True

    #now set the tag object
    visib_obj.set_metadata(tags)

    #now retrieve and dump them again
    tags = visib_obj.get_metadata()
    print("updated table metadata/tags = ", tags)

    #get the axis meta dat object
    for idx in [0,1,2,3]:
        print("axis: ", idx, " interval labels: ")
        ax_meta = visib_obj.get_axis_metadata(idx);
        print(ax_meta)
        if idx == 1:
            #channel axis, lets tweak it and whole-sale reset the meta data
            #this is super crude
            ax_meta['index_labels']['0']['bandwidth'] = 400.0
            visib_obj.set_axis_metadata(idx, ax_meta)
            ax_meta2 = visib_obj.get_axis_metadata(idx);
            print("MODIFIED: ", ax_meta2)


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




def test_plot_visibilities_bad(cstore_interface_obj, param_interface_obj):

    n_obj =  cstore_interface_obj.get_nobjects();
    print("n objects present = ", n_obj)
    vis_uuid = param_interface_obj.get_by_path("/uuid/visibilities");
    print("expecting an object with uuid: ", vis_uuid)
    print("object with that uuid is present? ", cstore_interface_obj.is_object_present(vis_uuid) )

    if not cstore_interface_obj.is_object_present(vis_uuid):
        return

    visib_obj = cstore_interface_obj.get_object(vis_uuid);

    vis_arr = visib_obj.get_numpy_array();
    axis0 = visib_obj.get_axis(0);
    axis1 = visib_obj.get_axis(1);
    axis2 = visib_obj.get_axis(2);
    axis3 = visib_obj.get_axis(3);

    #shoudl provoke an error
    vis_arr.resize( [1,2,3,4] )

    #should provoke an error
    vis_arr.reshape(3,4,5,6)

def labeling_test(cstore_interface_obj, param_interface_obj):
    print("executing python data-labeling operator")

def flagging_test(cstore_interface_obj, param_interface_obj):
    print("executing python data-flagging operator")

def calibration_test(cstore_interface_obj, param_interface_obj):
    print("executing python data-calibration operator")
