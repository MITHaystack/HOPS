#ifndef MHO_BaselineContainers_HH__
#define MHO_BaselineContainers_HH__

//this include file should not be used directly
#ifndef MHO_ContainerDefinitions_HH__
#error "Do not include MHO_BaselineContainers.hh directly; use MHO_ContainerDefinitions.hh instead."
#endif




namespace hops
{

/*!
*@file MHO_BaselineContainers.hh
*@author J. Barrett - barrettj@mit.edu
*@date Tue Apr 12 16:15:02 2022 -0400 Wed 21 Oct 2020 08:32:43 PM UTC
*@brief Definitions for single-baseline visibility data and related quantities.
* This will likely be re-worked as things progress.
*/

////////////////////////////////////////////////////////////////////////////
//the following definitions are for 'un-channelized data'
//this doesn't necessarily mean that this data can't be made chunked into channels
//and labelled as such, just that these channels are all indexed
//along the same (possibly discontinuous) frequency axis
//(not along separate channel dimension), making this data 3-dimensional
//This type of data layout is non-standard (and not used by original fourfit algo)
//Currently these data types are only used as an intermediate state during mk4 types -> hops conversion

#define UCH_VIS_NDIM 3
#define UCH_POLPROD_AXIS 0
#define UCH_TIME_AXIS 1
#define UCH_FREQ_AXIS 2

using uch_baseline_axis_pack = MHO_AxisPack< polprod_axis_type, time_axis_type, frequency_axis_type >;
using uch_visibility_type = MHO_TableContainer< visibility_element_type, uch_baseline_axis_pack >;
using uch_weight_type = MHO_TableContainer< weight_element_type, uch_baseline_axis_pack >;

////////////////////////////////////////////////////////////////////////////////
//Definitions for visibilities and related quantities (in-memory types)
//which have a separate channel axis. In this case all channels should have the
//same number of spectral points, or be padded out to the same size

#define VIS_NDIM 4
#define POLPROD_AXIS 0
#define CHANNEL_AXIS 1
#define TIME_AXIS 2
#define FREQ_AXIS 3

using baseline_axis_pack = MHO_AxisPack< polprod_axis_type, channel_axis_type, time_axis_type, frequency_axis_type >;
using mbd_dr_axis_pack = MHO_AxisPack< time_axis_type, delay_rate_axis_type >;

using visibility_type = MHO_TableContainer< visibility_element_type, baseline_axis_pack >;
using weight_type = MHO_TableContainer< weight_element_type, baseline_axis_pack >;
using sbd_type = visibility_type;
using sbd_dr_type = visibility_type;

using mbd_dr_type = MHO_TableContainer< visibility_element_type, mbd_dr_axis_pack >;
using mbd_dr_amp_type = MHO_TableContainer< weight_element_type, mbd_dr_axis_pack >;

//(here we are storing manual pcal as a rotation in 'degrees', should/could we use phasor instead?)
using pcal_axis_pack = MHO_AxisPack< pol_axis_type, channel_axis_type>;
using pcal_phase_type = MHO_TableContainer< double, pcal_axis_pack >;

////////////////////////////////////////////////////////////////////////////////
//Definitions for storage (on-disk) types for visibilities and related quantities to cut down on disk usage
using visibility_store_type = MHO_TableContainer< visibility_element_store_type, baseline_axis_pack >;
using weight_store_type = MHO_TableContainer< weight_element_store_type, baseline_axis_pack >;

using uch_visibility_store_type = MHO_TableContainer< visibility_element_store_type, uch_baseline_axis_pack >;
using uch_weight_store_type = MHO_TableContainer< weight_element_store_type, uch_baseline_axis_pack >;

















}//end of hops namespaces

#endif /*! end of include guard: MHO_BaselineContainers */
