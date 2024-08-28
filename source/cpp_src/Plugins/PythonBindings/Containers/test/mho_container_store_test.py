import pyMHO_Containers

def test_inter(cstore_interface_obj, param_interface_obj):
    n_obj =  cstore_interface_obj.get_nobjects();
    print("n objects present = ", n_obj)
    uuid_string = param_interface_obj.get_by_path("vis_uuid");
    print("expecting an object with uuid: ", uuid_string)
    print("object with that uuid is present? ", cstore_interface_obj.is_object_present(uuid_string) )

    visib_obj = cstore_interface_obj.get_object(uuid_string);
    arr = visib_obj.get_numpy_array() #this is already a numpy array
    print("vis array shape = ", arr.shape)
    print("vis array strides = ",arr.strides)

    arr[0,0,0,0] =  3.+10.j
    arr[0,0,1,1] =  5.+3.j


    wuuid_string = param_interface_obj.get_by_path("weight_uuid");
    print("expecting an object with uuid: ", wuuid_string)
    print("object with that uuid is present? ", cstore_interface_obj.is_object_present(wuuid_string) )

    weight_obj = cstore_interface_obj.get_object(wuuid_string);
    warr = weight_obj.get_numpy_array() #this is already a numpy array
    print("vis array shape = ", arr.shape)
    print("vis array strides = ",arr.strides)

    warr[0,0,0,0] = 1e6

    print("visib object = ")
    print(visib_obj)

    print("vis array = ")
    print(arr)

    ## TRIGGERS A FATAL ERRROR (as expected)
    # garbage_string = "sbli270.%$@#$9"
    # garbage_obj = cstore_interface_obj.get_object(garbage_string);
