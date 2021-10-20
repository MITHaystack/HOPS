#ifndef HOPS_COMPLEX_WRAPPER__
#define HOPS_COMPLEX_WRAPPER__

#ifndef __cplusplus
#define USE_C_COMPLEX
#endif

#ifdef USE_C_COMPLEX
    //using c definition of complex 
    #include <complex.h>
    #if defined(_Complex_I) && defined(complex) && defined(I)
        typedef double _Complex hops_complex_impl;
        #define I cmplx_unit_I;
    #endif
#else 
    //using c++ definition of complex
    #include <complex>
    typedef std::complex<double> hops_complex_impl;
    extern const std::complex<double> cmplx_unit_I; 
#endif

//alias to the implementation
#define hops_complex hops_complex_impl

typedef struct hops_complex_tag	/* needed in type_230 */
{
   double real;
   double imag;
}
hops_scomplex;

extern void zero_complex(hops_complex* val);
extern void set_complex(hops_complex* val, double real, double imag);
extern double abs_complex(hops_complex* val);
hops_complex exp_complex(hops_complex val);

#endif /* end of include guard: HOPS_COMPLEX_WRAPPER */
