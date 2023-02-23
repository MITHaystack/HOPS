#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerDefinitions.hh"

#define AddTable1(TYPE1)                                                       \
AddClassType< MHO_AxisPack_##TYPE1 >();                                        \
AddClassType< MHO_TableContainer<Int, MHO_AxisPack_##TYPE1 > >();              \
AddClassType< MHO_TableContainer<Double, MHO_AxisPack_##TYPE1 > >();           \
AddClassType< MHO_TableContainer<ComplexD, MHO_AxisPack_##TYPE1 > >();         \
AddClassType< MHO_TableContainer<ComplexF, MHO_AxisPack_##TYPE1 > >();

#define AddTable2(TYPE1,TYPE2)                                                 \
AddClassType< MHO_AxisPack_##TYPE1##_##TYPE2 >();                              \
AddClassType< MHO_TableContainer<Int, MHO_AxisPack_##TYPE1##_##TYPE2 > >();    \
AddClassType< MHO_TableContainer<Double, MHO_AxisPack_##TYPE1##_##TYPE2 > >(); \
AddClassType< MHO_TableContainer<ComplexD, MHO_AxisPack_##TYPE1##_##TYPE2 > >();\
AddClassType< MHO_TableContainer<ComplexF, MHO_AxisPack_##TYPE1##_##TYPE2 > >();

#define AddTable3(TYPE1, TYPE2, TYPE3)                                         \
AddClassType< MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3 >();                    \
AddClassType< MHO_TableContainer<Int, MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3 > >();\
AddClassType< MHO_TableContainer<Double, MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3 > >();\
AddClassType< MHO_TableContainer<ComplexD, MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3 > >();\
AddClassType< MHO_TableContainer<ComplexF, MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3 > >();

#define AddTable4(TYPE1,TYPE2,TYPE3,TYPE4)                                     \
AddClassType< MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3##_##TYPE4 >();          \
AddClassType< MHO_TableContainer< Int, MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3##_##TYPE4 > >();\
AddClassType< MHO_TableContainer< Double, MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3##_##TYPE4 > >();\
AddClassType< MHO_TableContainer< ComplexD, MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3##_##TYPE4 > >();\
AddClassType< MHO_TableContainer< ComplexF, MHO_AxisPack_##TYPE1##_##TYPE2##_##TYPE3##_##TYPE4 > >();


namespace hops
{

MHO_ContainerDictionary::MHO_ContainerDictionary()
{
    AddClassType<MHO_ObjectTags>();

    //NOTE: Most of the different types below are not needed or used anywhere 
    //so we could easily trim this long list down to a much more focused set of 
    //objects. However, having a wide variety defined already lets us handle stuff 
    //which may fit into these objects but which has not been defined yet, and 
    //demonstrate how to add support for new types

    #ifndef HOPS_BUILD_EXTRA_CONTAINERS

    //only add the bare minimum types here
    AddClassType<baseline_axis_pack>();
    AddClassType<visibility_type>();
    AddClassType<weight_type>();
    
    AddClassType<multitone_pcal_type>();
    AddClassType<station_coord_axis_pack>();
    AddClassType<station_coord_type>();

    //storage types explicitly meant for on-disk format
    AddClassType<visibility_store_type>();
    AddClassType<weight_store_type>();

    //other extraneous types 
    AddClassType<uch_baseline_axis_pack>();
    AddClassType<uch_visibility_type>(); 
    AddClassType<uch_weight_type>(); 

    #else

    #pragma message("Building extra data containers types.")


    //add all of the types we define
    AddClassType<MHO_ScalarBool>();
    AddClassType<MHO_ScalarChar>();
    AddClassType<MHO_ScalarUChar>();
    AddClassType<MHO_ScalarShort>();
    AddClassType<MHO_ScalarUShort>();
    AddClassType<MHO_ScalarInt>();
    AddClassType<MHO_ScalarUInt>();
    AddClassType<MHO_ScalarLong>();
    AddClassType<MHO_ScalarULong>();
    AddClassType<MHO_ScalarLongLong>();
    AddClassType<MHO_ScalarULongLong>();
    AddClassType<MHO_ScalarFloat>();
    AddClassType<MHO_ScalarDouble>();
    AddClassType<MHO_ScalarLongDouble>();
    AddClassType<MHO_ScalarComplexFloat>();
    AddClassType<MHO_ScalarComplexDouble>();
    AddClassType<MHO_ScalarComplexLongDouble>();
    AddClassType<MHO_ScalarString>();

    //boolean vectors are a problem due underlying impl of std::vector<bool (see MHO_NDArrayWrapper_1.hh line 195)
    //AddClassType<MHO_VectorBool>(); 
    AddClassType<MHO_VectorChar>();
    AddClassType<MHO_VectorUChar>();
    AddClassType<MHO_VectorShort>();
    AddClassType<MHO_VectorUShort>();
    AddClassType<MHO_VectorInt>();
    AddClassType<MHO_VectorUInt>();
    AddClassType<MHO_VectorLong>();
    AddClassType<MHO_VectorULong>();
    AddClassType<MHO_VectorLongLong>();
    AddClassType<MHO_VectorULongLong>();
    AddClassType<MHO_VectorFloat>();
    AddClassType<MHO_VectorDouble>();
    AddClassType<MHO_VectorLongDouble>();
    AddClassType<MHO_VectorComplexFloat>();
    AddClassType<MHO_VectorComplexDouble>();
    AddClassType<MHO_VectorComplexLongDouble>();
    AddClassType<MHO_VectorString>();

    //AddClassType<MHO_AxisBool>(); //no bools
    AddClassType<MHO_AxisChar>();
    AddClassType<MHO_AxisUChar>();
    AddClassType<MHO_AxisShort>();
    AddClassType<MHO_AxisUShort>();
    AddClassType<MHO_AxisInt>();
    AddClassType<MHO_AxisUInt>();
    AddClassType<MHO_AxisLong>();
    AddClassType<MHO_AxisULong>();
    AddClassType<MHO_AxisLongLong>();
    AddClassType<MHO_AxisULongLong>();
    AddClassType<MHO_AxisFloat>();
    AddClassType<MHO_AxisDouble>();
    AddClassType<MHO_AxisLongDouble>();
    AddClassType<MHO_AxisComplexFloat>();
    AddClassType<MHO_AxisComplexDouble>();
    AddClassType<MHO_AxisComplexLongDouble>();
    AddClassType<MHO_AxisString>();

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

    #endif

};

}//end of namespace
