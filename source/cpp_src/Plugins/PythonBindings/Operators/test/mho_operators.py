import pyMHO_Containers

def TestThreshold(input_table):
    vis_arr = input_table.GetNumpyArray();
    nd = vis_arr.ndim
    threshold = 11000
    if nd == 4:
        vis_shape = vis_arr.shape
        #select polprod 0, channel 0 and zero out all visibilities
        #with a magnitude greater than the threshold
        pp = 0
        ch = 0
        for n in range(0, vis_shape[2]):
            for m in range(0, vis_shape[3]):
                if abs( vis_arr[pp,ch,n,m] ) > threshold:
                    vis_arr[pp,ch,n,m] = 0.0
    print("made it here")
