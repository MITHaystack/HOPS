#ifndef MHOVisibilities_HH__
#define MHOVisibilities_HH__

/*
*File: MHOVisibilities.hh
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: Wed 21 Oct 2020 08:32:43 PM UTC
*Description: Place-holder definitions for single-baseline visibility data-container
* This will likely be re-worked as things progress.
*/

#include <string>
#include <complex>

#include "MHOAxis.hh"
#include "MHOAxisPack.hh"
#include "MHOTableContainer.hh"

namespace hops
{

using visibility_type = std::complex<double>;

using polprod_axis_type = MHOAxis<std::string>;
using frequency_axis_type = MHOAxis<double>;
using time_axis_type = MHOAxis<double>;

#define VIS_NDIM 3
#define POLPROD_AXIS 0
#define TIME_AXIS 1
#define FREQ_AXIS 2

using baseline_axis_pack = MHOAxisPack< polprod_axis_type, time_axis_type, frequency_axis_type >;
using baseline_data_type = MHOTableContainer< visibility_type, baseline_axis_pack >;


}//end of hops namespaces

#endif /* end of include guard: MHOVisibilities */
