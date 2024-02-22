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



def test_noema(cstore_interface_obj, param_interface_obj):

    print("HELLO FROM PYTHON")

    #grab the UUID of the visibility object
    vis_uuid = param_interface_obj.get_by_path("/uuid/visibilities");
    visib_obj = cstore_interface_obj.get_object(vis_uuid);

    if visib_obj is None:
        return

    vis_arr = visib_obj.get_numpy_array();
    rank = visib_obj.get_rank()
    axis1 = visib_obj.get_axis(1); #get the channel axis
    axis3 = visib_obj.get_axis(3); #get the spectral point axis (sub-channel)
    chan_meta_data = visib_obj.get_axis_metadata(1) #channel axis meta data object
    channel_info = chan_meta_data["index_labels"] #channel bin label dict

    #hacky dictionary of phase jumps for NOEMA L-pol channels
    #first number is intra-channel frequency, second is phasor correction
    #roughly estimated from fringe plots
    jumps = {
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

    nspectral = visib_obj.get_dimension(3) # number of spectral points in channel
    for idx in range(0,len(axis1)):
        idx_key = str(idx) #meta data keys are strings (e.g. "1", "2") not integers
        fourfit_chan_label = channel_info[idx_key]["channel_label"] #fourfit label
        if fourfit_chan_label in jumps:
            jump_freq = jumps[fourfit_chan_label][0]
            jump_phasor = jumps[fourfit_chan_label][1]
            #now apply the phase shift at/after the jump freq
            for sp in range(0,nspectral):
                freq = axis3[sp]
                if freq > jump_freq:
                    vis_arr[0,idx,:,sp] *= jump_phasor
