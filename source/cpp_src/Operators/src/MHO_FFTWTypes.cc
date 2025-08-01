#include "MHO_FFTWTypes.hh"

namespace hops
{

MHO_FFTWTypeInfo::MHO_FFTWTypeInfo(){};
MHO_FFTWTypeInfo::~MHO_FFTWTypeInfo(){};

const decltype(&fftwf_execute) MHO_FFTWTypes< float >::execute_func = fftwf_execute;
const decltype(&fftwf_execute_dft) MHO_FFTWTypes< float >::execute_dft_func = fftwf_execute_dft;
const decltype(&fftwf_alignment_of) MHO_FFTWTypes< float >::alignment_of_func = fftwf_alignment_of;
const decltype(&fftwf_destroy_plan) MHO_FFTWTypes< float >::destroy_plan_func = fftwf_destroy_plan;
const decltype(&fftwf_alloc_complex) MHO_FFTWTypes< float >::alloc_func = fftwf_alloc_complex;
const decltype(&fftwf_free) MHO_FFTWTypes< float >::free_func = fftwf_free;
const decltype(&fftwf_plan_guru_dft) MHO_FFTWTypes< float >::plan_guru_func = fftwf_plan_guru_dft;

const decltype(&fftw_execute) MHO_FFTWTypes< double >::execute_func = fftw_execute;
const decltype(&fftw_execute_dft) MHO_FFTWTypes< double >::execute_dft_func = fftw_execute_dft;
const decltype(&fftw_alignment_of) MHO_FFTWTypes< double >::alignment_of_func = fftw_alignment_of;
const decltype(&fftw_destroy_plan) MHO_FFTWTypes< double >::destroy_plan_func = fftw_destroy_plan;
const decltype(&fftw_alloc_complex) MHO_FFTWTypes< double >::alloc_func = fftw_alloc_complex;
const decltype(&fftw_free) MHO_FFTWTypes< double >::free_func = fftw_free;
const decltype(&fftw_plan_guru_dft) MHO_FFTWTypes< double >::plan_guru_func = fftw_plan_guru_dft;

const decltype(&fftwl_execute) MHO_FFTWTypes< long double >::execute_func = fftwl_execute;
const decltype(&fftwl_execute_dft) MHO_FFTWTypes< long double >::execute_dft_func = fftwl_execute_dft;
const decltype(&fftwl_alignment_of) MHO_FFTWTypes< long double >::alignment_of_func = fftwl_alignment_of;
const decltype(&fftwl_destroy_plan) MHO_FFTWTypes< long double >::destroy_plan_func = fftwl_destroy_plan;
const decltype(&fftwl_alloc_complex) MHO_FFTWTypes< long double >::alloc_func = fftwl_alloc_complex;
const decltype(&fftwl_free) MHO_FFTWTypes< long double >::free_func = fftwl_free;
const decltype(&fftwl_plan_guru_dft) MHO_FFTWTypes< long double >::plan_guru_func = fftwl_plan_guru_dft;

} // namespace hops
