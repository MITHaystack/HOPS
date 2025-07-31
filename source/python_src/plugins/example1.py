import pyMHO_Containers

def dummy(fringe_data_interface):
    param_interface_obj = fringe_data_interface.get_parameter_store()

    ref_station_id = param_interface_obj.get_by_path("/ref_station/mk4id")
    rem_station_id = param_interface_obj.get_by_path("/rem_station/mk4id")

    print("reference station", ref_station_id, ", remote station: ", rem_station_id)
