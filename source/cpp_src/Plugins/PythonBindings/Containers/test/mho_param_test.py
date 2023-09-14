import pyMHO_Containers

def test_inter(interface_obj):
    key = "/my/test/value"
    int_key = "/a/int/value"
    float_key = "/a/float/value"
    string_key = "/a/string/value"

    another_test_value = "/another/test/value"
    bad_key = "/does/not/exist"

    print("is the value ", key, " present? ", interface_obj.IsPresent(key) )
    print("value of ", int_key, ": ", interface_obj.Get(int_key) )
    print("value of ", float_key, ": ", interface_obj.Get(float_key) )
    print("value of ", string_key, ": ", interface_obj.Get(string_key) )
    print("value of ", another_test_value, ": ", interface_obj.Get(another_test_value) )
    print("value of ", bad_key, ": ", interface_obj.Get(bad_key) )
    print("value of ", another_test_value, ": ", interface_obj.Get(another_test_value) )

    a_string = "this is a python string"
    a_int = 42
    a_float = 243.234233

    interface_obj.Set(int_key, a_int)
    interface_obj.Set(float_key, a_float)
    interface_obj.Set(string_key, a_string)

    print("value of ", int_key, ": ", interface_obj.Get(int_key) )
    print("value of ", float_key, ": ", interface_obj.Get(float_key) )
    print("value of ", string_key, ": ", interface_obj.Get(string_key) )
