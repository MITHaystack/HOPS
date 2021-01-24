#ifndef MHOChannelizedVisibilities_HH__
#define MHOChannelizedVisibilities_HH__

/*
*File: MHOChannelizedVisibilities.hh
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

using ch_visibility_type = std::complex<double>;

using ch_polprod_axis_type = MHOAxis<std::string>;
using ch_channel_axis_type = MHOAxis<int>; //channels are simply numbered
using ch_frequency_axis_type = MHOAxis<double>;
using ch_time_axis_type = MHOAxis<double>;

#define CH_VIS_NDIM 4
#define CH_POLPROD_AXIS 0
#define CH_CHANNEL_AXIS 1
#define CH_TIME_AXIS 2
#define CH_FREQ_AXIS 3

using ch_baseline_axis_pack = MHOAxisPack< ch_polprod_axis_type, ch_channel_axis_type, ch_time_axis_type, ch_frequency_axis_type >;
using ch_baseline_data_type = MHOTableContainer< ch_visibility_type, ch_baseline_axis_pack >;


}//end of hops namespaces

#endif /* end of include guard: MHOChannelizedVisibilities */
