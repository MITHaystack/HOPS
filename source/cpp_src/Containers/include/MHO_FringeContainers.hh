#ifndef MHO_FringeContainers_HH__
#define MHO_FringeContainers_HH__

//this include file should not be used directly
#ifndef MHO_ContainerDefinitions_HH__
#error "Do not include MHO_StationContainers.hh directly; use MHO_ContainerDefinitions.hh instead."
#endif

namespace hops
{

//quick and dirty, TODO formalize this

using xpower_type = MHO_TableContainer< std::complex<double>, MHO_AxisPack< MHO_Axis<double> > >;
using phasor_type = MHO_TableContainer< std::complex<double>, MHO_AxisPack< MHO_Axis<double>, MHO_Axis<double> > >;
using xpower_amp_type = MHO_TableContainer< double, MHO_AxisPack< MHO_Axis<double> > >;


}//end of hops namespaces

#endif /* end of include guard: MHO_StationCoordinates */
