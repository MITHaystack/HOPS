//definitions of a non-channelized set of single-baseline visbility data
using visibility_type = std::complex<double>;

using polprod_axis_type = MHO_Axis<std::string>;
using frequency_axis_type = MHO_Axis<double>;
using time_axis_type = MHO_Axis<double>;

using baseline_axis_pack = MHO_AxisPack< polprod_axis_type, time_axis_type, frequency_axis_type >;
using baseline_data_type = MHO_TableContainer< visibility_type, baseline_axis_pack >;

