#ifndef MHO_VisibilityType_H__
#define MHO_VisibilityType_H__

#include <complex>

namespace hops
{

//global definition of visibility floating point type
//we use double for now...but may want to consider float (to reduce unecessary memory)
//at the moment the main issue for switching to float is that native type of the fftw library
//uses a complex double, so we'd need to do a coversion/copy on transform.
//This isn't really a problem, just extra work which can wait for now.
typedef double VFP_TYPE;
using visibility_type = std::complex<VFP_TYPE>;

typedef double WFP_TYPE;
using weight_type = WFP_TYPE;

}

#endif /* end of include guard: MHO_VisibilityType_H__ */
