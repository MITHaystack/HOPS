#ifndef MHO_BaselineContainers_HH__
#define MHO_BaselineContainers_HH__

//this include file should not be used directly
#ifndef MHO_ContainerDefinitions_HH__
#error "Do not include MHO_BaselineContainers.hh directly; use MHO_ContainerDefinitions.hh instead."
#endif


/*
*File: MHO_BaselineContainers.hh
*Author: J. Barrett
*Email: barrettj@mit.edu
*Date: Wed 21 Oct 2020 08:32:43 PM UTC
*Description: Definitions for single-baseline visibility data and related quantities.
* This will likely be re-worked as things progress.
*/

namespace hops
{

////////////////////////////////////////////////////////////////////////////
//the following definitions are for 'un-channelized data'
//this doesn't necessarily mean that data can't be made chunked into channels 
//and labelled as such using these axes, just that these channels are all indexed 
//along the same (possibly discontinuous) frequency axis 
//(not a separate channel axis), making this data 3-dimensional

#define VIS_NDIM 3
#define POLPROD_AXIS 0
#define TIME_AXIS 1
#define FREQ_AXIS 2

using baseline_axis_pack = MHO_AxisPack< polprod_axis_type, time_axis_type, frequency_axis_type >;
using visibility_type = MHO_TableContainer< visibility_element_type, baseline_axis_pack >;
using weight_type = MHO_TableContainer< weight_element_type, baseline_axis_pack >;

////////////////////////////////////////////////////////////////////////////////
//Definitions for visibilities and related quantities
//which have a separate channel axis. In this case all channels should have the 
//same number of spectral points, or be padded out to the same size

#define CH_VIS_NDIM 4
#define CH_POLPROD_AXIS 0
#define CH_CHANNEL_AXIS 1
#define CH_TIME_AXIS 2
#define CH_FREQ_AXIS 3

using ch_baseline_axis_pack = MHO_AxisPack< polprod_axis_type, channel_axis_type, time_axis_type, frequency_axis_type >;
using ch_baseline_mbd_axis_pack =  MHO_AxisPack< polprod_axis_type, time_axis_type, time_axis_type, frequency_axis_type >;

using ch_visibility_type = MHO_TableContainer< visibility_element_type, ch_baseline_axis_pack >;
using ch_weight_type = MHO_TableContainer< weight_element_type, ch_baseline_axis_pack >;
using ch_sbd_type = MHO_TableContainer< visibility_element_type, ch_baseline_axis_pack >;
using ch_mbd_type = MHO_TableContainer< visibility_element_type, ch_baseline_mbd_axis_pack >;

//(here we are storing pcal as a rotation in 'degrees', should/could we use phasor instead?)
using ch_pcal_axis_pack = MHO_AxisPack< pol_axis_type, channel_axis_type>;
using ch_pcal_phase_type = MHO_TableContainer< double, ch_pcal_axis_pack >;  

}//end of hops namespaces

#endif /* end of include guard: MHO_BaselineContainers */
