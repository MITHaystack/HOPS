#ifndef MHO_FFTWTypes_HH__
#define MHO_FFTWTypes_HH__

#include <complex>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

#include <fftw3.h>

//specifying the weak attribute avoids 
//"no-args depending on template parameter error"
//note: these functions are only available for FFTW version > 3.3.4
int fftwf_alignment_of(float*) __attribute__((weak));
int fftw_alignment_of(double*) __attribute__((weak));
int fftwl_alignment_of(long double*) __attribute__((weak));

namespace hops
{

/*!
 *@file MHO_FFTWTypes.hh
 *@class MHO_FFTWTypes
 *@author J. Barrett - barrettj@mit.edu
 *@date Tue Jun 22 14:53:20 2021 -0400
 *@brief
 * template declaration of common FFTW3 types (dependent on floating precision)
 * These are necessary to get around partial template specialization in the FFT x-form classes
 */

template< typename XFloatType = void > struct MHO_FFTWTypes;

template<> struct MHO_FFTWTypes< float >
{
        using complex_type = std::complex< float >;
        using complex_type_ptr = std::complex< float >*;
        using fftw_complex_type = fftwf_complex;
        using fftw_complex_type_ptr = fftwf_complex*;
        using iodim_type = fftwf_iodim;
        using plan_type = fftwf_plan;

        static const decltype(&fftwf_execute) execute_func;
        static const decltype(&fftwf_execute_dft) execute_dft_func;
        static const decltype(&fftwf_alignment_of) alignment_of_func;
        static const decltype(&fftwf_destroy_plan) destroy_plan_func;
        static const decltype(&fftwf_alloc_complex) alloc_func;
        static const decltype(&fftwf_free) free_func;
        static const decltype(&fftwf_plan_guru_dft) plan_guru_func;

        // static constexpr auto execute_func = fftwf_execute;
        // static constexpr auto execute_dft_func = fftwf_execute_dft;
        // static constexpr auto alignment_of_func = fftwf_alignment_of;
        // static constexpr auto destroy_plan_func = fftwf_destroy_plan;
        // static constexpr auto alloc_func = fftwf_alloc_complex;
        // static constexpr auto free_func = fftwf_free;
        // static constexpr auto plan_guru_func = fftwf_plan_guru_dft;
};

template<> struct MHO_FFTWTypes< double >
{
        using complex_type = std::complex< double >;
        using complex_type_ptr = std::complex< double >*;
        using fftw_complex_type = fftw_complex;
        using fftw_complex_type_ptr = fftw_complex*;
        using iodim_type = fftw_iodim;
        using plan_type = fftw_plan;

        static const decltype(&fftw_execute) execute_func;
        static const decltype(&fftw_execute_dft) execute_dft_func;
        static const decltype(&fftw_alignment_of) alignment_of_func;
        static const decltype(&fftw_destroy_plan) destroy_plan_func;
        static const decltype(&fftw_alloc_complex) alloc_func;
        static const decltype(&fftw_free) free_func;
        static const decltype(&fftw_plan_guru_dft) plan_guru_func;

        // static constexpr auto execute_func = fftw_execute;
        // static constexpr auto execute_dft_func = fftw_execute_dft;
        // static constexpr auto alignment_of_func = fftw_alignment_of;
        // static constexpr auto destroy_plan_func = fftw_destroy_plan;
        // static constexpr auto alloc_func = fftw_alloc_complex;
        // static constexpr auto free_func = fftw_free;
        // static constexpr auto plan_guru_func = fftw_plan_guru_dft;
};

template<> struct MHO_FFTWTypes< long double >
{
        using complex_type = std::complex< long double >;
        using complex_type_ptr = std::complex< long double >*;
        using fftw_complex_type = fftwl_complex;
        using fftw_complex_type_ptr = fftwl_complex*;
        using iodim_type = fftwl_iodim;
        using plan_type = fftwl_plan;

        static const decltype(&fftwl_execute) execute_func;
        static const decltype(&fftwl_execute_dft) execute_dft_func;
        static const decltype(&fftwl_alignment_of) alignment_of_func;
        static const decltype(&fftwl_destroy_plan) destroy_plan_func;
        static const decltype(&fftwl_alloc_complex) alloc_func;
        static const decltype(&fftwl_free) free_func;
        static const decltype(&fftwl_plan_guru_dft) plan_guru_func;

        // static constexpr auto execute_func = fftwl_execute;
        // static constexpr auto execute_dft_func = fftwl_execute_dft;
        // static constexpr auto alignment_of_func = fftwl_alignment_of;
        // static constexpr auto destroy_plan_func = fftwl_destroy_plan;
        // static constexpr auto alloc_func = fftwl_alloc_complex;
        // static constexpr auto free_func = fftwl_free;
        // static constexpr auto plan_guru_func = fftwl_plan_guru_dft;
};


//version info
class MHO_FFTWTypeInfo
{
    public:
        MHO_FFTWTypeInfo();
        virtual ~MHO_FFTWTypeInfo();
        
        
    static std::string get_fftw_version_raw()
    {
        //examples:
        //fftw-3.3.3-sse2
        //fftw-3.3.8-sse2-avx
        std::string vstr = fftw_version;
        return vstr;
    }
    
    static std::vector< std::string > split_version_string(std::string vstr, char delim)
    {
        std::vector<std::string> parts;
        std::size_t start = 0;
        std::size_t end = 0; 
        while((end = vstr.find(delim, start)) != std::string::npos)
        { 
            parts.push_back(vstr.substr(start, end - start));
            start = end + 1; 
        }
        parts.push_back(vstr.substr(start));
        return parts;
    }
    
    static std::string get_fftw_version_numeric()
    {
        std::string vstr = fftw_version;
        std::vector<std::string> parts = split_version_string(vstr, '-');
        for(std::size_t i=0; i<parts.size(); i++)
        {
            std::size_t ndots = std::count(parts[i].begin(), parts[i].end(), '.');
            if(ndots == 2)
            {
                return parts[i];
            }
        }
        //otherwise return a garbage value 
        vstr = "0.0.0";
        return vstr;
    }

    static int get_fftw_version_major()
    {
        std::string vstr = get_fftw_version_numeric();
        std::vector<std::string> parts = split_version_string(vstr, '.');
        std::stringstream ss;
        if(parts.size() < 1){return 0;}
        ss << parts[0];
        int value;
        ss >> value;
        return value;
    }
    
    static int get_fftw_version_minor()
    {
        std::string vstr = get_fftw_version_numeric();
        std::vector<std::string> parts = split_version_string(vstr, '.');
        if(parts.size() < 2){return 0;}
        std::stringstream ss;
        ss << parts[1];
        int value;
        ss >> value;
        return value;
    }
    
    static int get_fftw_version_patch()
    {
        std::string vstr = get_fftw_version_numeric();
        std::vector<std::string> parts = split_version_string(vstr, '.');
        if(parts.size() < 3){return 0;}
        std::stringstream ss;
        ss << parts[2];
        int value;
        ss >> value;
        return value;
    }
    
};


} // namespace hops

#endif /*! end of include guard: MHO_FFTWTypes */
