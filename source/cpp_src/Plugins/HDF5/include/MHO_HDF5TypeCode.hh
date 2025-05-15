#include <iostream>
#include <string>
#include <complex>
#include "MHO_Types.hh"

#include "hdf5.h"

#ifndef MHO_HDF5TypeCode_HH__
#define MHO_HDF5TypeCode_HH__

/*!
*@file MHO_HDF5TypeCode.hh
*@class
*@author J. Barrett - barrettj@mit.edu
*@date Thu May 15 02:13:28 PM UTC 2025
*@brief
*/

// Datatype 	Description 
// H5T_NATIVE_CHAR 	C-style char 
// H5T_NATIVE_SCHAR 	C-style signed char 
// H5T_NATIVE_UCHAR 	C-style unsigned signed char 
// H5T_NATIVE_SHORT 	C-style short 
// H5T_NATIVE_USHORT 	C-style unsigned short 
// H5T_NATIVE_INT 	C-style int 
// H5T_NATIVE_UINT 	C-style unsigned int 
// H5T_NATIVE_LONG 	C-style long 
// H5T_NATIVE_ULONG 	C-style unsigned long 
// H5T_NATIVE_LLONG 	C-style long long 
// H5T_NATIVE_ULLONG 	C-style unsigned long long 
// H5T_NATIVE_FLOAT16 	C-style _Float16 (May be H5I_INVALID_HID if platform doesn't support _Float16 type) 
// H5T_NATIVE_FLOAT 	C-style float 
// H5T_NATIVE_DOUBLE 	C-style double 
// H5T_NATIVE_LDOUBLE 	C-style long double 
// H5T_NATIVE_FLOAT_COMPLEX 	C-style float _Complex (MSVC _Fcomplex) (May be H5I_INVALID_HID if platform doesn't support float _Complex / _Fcomplex type) 
// H5T_NATIVE_DOUBLE_COMPLEX 	C-style double _Complex (MSVC _Dcomplex) (May be H5I_INVALID_HID if platform doesn't support double _Complex / _Dcomplex type) 
// H5T_NATIVE_LDOUBLE_COMPLEX 	C-style long double _Complex (MSVC _Lcomplex) (May be H5I_INVALID_HID if platform doesn't support long double _Complex / _Lcomplex type) 
// H5T_NATIVE_B8 	8-bit bitfield based on native types 
// H5T_NATIVE_B16 	16-bit bitfield based on native types 
// H5T_NATIVE_B32 	32-bit bitfield based on native types 
// H5T_NATIVE_B64 	64-bit bitfield based on native types 
// H5T_NATIVE_OPAQUE 	opaque unit based on native types 
// H5T_NATIVE_HADDR 	address type based on native types 
// H5T_NATIVE_HSIZE 	size type based on native types 
// H5T_NATIVE_HSSIZE 	signed size type based on native types 
// H5T_NATIVE_HERR 	error code type based on native types 
// H5T_NATIVE_HBOOL 	Boolean type based on native types 

namespace hops 
{

//generic returns invalid
template< typename XType > hid_t MHO_HDF5TypeCode()
{
    return H5I_INVALID_HID;
};

//only the below specializations are supported, everything else will return empty 
template<> inline hid_t MHO_HDF5TypeCode< int8_t >(){  return H5T_NATIVE_CHAR; }
template<> inline hid_t MHO_HDF5TypeCode< uint8_t >(){  return H5T_NATIVE_UCHAR; }
template<> inline hid_t MHO_HDF5TypeCode< int16_t >(){  return H5T_NATIVE_SHORT; }
template<> inline hid_t MHO_HDF5TypeCode< uint16_t >(){  return H5T_NATIVE_USHORT; }
template<> inline hid_t MHO_HDF5TypeCode< int32_t >(){  return H5T_NATIVE_INT; }
template<> inline hid_t MHO_HDF5TypeCode< uint32_t >(){  return H5T_NATIVE_UINT; }
template<> inline hid_t MHO_HDF5TypeCode< int64_t >(){  return H5T_NATIVE_LLONG; }
template<> inline hid_t MHO_HDF5TypeCode< uint64_t >(){  return H5T_NATIVE_ULLONG; }

template<> inline hid_t MHO_HDF5TypeCode< float >(){  return H5T_NATIVE_FLOAT; }
template<> inline hid_t MHO_HDF5TypeCode< double >(){  return H5T_NATIVE_DOUBLE; }

// template<> inline hid_t MHO_HDF5TypeCode< std::complex<float> >(){  return H5T_NATIVE_FLOAT_COMPLEX; }
// template<> inline hid_t MHO_HDF5TypeCode< std::complex<double> >(){  return H5T_NATIVE_DOUBLE_COMPLEX; }

}//end namespace

#endif /*! end of include guard:  */
