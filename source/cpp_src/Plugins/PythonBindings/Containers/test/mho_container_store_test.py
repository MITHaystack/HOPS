import pyMHO_Containers

def test_inter(cstore_interface_obj, param_interface_obj):
    n_obj =  cstore_interface_obj.GetNObjects();
    print("n objects present = ", n_obj)
    uuid_string = param_interface_obj.Get("my_uuid");
    print("expecting an object with uuid: ", uuid_string)
    print("object with that uuid is present? ", cstore_interface_obj.IsObjectPresent(uuid_string) )
    #std::cout<<"object uuid = "<<uuid_string<<std::endl;
