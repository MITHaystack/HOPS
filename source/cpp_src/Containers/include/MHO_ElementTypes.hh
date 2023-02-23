#ifndef MHO_ElementTypes_H__
#define MHO_ElementTypes_H__

#include <complex>

//this include file should not be used directly
#ifndef MHO_ContainerDefinitions_HH__
#error "Do not include MHO_ElementTypes.hh directly; use MHO_ContainerDefinitions.hh instead."
#endif

namespace hops
{

//global definition of floating point types for use in data containers//////
//we primarily use double for now...but may want to consider float 
//(to reduce unecessary memory usage) or perhaps consider a separate float format
//that can be used for disk-storage, while the double format is used 
//for in-memory computation...we should look at trade-offs with this

typedef double VFP_TYPE;
typedef double WFP_TYPE;
typedef double PCFP_TYPE;
typedef double SPLINE_TYPE;
typedef char FLAG_TYPE;

using visibility_element_type = std::complex<VFP_TYPE>;
using weight_element_type = WFP_TYPE;
using pcal_phasor_type = std::complex<PCFP_TYPE>;
using manual_pcal_element_type = PCFP_TYPE;
using spline_coeff_type = SPLINE_TYPE;

//definitions of the type used in the flag table, we use char, as for the
//the most part we only boolean-style flagging (good/bad)
using flag_element_type = FLAG_TYPE;

//specific definitions for storage types (e.g. on-disk)
//since the double based-types take up twice as much space
typedef float VFP_STORE_TYPE;
typedef float WFP_STORE_TYPE;

using visibility_element_store_type = std::complex<VFP_STORE_TYPE>;
using weight_element_store_type = WFP_STORE_TYPE;



}//end namespace

#endif /* end of include guard: MHO_ElementTypes_H__ */
