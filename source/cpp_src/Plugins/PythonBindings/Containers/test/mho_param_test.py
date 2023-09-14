import pyMHO_Containers

def test_inter(interface_obj):
    key = "/my/test/value"
    print("is the value ", key, " present? ", interface_obj.IsPresent(key) )
