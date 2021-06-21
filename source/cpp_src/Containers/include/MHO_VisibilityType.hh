#ifndef MHO_VisibilityType_H__
#define MHO_VisibilityType_H__

#include <complex>

namespace hops
{

//global definition of visibility floating point type
//we use double for now...but may want to consider float (to reduce unecessary memory)
//at the moment the main issue for switching to float is that native type of the fftw library
//uses a complex double, so we'd need to do a coversion/copy on transform
typedef double VFP_TYPE;
using visibility_type = std::complex<VFP_TYPE>;

}

#endif /* end of include guard: MHO_VisibilityType_H__ */
