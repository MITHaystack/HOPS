import pyMHO_Containers

def test_inter(cstore_interface_obj, param_interface_obj):
    n_obj =  cstore_interface_obj.GetNObjects();
    print("n objects present = ", n_obj)
    uuid_string = param_interface_obj.Get("my_uuid");
    print("expecting an object with uuid: ", uuid_string)
    print("object with that uuid is present? ", cstore_interface_obj.IsObjectPresent(uuid_string) )

    visib_obj = cstore_interface_obj.GetVisibilityObject(uuid_string);
    arr = visib_obj.GetNumpyArray() #this is already a numpy array
    print("vis array shape = ", arr.shape)
    print("vis array strides = ",arr.strides)

    arr[0,0,0,0] =  3.+10.j
    arr[0,0,1,1] =  5.+3.j
