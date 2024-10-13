#ifndef MHO_StationContainers_HH__
#define MHO_StationContainers_HH__

//this include file should not be used directly
#ifndef MHO_ContainerDefinitions_HH__
    #error "Do not include MHO_StationContainers.hh directly; use MHO_ContainerDefinitions.hh instead."
#endif

namespace hops
{

#define STATION_NDIM 4
#define COORD_AXIS 0
#define INTERVAL_AXIS 1
#define COEFF_AXIS 2

#define NCOORD 7 //delay, az, el, par-angle, u, v, w
#define NCOEFF 6 //hard coded in mk4 library, but can be flexible

#define MTPCAL_NDIM 3
#define MTPCAL_POL_AXIS 0
#define MTPCAL_TIME_AXIS 1
#define MTPCAL_FREQ_AXIS 2

using station_coord_axis_pack = MHO_AxisPack< coord_axis_type, time_axis_type, coeff_axis_type >;
using station_coord_type = MHO_TableContainer< spline_coeff_type, station_coord_axis_pack >;

//multi-tone pcal data
using multitone_pcal_axis_type = MHO_AxisPack< pol_axis_type, time_axis_type, frequency_axis_type >;
using multitone_pcal_type = MHO_TableContainer< pcal_phasor_type, multitone_pcal_axis_type >;

// //reduced multi-tone pcal data (simple piecewise linear spline (phase + delay) for each channel/AP
// using pcal_phase_delay_axis_type = MHO_AxisPack< pol_axis_type, channel_axis_type, time_axis_type, coeff_axis_type>;
// using pcal_phase_delay_type = MHO_TableContainer< pcal_phdly_type, pcal_phase_delay_axis_type>;

//manual (per-channel) pcal (phase) offsets
using manual_pcal_axis_type = MHO_AxisPack< pol_axis_type, channel_axis_type >;
using manual_pcal_type = MHO_TableContainer< manual_pcal_element_type, manual_pcal_axis_type >;

//manual (per-channel) pcal (delay) offsets
using manual_pcal_delay_type = MHO_TableContainer< manual_pcal_element_type, manual_pcal_axis_type >;

} // namespace hops

#endif /*! end of include guard: MHO_StationCoordinates */
