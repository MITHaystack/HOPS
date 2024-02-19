import pyMHO_Containers

def test_standalone_file_interface(dirname, baseline):
    # dirname = "./test_data/vt9105/1111/105-1800/"
    # baseline = "GE"
    scan_store = pyMHO_Containers.MHO_PyScanStoreInterface()
    scan_store.set_directory(dirname)
    ok = scan_store.initialize()

    if ok:
        bl_list = scan_store.get_baseline_list()
        if baseline in bl_list:
            ret_val = scan_store.load_baseline(baseline)
            if ret_val == 0:
                if scan_store.is_baseline_loaded(baseline):
                    bl_data = scan_store.get_baseline_data(baseline)
                    n_obj = bl_data.get_nobjects()
                    obj_info = bl_data.get_object_id_list()
                    uuid = ""
                    for oid in obj_info:
                        if oid["shortname"] == "vis":
                            uuid = oid["object_uuid"]
                    if uuid != "":
                        vis = bl_data.get_object(uuid)
                        if vis != None:
                            print("retrieved object: ", uuid, " as visibility")
                            #TODO FIXME -- test all of these functions!
# get_axis()          get_classname()     get_metadata()      get_rank()          set_axis_metadata()
# get_axis_metadata() get_dimension()     get_numpy_array()   set_axis_label()    set_metadata()
