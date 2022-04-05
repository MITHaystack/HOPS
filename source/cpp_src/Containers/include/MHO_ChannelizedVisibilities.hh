#ifndef MHO_ChannelizedVisibilities_HH__
#define MHO_ChannelizedVisibilities_HH__

/*
*File: MHO_ChannelizedVisibilities.hh
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

using ch_polprod_axis_type = MHO_Axis<std::string>;
using ch_pol_axis_type = MHO_Axis<std::string>;
using ch_channel_axis_type = MHO_Axis<int>; //channels are simply numbered
using ch_frequency_axis_type = MHO_Axis<double>;
using ch_time_axis_type = MHO_Axis<double>;

#define CH_VIS_NDIM 4
#define CH_POLPROD_AXIS 0
#define CH_CHANNEL_AXIS 1
#define CH_TIME_AXIS 2
#define CH_FREQ_AXIS 3

using ch_baseline_axis_pack = MHO_AxisPack< ch_polprod_axis_type, ch_channel_axis_type, ch_time_axis_type, ch_frequency_axis_type >;
using ch_baseline_data_type = MHO_TableContainer< visibility_type, ch_baseline_axis_pack >;
using ch_baseline_weight_type = MHO_TableContainer< weight_type, ch_baseline_axis_pack >;
using ch_baseline_sbd_type = MHO_TableContainer< visibility_type, ch_baseline_axis_pack >;


//other auxilliary data types (perhaps we should have a 'MHO_ChannelizedQuantities' wrapper header for all of this )
//this is for per-channel manual pcal
using ch_pcal_axis_pack = MHO_AxisPack< ch_polprod_axis_type, ch_channel_axis_type>;
using ch_pcal_phase_type = MHO_TableContainer< double, ch_pcal_axis_pack >;  //(here we are storing pcal as a rotation in 'degrees', should/could we use phasor instead?)




}//end of hops namespaces

#endif /* end of include guard: MHO_ChannelizedVisibilities */
