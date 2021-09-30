//definitions of a channelized set of single-baseline visbility data
using visibility_type = std::complex<double>;

using ch_polprod_axis_type = MHO_Axis<std::string>;
using ch_channel_axis_type = MHO_Axis<int>; //channels are simply numbered here
using ch_frequency_axis_type = MHO_Axis<double>;
using ch_time_axis_type = MHO_Axis<double>;

using ch_baseline_axis_pack = MHO_AxisPack< ch_polprod_axis_type, ch_channel_axis_type, ch_time_axis_type, ch_frequency_axis_type >;
using ch_baseline_data_type = MHO_TableContainer< visibility_type, ch_baseline_axis_pack >;
using ch_baseline_weight_type = MHO_TableContainer< weight_type, ch_baseline_axis_pack >;
using ch_baseline_sbd_type = MHO_TableContainer< visibility_type, ch_baseline_axis_pack >;
