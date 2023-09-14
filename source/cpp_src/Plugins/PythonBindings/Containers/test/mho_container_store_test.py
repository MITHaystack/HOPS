import pyMHO_Containers

def test_inter(cstore_interface_obj, param_interface_obj):
    n_obj =  cstore_interface_obj.GetNObjects();
    print("n objects present = ", n_obj)
    uuid_string = param_interface_obj.Get("vis_uuid");
    print("expecting an object with uuid: ", uuid_string)
    print("object with that uuid is present? ", cstore_interface_obj.IsObjectPresent(uuid_string) )

    visib_obj = cstore_interface_obj.GetVisibilityObject(uuid_string);
    arr = visib_obj.GetNumpyArray() #this is already a numpy array
    print("vis array shape = ", arr.shape)
    print("vis array strides = ",arr.strides)

    arr[0,0,0,0] =  3.+10.j
    arr[0,0,1,1] =  5.+3.j


    wuuid_string = param_interface_obj.Get("weight_uuid");
    print("expecting an object with uuid: ", wuuid_string)
    print("object with that uuid is present? ", cstore_interface_obj.IsObjectPresent(wuuid_string) )

    weight_obj = cstore_interface_obj.GetWeightObject(wuuid_string);
    warr = weight_obj.GetNumpyArray() #this is already a numpy array
    print("vis array shape = ", arr.shape)
    print("vis array strides = ",arr.strides)

    warr[0,0,0,0] = 1e6

    #TRIGGERS A FATAL ERRROR (as expected)
    # garbage_string = "sbli270.%$@#$9"
    # garbage_obj = cstore_interface_obj.GetVisibilityObject(garbage_string);
