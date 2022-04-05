#ifndef MHO_VisibilityType_H__
#define MHO_VisibilityType_H__

#include <complex>

namespace hops
{

//global definition of visibility floating point type
//we use double for now...but may want to consider float (to reduce unecessary memory)
//perhaps a float format can be used for disk-storage, while a double format can be
//used for in-memory computation...should look at trade-offs with this

typedef double VFP_TYPE;
using visibility_type = std::complex<VFP_TYPE>;

typedef double WFP_TYPE;
using weight_type = WFP_TYPE;

typedef double PCAL_TYPE;
using pcal_phasor_type = std::complex<PCAL_TYPE>;

}

#endif /* end of include guard: MHO_VisibilityType_H__ */
