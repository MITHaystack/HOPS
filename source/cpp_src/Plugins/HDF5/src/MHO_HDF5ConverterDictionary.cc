#include "MHO_HDF5ConverterDictionary.hh"
#include "MHO_ContainerDefinitions.hh"

#define AddTable1(TYPE1)                                                                                                       \
    AddHDF5ClassType< MHO_AxisPack_##TYPE1 >();                                                                                    \
    AddHDF5ClassType< MHO_TableContainer< Int, MHO_AxisPack_##TYPE1 > >();                                                         \
    AddHDF5ClassType< MHO_TableContainer< Double, MHO_AxisPack_##TYPE1 > >();                                                      \
    AddHDF5ClassType< MHO_TableContainer< ComplexD, MHO_AxisPack_##TYPE1 > >();                                                    \
    AddHDF5ClassType< MHO_TableContainer< ComplexF, MHO_AxisPack_##TYPE1 > >();

#define AddTable2(TYPE1, TYPE2)                                                                                                \
    AddHDF5ClassType< MHO_AxisPack_##TYPE1##_##TYPE2 >();                                                                          \
    AddHDF5ClassType< MHO_TableContainer< Int, MHO_AxisPack_##TYPE1##_##TYPE2 > >();                                               \
    AddHDF5ClassType< MHO_TableContainer< Double, MHO_AxisPack_##TYPE1##_##TYPE2 > >();                                            \
    AddHDF5ClassType< MHO_TableContainer< ComplexD, MHO_AxisPack_##TYPE1##_##TYPE2 > >();                                          \
    AddHDF5ClassType< MHO_TableContainer< ComplexF, MHO_AxisPack_##TYPE1##_##TYPE2 > >();

#define AddTable3(TYPE1, TYPE2, TYPE3)                                                                                         \
    AddHDF5ClassType< MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3 >();                                                                \
    AddHDF5ClassType< MHO_TableContainer< Int, MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3 > >();                                     \
    AddHDF5ClassType< MHO_TableContainer< Double, MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3 > >();                                  \
    AddHDF5ClassType< MHO_TableContainer< ComplexD, MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3 > >();                                \
    AddHDF5ClassType< MHO_TableContainer< ComplexF, MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3 > >();

#define AddTable4(TYPE1, TYPE2, TYPE3, TYPE4)                                                                                  \
    AddHDF5ClassType< MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3##_##TYPE4 >();                                                      \
    AddHDF5ClassType< MHO_TableContainer< Int, MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3##_##TYPE4 > >();                           \
    AddHDF5ClassType< MHO_TableContainer< Double, MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3##_##TYPE4 > >();                        \
    AddHDF5ClassType< MHO_TableContainer< ComplexD, MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3##_##TYPE4 > >();                      \
    AddHDF5ClassType< MHO_TableContainer< ComplexF, MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3##_##TYPE4 > >();

