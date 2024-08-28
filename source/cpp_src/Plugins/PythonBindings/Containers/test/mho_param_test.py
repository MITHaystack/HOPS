import pyMHO_Containers

def test_inter(interface_obj):
    key = "/my/test/value"
    int_key = "/a/int/value"
    float_key = "/a/float/value"
    string_key = "/a/string/value"

    another_test_value = "/another/test/value"
    bad_key = "/does/not/exist"

    print("is the value ", key, " present? ", interface_obj.is_present(key) )
    print("value of ", int_key, ": ", interface_obj.get_by_path(int_key) )
    print("value of ", float_key, ": ", interface_obj.get_by_path(float_key) )
    print("value of ", string_key, ": ", interface_obj.get_by_path(string_key) )
    print("value of ", another_test_value, ": ", interface_obj.get_by_path(another_test_value) )
    print("value of ", bad_key, ": ", interface_obj.get_by_path(bad_key) )
    print("value of ", another_test_value, ": ", interface_obj.get_by_path(another_test_value) )

    a_string = "this is a python string"
    a_int = 42
    a_float = 243.234233

    interface_obj.set_by_path(int_key, a_int)
    interface_obj.set_by_path(float_key, a_float)
    interface_obj.set_by_path(string_key, a_string)

    print("value of ", int_key, ": ", interface_obj.get_by_path(int_key) )
    print("value of ", float_key, ": ", interface_obj.get_by_path(float_key) )
    print("value of ", string_key, ": ", interface_obj.get_by_path(string_key) )

    my_list = [0.1, 43, 85, 0.6, 1.434]

    interface_obj.set_by_path("a_list", my_list);

    myNoneType = None
    interface_obj.set_by_path("a_none_type", myNoneType);

    #lets dump the entire dictionary
    contents = interface_obj.get_contents();
    print("full contents = ", contents);
