#include <iostream>
#include <string>
#include <complex>
#include "MHO_Types.hh"

#ifndef MHO_NumpyTypeCode_HH__
#define MHO_NumpyTypeCode_HH__

/*!
*@file MHO_NumpyTypeCode.hh
*@class
*@author J. Barrett - barrettj@mit.edu
*@date Thu May 15 02:13:28 PM UTC 2025
*@brief
*/

// Type                                 Type code                     Description
// int8, uint8                          i1, u1      Signed and unsigned 8-bit (1-byte) integer types
// int16, uint16                        i2, u2      Signed and unsigned 16-Bit (2 Byte) integer types
// int32, uint32                        i4, u4      Signed and unsigned 32-Bit (4 Byte) integer types
// int64, uint64                        i8, u8      Signed and unsigned 64-Bit (8 Byte) integer types

// float32                              f4 or f     Standard floating point with single precision; compatible with C float
// float64                              f8 or d     Standard floating point with double precision; compatible with C double and Python float object
// complex64, complex128,               c8, c16     Complex numbers represented by two 32, 64 or 128 floating point numbers respectively

//UNSUPPORTED:
// bool             ?    Boolean type that stores the values True and False
// object           O    Python object type; a value can be any Python object
// string_          S    ASCII string type with fixed length (1 byte per character); to create a string type with length 7, for example, use S7; longer inputs are truncated without warning
// unicode_         U    Unicode type with fixed length where the number of bytes is platform-specific; uses the same specification semantics as string_, e.g. U7
// float16          f2   Standard floating point with half precision

namespace hops 
{

//generic returns empty string
/**
 * @brief Returns an empty string for any given type XType in MHO postprocessing system.
 * 
 * @return Empty std::string
 */
template< typename XType > std::string MHO_NumpyTypeCode()
{
    std::string type_code = "";
    return type_code;
};

//only the below specializations are supported, everything else will return empty 
/**
 * @brief Function MHO_NumpyTypeCode<int8_t>
 * 
 * @return Return value (std::string)
 */
template<> inline std::string MHO_NumpyTypeCode< int8_t >(){  return "i1"; }
/**
 * @brief Function MHO_NumpyTypeCode<uint8_t>
 * 
 * @tparam inline Template parameter inline
 * @return Return value (std::string)
 */
template<> inline std::string MHO_NumpyTypeCode< uint8_t >(){  return "u1"; }
/**
 * @brief Function MHO_NumpyTypeCode<int16_t>
 * 
 * @tparam inline Template parameter inline
 * @return Return value (std::string)
 */
template<> inline std::string MHO_NumpyTypeCode< int16_t >(){  return "i2"; }
/**
 * @brief Function MHO_NumpyTypeCode<uint16_t>
 * 
 * @tparam inline Template parameter inline
 * @return Return value (std::string)
 */
template<> inline std::string MHO_NumpyTypeCode< uint16_t >(){  return "u2"; }
/**
 * @brief Function MHO_NumpyTypeCode<int32_t>
 * 
 * @tparam inline Template parameter inline
 * @return Return value (std::string)
 */
template<> inline std::string MHO_NumpyTypeCode< int32_t >(){  return "i4"; }
/**
 * @brief Function MHO_NumpyTypeCode<uint32_t>
 * 
 * @tparam inline Template parameter inline
 * @return Return value (std::string)
 */
template<> inline std::string MHO_NumpyTypeCode< uint32_t >(){  return "u4"; }
/**
 * @brief Function MHO_NumpyTypeCode<int64_t>
 * 
 * @tparam inline Template parameter inline
 * @return Return value (std::string)
 */
template<> inline std::string MHO_NumpyTypeCode< int64_t >(){  return "i8"; }
/**
 * @brief Function MHO_NumpyTypeCode<uint64_t>
 * 
 * @tparam inline Template parameter inline
 * @return Return value (std::string)
 */
template<> inline std::string MHO_NumpyTypeCode< uint64_t >(){  return "u8"; }

/**
 * @brief Function MHO_NumpyTypeCode<float>
 * 
 * @tparam inline Template parameter inline
 * @return Return value (std::string)
 */
template<> inline std::string MHO_NumpyTypeCode< float >(){  return "f4"; }
/**
 * @brief Function MHO_NumpyTypeCode<double>
 * 
 * @tparam inline Template parameter inline
 * @return Return value (std::string)
 */
template<> inline std::string MHO_NumpyTypeCode< double >(){  return "f8"; }

/**
 * @brief Function MHO_NumpyTypeCode<std::complex<float>>
 * 
 * @tparam inline Template parameter inline
 * @return Return value (std::string)
 */
template<> inline std::string MHO_NumpyTypeCode< std::complex<float> >(){  return "c8"; }
/**
 * @brief Function MHO_NumpyTypeCode<std::complex<double>>
 * 
 * @tparam inline Template parameter inline
 * @return Return value (std::string)
 */
template<> inline std::string MHO_NumpyTypeCode< std::complex<double> >(){  return "c16"; }

}// end namespace

#endif /*! end of include guard:  */