namespace hops
{

MHO_HDF5ConverterDictionary::MHO_HDF5ConverterDictionary()
{
    AddHDF5ClassType< MHO_ObjectTags >();

    //NOTE: Most of the different types below are not needed or used anywhere
    //so we could easily trim this long list down to a much more focused set of
    //objects. However, having a wide variety defined already lets us handle stuff
    //which may fit into these objects but which has not been defined yet, and
    //demonstrate how to add support for new types

#ifndef HOPS_BUILD_EXTRA_CONTAINERS

    //only add the bare minimum types here
    AddHDF5ClassType< baseline_axis_pack >();
    AddHDF5ClassType< visibility_type >();
    AddHDF5ClassType< weight_type >();

    AddHDF5ClassType< mbd_dr_axis_pack >();
    AddHDF5ClassType< mbd_dr_type >();
    AddHDF5ClassType< mbd_dr_amp_type >();

    AddHDF5ClassType< multitone_pcal_type >();
    AddHDF5ClassType< station_coord_axis_pack >();
    AddHDF5ClassType< station_coord_type >();

    //storage types explicitly meant for on-disk format
    AddHDF5ClassType< visibility_store_type >();
    AddHDF5ClassType< weight_store_type >();

    //other extraneous types
    AddHDF5ClassType< uch_baseline_axis_pack >();
    AddHDF5ClassType< uch_visibility_type >();
    AddHDF5ClassType< uch_weight_type >();
    AddHDF5ClassType< uch_visibility_store_type >();
    AddHDF5ClassType< uch_weight_store_type >();

#else

    //only add the bare minimum types here
    AddHDF5ClassType< baseline_axis_pack >();
    AddHDF5ClassType< visibility_type >();
    AddHDF5ClassType< weight_type >();

    AddHDF5ClassType< mbd_dr_axis_pack >();
    AddHDF5ClassType< mbd_dr_type >();
    AddHDF5ClassType< mbd_dr_amp_type >();

    AddHDF5ClassType< multitone_pcal_type >();
    AddHDF5ClassType< station_coord_axis_pack >();
    AddHDF5ClassType< station_coord_type >();

    //storage types explicitly meant for on-disk format
    AddHDF5ClassType< visibility_store_type >();
    AddHDF5ClassType< weight_store_type >();

    //other extraneous types
    AddHDF5ClassType< uch_baseline_axis_pack >();
    AddHDF5ClassType< uch_visibility_type >();
    AddHDF5ClassType< uch_weight_type >();
    AddHDF5ClassType< uch_visibility_store_type >();
    AddHDF5ClassType< uch_weight_store_type >();

    //add all of the types we define
    AddHDF5ClassType< MHO_ScalarBool >();
    AddHDF5ClassType< MHO_ScalarChar >();
    AddHDF5ClassType< MHO_ScalarUChar >();
    AddHDF5ClassType< MHO_ScalarShort >();
    AddHDF5ClassType< MHO_ScalarUShort >();
    AddHDF5ClassType< MHO_ScalarInt >();
    AddHDF5ClassType< MHO_ScalarUInt >();
    AddHDF5ClassType< MHO_ScalarLong >();
    AddHDF5ClassType< MHO_ScalarULong >();
    AddHDF5ClassType< MHO_ScalarLongLong >();
    AddHDF5ClassType< MHO_ScalarULongLong >();
    AddHDF5ClassType< MHO_ScalarFloat >();
    AddHDF5ClassType< MHO_ScalarDouble >();
    AddHDF5ClassType< MHO_ScalarLongDouble >();
    AddHDF5ClassType< MHO_ScalarComplexFloat >();
    AddHDF5ClassType< MHO_ScalarComplexDouble >();
    AddHDF5ClassType< MHO_ScalarComplexLongDouble >();
    AddHDF5ClassType< MHO_ScalarString >();

    //boolean vectors are a problem due underlying impl of std::vector<bool (see MHO_NDArrayWrapper_1.hh line 195)
    //AddHDF5ClassType<MHO_VectorBool>();
    AddHDF5ClassType< MHO_VectorChar >();
    AddHDF5ClassType< MHO_VectorUChar >();
    AddHDF5ClassType< MHO_VectorShort >();
    AddHDF5ClassType< MHO_VectorUShort >();
    AddHDF5ClassType< MHO_VectorInt >();
    AddHDF5ClassType< MHO_VectorUInt >();
    AddHDF5ClassType< MHO_VectorLong >();
    AddHDF5ClassType< MHO_VectorULong >();
    AddHDF5ClassType< MHO_VectorLongLong >();
    AddHDF5ClassType< MHO_VectorULongLong >();
    AddHDF5ClassType< MHO_VectorFloat >();
    AddHDF5ClassType< MHO_VectorDouble >();
    AddHDF5ClassType< MHO_VectorLongDouble >();
    AddHDF5ClassType< MHO_VectorComplexFloat >();
    AddHDF5ClassType< MHO_VectorComplexDouble >();
    AddHDF5ClassType< MHO_VectorComplexLongDouble >();
    AddHDF5ClassType< MHO_VectorString >();

    //AddHDF5ClassType<MHO_AxisBool>(); //no bools
    AddHDF5ClassType< MHO_AxisChar >();
    AddHDF5ClassType< MHO_AxisUChar >();
    AddHDF5ClassType< MHO_AxisShort >();
    AddHDF5ClassType< MHO_AxisUShort >();
    AddHDF5ClassType< MHO_AxisInt >();
    AddHDF5ClassType< MHO_AxisUInt >();
    AddHDF5ClassType< MHO_AxisLong >();
    AddHDF5ClassType< MHO_AxisULong >();
    AddHDF5ClassType< MHO_AxisLongLong >();
    AddHDF5ClassType< MHO_AxisULongLong >();
    AddHDF5ClassType< MHO_AxisFloat >();
    AddHDF5ClassType< MHO_AxisDouble >();
    AddHDF5ClassType< MHO_AxisLongDouble >();
    AddHDF5ClassType< MHO_AxisComplexFloat >();
    AddHDF5ClassType< MHO_AxisComplexDouble >();
    AddHDF5ClassType< MHO_AxisComplexLongDouble >();
    AddHDF5ClassType< MHO_AxisString >();

    ////////////////////////////////////////////////////////////////////////////////

    AddTable1(Int);
    AddTable1(Double);
    AddTable1(String);

    ////////////////////////////////////////////////////////////////////////////////

    AddTable2(Int, Int);
    AddTable2(Int, Double);
    AddTable2(Int, String);
    AddTable2(Double, Int);
    AddTable2(Double, Double);
    AddTable2(Double, String);
    AddTable2(String, Int);
    AddTable2(String, Double);
    AddTable2(String, String);

    ////////////////////////////////////////////////////////////////////////////////

    AddTable3(Int, Int, Int);
    AddTable3(Int, Int, Double);
    AddTable3(Int, Int, String);
    AddTable3(Int, Double, Int);
    AddTable3(Int, Double, Double);
    AddTable3(Int, Double, String);
    AddTable3(Int, String, Int);
    AddTable3(Int, String, Double);
    AddTable3(Int, String, String);

    AddTable3(Double, Int, Int);
    AddTable3(Double, Int, Double);
    AddTable3(Double, Int, String);
    AddTable3(Double, Double, Int);
    AddTable3(Double, Double, Double);
    AddTable3(Double, Double, String);
    AddTable3(Double, String, Int);
    AddTable3(Double, String, Double);
    AddTable3(Double, String, String);

    AddTable3(String, Int, Int);
    AddTable3(String, Int, Double);
    AddTable3(String, Int, String);
    AddTable3(String, Double, Int);
    AddTable3(String, Double, Double);
    AddTable3(String, Double, String);
    AddTable3(String, String, Int);
    AddTable3(String, String, Double);
    AddTable3(String, String, String);

    ////////////////////////////////////////////////////////////////////////////////
    /*

    AddTable4(Int, Int, Int, Int);
    AddTable4(Int, Int, Int, Double);
    AddTable4(Int, Int, Int, String);
    AddTable4(Int, Int, Double, Int);
    AddTable4(Int, Int, Double, Double);
    AddTable4(Int, Int, Double, String);
    AddTable4(Int, Int, String, Int);
    AddTable4(Int, Int, String, Double);
    AddTable4(Int, Int, String, String);

    AddTable4(Int, Double, Int, Int);
    AddTable4(Int, Double, Int, Double);
    AddTable4(Int, Double, Int, String);
    AddTable4(Int, Double, Double, Int);
    AddTable4(Int, Double, Double, Double);
    AddTable4(Int, Double, Double, String);
    AddTable4(Int, Double, String, Int);
    AddTable4(Int, Double, String, Double);
    AddTable4(Int, Double, String, String);

    AddTable4(Int, String, Int, Int);
    AddTable4(Int, String, Int, Double);
    AddTable4(Int, String, Int, String);
    AddTable4(Int, String, Double, Int);
    AddTable4(Int, String, Double, Double);
    AddTable4(Int, String, Double, String);
    AddTable4(Int, String, String, Int);
    AddTable4(Int, String, String, Double);
    AddTable4(Int, String, String, String);

    ////////////

    AddTable4(Double, Int, Int, Int);
    AddTable4(Double, Int, Int, Double);
    AddTable4(Double, Int, Int, String);
    AddTable4(Double, Int, Double, Int);
    AddTable4(Double, Int, Double, Double);
    AddTable4(Double, Int, Double, String);
    AddTable4(Double, Int, String, Int);
    AddTable4(Double, Int, String, Double);
    AddTable4(Double, Int, String, String);

    AddTable4(Double, Double, Int, Int);
    AddTable4(Double, Double, Int, Double);
    AddTable4(Double, Double, Int, String);
    AddTable4(Double, Double, Double, Int);
    AddTable4(Double, Double, Double, Double);
    AddTable4(Double, Double, Double, String);
    AddTable4(Double, Double, String, Int);
    AddTable4(Double, Double, String, Double);
    AddTable4(Double, Double, String, String);

    AddTable4(Double, String, Int, Int);
    AddTable4(Double, String, Int, Double);
    AddTable4(Double, String, Int, String);
    AddTable4(Double, String, Double, Int);
    AddTable4(Double, String, Double, Double);
    AddTable4(Double, String, Double, String);
    AddTable4(Double, String, String, Int);
    AddTable4(Double, String, String, Double);
    AddTable4(Double, String, String, String);

    ////////////

    AddTable4(String, Int, Int, Int);
    AddTable4(String, Int, Int, Double);
    AddTable4(String, Int, Int, String);
    AddTable4(String, Int, Double, Int);
    AddTable4(String, Int, Double, Double);
    AddTable4(String, Int, Double, String);
    AddTable4(String, Int, String, Int);
    AddTable4(String, Int, String, Double);
    AddTable4(String, Int, String, String);

    AddTable4(String, Double, Int, Int);
    AddTable4(String, Double, Int, Double);
    AddTable4(String, Double, Int, String);
    AddTable4(String, Double, Double, Int);
    AddTable4(String, Double, Double, Double);
    AddTable4(String, Double, Double, String);
    AddTable4(String, Double, String, Int);
    AddTable4(String, Double, String, Double);
    AddTable4(String, Double, String, String);

    AddTable4(String, String, Int, Int);
    AddTable4(String, String, Int, Double);
    AddTable4(String, String, Int, String);
    AddTable4(String, String, Double, Int);
    AddTable4(String, String, Double, Double);
    AddTable4(String, String, Double, String);
    AddTable4(String, String, String, Int);
    AddTable4(String, String, String, Double);
    AddTable4(String, String, String, String);

    */

#endif
};

} // namespace hops
