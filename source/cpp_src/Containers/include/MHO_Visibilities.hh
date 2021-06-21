#ifndef MHO_Visibilities_HH__
#define MHO_Visibilities_HH__

/*
*File: MHO_Visibilities.hh
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: Wed 21 Oct 2020 08:32:43 PM UTC
*Description: Place-holder definitions for single-baseline visibility data-container
* This will likely be re-worked as things progress.
*/

#include <string>

#include "MHO_Axis.hh"
#include "MHO_AxisPack.hh"
#include "MHO_TableContainer.hh"
#include "MHO_VisibilityType.hh"

namespace hops
{

using polprod_axis_type = MHO_Axis<std::string>;
using frequency_axis_type = MHO_Axis<double>;
using time_axis_type = MHO_Axis<double>;

#define VIS_NDIM 3
#define POLPROD_AXIS 0
#define TIME_AXIS 1
#define FREQ_AXIS 2

using baseline_axis_pack = MHO_AxisPack< polprod_axis_type, time_axis_type, frequency_axis_type >;
using baseline_data_type = MHO_TableContainer< visibility_type, baseline_axis_pack >;


}//end of hops namespaces

#endif /* end of include guard: MHO_Visibilities */
