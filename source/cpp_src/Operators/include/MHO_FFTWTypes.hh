#ifndef MHO_FFTWTypes_HH__
#define MHO_FFTWTypes_HH__

/*!
*@file MHO_FFTWTypes.hh
*@class MHO_FFTWTypes
*@author J. Barrett - barrettj@mit.edu
*@date
*@brief
* template declaration of common FFTW3 types (dependent on floating precision)
* These are necessary to get around partial template specialization in the FFT x-form classes
*/

#include <fftw3.h>
#include <complex>

//avoids "no-args depending on template parameter error"
int fftwf_alignment_of(float*) __attribute__((weak));
int fftw_alignment_of(double*) __attribute__((weak));
int fftwl_alignment_of(long double*) __attribute__((weak));

namespace hops
{

template< typename XFloatType = void>
struct MHO_FFTWTypes;

template<>
struct MHO_FFTWTypes<float>
{
    using complex_type = std::complex<float>;
    using complex_type_ptr = std::complex<float>*;
    using fftw_complex_type = fftwf_complex;
    using fftw_complex_type_ptr = fftwf_complex*;
    using iodim_type = fftwf_iodim;
    using plan_type = fftwf_plan;

    static constexpr auto execute_func = fftwf_execute;
    static constexpr auto execute_dft_func = fftwf_execute_dft;
    static constexpr auto alignment_of_func = fftwf_alignment_of;
    static constexpr auto destroy_plan_func = fftwf_destroy_plan;
    static constexpr auto alloc_func = fftwf_alloc_complex;
    static constexpr auto free_func = fftwf_free;
    static constexpr auto plan_guru_func = fftwf_plan_guru_dft;
};

template<>
struct MHO_FFTWTypes<double>
{
    using complex_type = std::complex<double>;
    using complex_type_ptr = std::complex<double>*;
    using fftw_complex_type = fftw_complex;
    using fftw_complex_type_ptr = fftw_complex*;
    using iodim_type = fftw_iodim;
    using plan_type = fftw_plan;

    static constexpr auto execute_func = fftw_execute;
    static constexpr auto execute_dft_func = fftw_execute_dft;
    static constexpr auto alignment_of_func = fftw_alignment_of;
    static constexpr auto destroy_plan_func = fftw_destroy_plan;
    static constexpr auto alloc_func = fftw_alloc_complex;
    static constexpr auto free_func = fftw_free;
    static constexpr auto plan_guru_func = fftw_plan_guru_dft;
};



template<>
struct MHO_FFTWTypes<long double>
{
    using complex_type = std::complex<long double>;
    using complex_type_ptr = std::complex<long double>*;
    using fftw_complex_type = fftwl_complex;
    using fftw_complex_type_ptr = fftwl_complex*;
    using iodim_type = fftwl_iodim;
    using plan_type = fftwl_plan;

    static constexpr auto execute_func = fftwl_execute;
    static constexpr auto execute_dft_func = fftwl_execute_dft;
    static constexpr auto alignment_of_func = fftwl_alignment_of;
    static constexpr auto destroy_plan_func = fftwl_destroy_plan;
    static constexpr auto alloc_func = fftwl_alloc_complex;
    static constexpr auto free_func = fftwl_free;
    static constexpr auto plan_guru_func = fftwl_plan_guru_dft;
};

}

#endif /*! end of include guard: MHO_FFTWTypes */
