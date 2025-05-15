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


namespace hops 
{

//helper structs and functions needed for complex numbers

typedef struct h5helper_complex_float 
{
    double real;
    double imag;
}
h5helper_complex_float;


typedef struct h5helper_complex_double 
{
    double real;
    double imag;
}
h5helper_complex_double; 

hid_t 
create_complex_dtype_float() 
{
    hid_t complex_dtype = H5Tcreate(H5T_COMPOUND, sizeof(h5helper_complex_float));
    H5Tinsert(complex_dtype, "real", HOFFSET(h5helper_complex_float, real), H5T_NATIVE_FLOAT);
    H5Tinsert(complex_dtype, "imag", HOFFSET(h5helper_complex_float, imag), H5T_NATIVE_FLOAT);
    return complex_dtype;
}

//needed for std::complex<double>
hid_t 
create_complex_dtype_double() 
{
    hid_t complex_dtype = H5Tcreate(H5T_COMPOUND, sizeof(h5helper_complex_double));
    H5Tinsert(complex_dtype, "real", HOFFSET(h5helper_complex_double, real), H5T_NATIVE_DOUBLE);
    H5Tinsert(complex_dtype, "imag", HOFFSET(h5helper_complex_double, imag), H5T_NATIVE_DOUBLE);
    return complex_dtype;
}

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

template<> inline hid_t MHO_HDF5TypeCode< std::complex<float> >()
{
    return create_complex_dtype_float();
}

template<> inline hid_t MHO_HDF5TypeCode< std::complex<double> >()
{
    return create_complex_dtype_double();
}

}//end namespace

#endif /*! end of include guard:  */
