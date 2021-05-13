#ifndef MHO_StationCoordinates_HH__
#define MHO_StationCoordinates_HH__

/*
*File: MHO_StationCoordinates.hh
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: Wed 21 Oct 2020 08:32:43 PM UTC
*Description: Place-holder definitions for station 'coords' - mostly a priori spline coefficients
* This will likely be re-worked as things progress, and probably re-named
*/

#include <string>
#include <complex>

#include "MHO_Axis.hh"
#include "MHO_AxisPack.hh"
#include "MHO_TableContainer.hh"

namespace hops
{

using spline_coeff_type = double;

using coord_axis_type = MHO_Axis<std::string>; //coordinate name (delay, phase, parallatic_angle, az, el, u, v, w)
using channel_axis_type = MHO_Axis<std::string>; //channel id
using interval_axis_type = MHO_Axis<int>; //time interval index (could be double??)
using coeff_axis_type = MHO_Axis<int>; //coefficient index 0,1,2...6(max)

#define STATION_NDIM 4
#define COORD_AXIS 0
#define CHAN_AXIS 1
#define INTERVAL_AXIS 2
#define COEFF_AXIS 3

//these are hard-coded in the mk4 library
#define NCOORD 8
#define NCOEFF 6

using station_coord_axis_pack = MHO_AxisPack< coord_axis_type, channel_axis_type, interval_axis_type, coeff_axis_type>;
using station_coord_data_type = MHO_TableContainer< spline_coeff_type, station_coord_axis_pack >;


}//end of hops namespaces

#endif /* end of include guard: MHO_StationCoordinates */
