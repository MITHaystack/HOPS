#ifndef MHO_ContainerTypeTags_HH__
#define MHO_ContainerTypeTags_HH__

#include "MHO_ClassIdentity.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ObjectTags.hh"

/*!
 *@file MHO_ContainerTypeTags.hh
 *@author J. Barrett - barrettj@mit.edu
 *@date Fri May 22 05:42:12 PM EDT 2026
 *@brief Declares frozen UUID tag registrations for every container type written to disk.
 *
 * Each HOPS_REGISTER_TYPE_TAG call below specializes hops::MHO_TypeTag< T > to
 * return a FIXED string literal. That string is the MD5 input used to compute
 * the type's UUID, so once a type is committed to on-disk format the literal
 * should never change. DO NOT EDIT the strings below: any change here changes
 * the type's UUID and will make all previously written files unreadable.
 * This is needed to guard against upstream GCC/clang changes which
 * could *hypothetically* affect the way class names are printed.
 *
 * The current strings exactly match what MHO_ClassName< T >() produces under
 * GCC/libstdc++ (GGC 11.4) at the time these tags were introduced. This preserves all
 * existing on-disk UUIDs and pins future UUIDs against compiler/stdlib/ABI
 * drift.
 *
 * This header must be included by every translation unit that computes a UUID
 * for one of these types (directly or via the container dictionary). Including
 * it from MHO_ContainerDefinitions.hh ensures that any code that uses the
 * container typedefs also sees the tag specializations.
 *
 * Note: Any future classes added/registered in MHO_ContainerDictionary for use in file I/O
 * *MUST* declare a HOPS_REGISTER_TYPE_TAG here to maintain UUID compatibility across platforms/future.
 */

HOPS_REGISTER_TYPE_TAG(MHO_ObjectTags,
"MHO_ObjectTags")

HOPS_REGISTER_TYPE_TAG(mbd_dr_axis_pack,
"MHO_AxisPack<MHO_Axis<double>,MHO_Axis<double>>")

HOPS_REGISTER_TYPE_TAG(baseline_axis_pack,
"MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>,MHO_Axis<double>>")

HOPS_REGISTER_TYPE_TAG(uch_baseline_axis_pack, //also aliased as multitone_pcal_axis_type (same underlying type)
"MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>>")

HOPS_REGISTER_TYPE_TAG(station_coord_axis_pack,
"MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<int>>")

HOPS_REGISTER_TYPE_TAG(mbd_dr_amp_type,
"MHO_TableContainer<double,MHO_AxisPack<MHO_Axis<double>,MHO_Axis<double>>>")

HOPS_REGISTER_TYPE_TAG(weight_type,
"MHO_TableContainer<double,MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>,MHO_Axis<double>>>")

HOPS_REGISTER_TYPE_TAG(uch_weight_type,
"MHO_TableContainer<double,MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>>>")

HOPS_REGISTER_TYPE_TAG(station_coord_type,
"MHO_TableContainer<double,MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<int>>>")

HOPS_REGISTER_TYPE_TAG(weight_store_type,
"MHO_TableContainer<float,MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>,MHO_Axis<double>>>")

HOPS_REGISTER_TYPE_TAG(uch_weight_store_type,
"MHO_TableContainer<float,MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>>>")

HOPS_REGISTER_TYPE_TAG(mbd_dr_type,
"MHO_TableContainer<std::complex<double>,MHO_AxisPack<MHO_Axis<double>,MHO_Axis<double>>>")

HOPS_REGISTER_TYPE_TAG(visibility_type,
"MHO_TableContainer<std::complex<double>,MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>,MHO_Axis<double>>>")

HOPS_REGISTER_TYPE_TAG(multitone_pcal_type, //also aliased as uch_visibility_type (same underlying type)
"MHO_TableContainer<std::complex<double>,MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>>>")

HOPS_REGISTER_TYPE_TAG(visibility_store_type,
"MHO_TableContainer<std::complex<float>,MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>,MHO_Axis<double>>>")

HOPS_REGISTER_TYPE_TAG(uch_visibility_store_type,
"MHO_TableContainer<std::complex<float>,MHO_AxisPack<MHO_Axis<string>,MHO_Axis<double>,MHO_Axis<double>>>")

#endif /*! end of include guard: MHO_ContainerTypeTags */
