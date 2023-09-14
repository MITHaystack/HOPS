import pyMHO_Containers

def test_inter(interface_obj):
    key = "/my/test/value"
    int_key = "/a/int/value"
    float_key = "/a/float/value"
    string_key = "/a/string/value"

    another_test_value = "/another/test/value"
    bad_key = "/does/not/exist"

    print("is the value ", key, " present? ", interface_obj.IsPresent(key) )
    print("value of ", int_key, ": ", interface_obj.GetAsInt(int_key) )
    print("value of ", float_key, ": ", interface_obj.GetAsFloat(float_key) )
    print("value of ", string_key, ": ", interface_obj.GetAsString(string_key) )
    print("value of ", another_test_value, ": ", interface_obj.Get(another_test_value) )
    print("value of ", bad_key, ": ", interface_obj.Get(bad_key) )
