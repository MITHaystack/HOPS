#ifndef MHO_StationCoordinates_HH__
#define MHO_StationCoordinates_HH__

//this include file should not be used directly
#ifndef MHO_ContainerDefinitions_HH__
#error "Do not include MHO_StationContainers.hh directly; use MHO_ContainerDefinitions.hh instead."
#endif

namespace hops
{

#define STATION_NDIM 4
#define COORD_AXIS 0
#define CHAN_AXIS 1
#define INTERVAL_AXIS 2
#define COEFF_AXIS 3

//these are hard-coded in the mk4 library
#define NCOORD 8
#define NCOEFF 6

using station_coord_axis_pack = MHO_AxisPack< coord_axis_type, channel_id_axis_type, interval_axis_type, coeff_axis_type>;
using station_coord_type = MHO_TableContainer< spline_coeff_type, station_coord_axis_pack >;

using station_coord_axis_pack2 = MHO_AxisPack< coord_axis_type, interval_axis_type, coeff_axis_type>;
using station_coord_type2 = MHO_TableContainer< spline_coeff_type, station_coord_axis_pack2 >;

//multi-tone pcal data
using multitone_pcal_axis_type = MHO_AxisPack< pol_axis_type, time_axis_type, frequency_axis_type >;
using multitone_pcal_type = MHO_TableContainer< pcal_phasor_type, multitone_pcal_axis_type >;

}//end of hops namespaces

#endif /* end of include guard: MHO_StationCoordinates */
