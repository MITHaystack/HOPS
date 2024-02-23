#ifndef MHO_Types_HH__
#define MHO_Types_HH__

/*!
*@file
*@class
*@date
*@brief
* The static asserts are here to trigger build failure if the sizes of these types
* are not as expecte. Systems with implementation defined sizes which differ from
* these will not produce portable data files, so we don't allow compilation.
* For now, disabling this is not user-configurable.
*@author J. Barrett - barrettj@mit.edu
*/

#include <complex>
#include <cstdint>


#define ENSURE_PORTABILITY

#ifdef ENSURE_PORTABILITY

static_assert( sizeof(char) == 1, "char size is not 1.");
static_assert( sizeof(bool) == 1, "bool size is not 1.");
static_assert( sizeof(int64_t) == 8, "int64_t size is not 8.");
static_assert( sizeof(uint64_t) == 8, "uint64_t size is not 8.");
static_assert( sizeof(double) == 8, "double size is not 8.");
static_assert( sizeof(float) == 4, "float size is not 4.");
static_assert( sizeof( std::complex<double> ) == 16, "std::complex<double> size is not 16.");
static_assert( sizeof( std::complex<float> ) == 8, "std::complex<float> size is not 8.");

#endif


#endif /*! end of include guard: MHO_Types_HH__ */
