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
                    for obj_id in obj_info:
                        if obj_id["shortname"] == "vis":
                            uuid = obj_id["object_uuid"]
                    if uuid != "":
                        vis = bl_data.get_object(uuid)
                        if vis != None:
                            print("retrieved object: ", uuid, " as visibility")
                            class_name = vis.get_classname()
                            rank = vis.get_rank()
                            print("visibility object has rank: ", rank, " and is a ", class_name)
                            table_meta = vis.get_metadata()
                            print("meta data is: ", table_meta)
                            vis_arr = vis.get_numpy_array()
                            print("vis array shape = ", vis_arr.shape)
                            print("vis array strides = ", vis_arr.strides)
                            for i in range(0,rank):
                                dim = vis.get_dimension(i)
                                print("dimension: ", i, " has size: ", dim)
                                axis = vis.get_axis(i)
                                print("axis: ", i, " = ", axis)
                                axis_meta = vis.get_axis_metadata(i)
                                print("axis meta data = ", axis_meta)
