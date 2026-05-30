#include <complex>
#include <cstdio>
#include <iostream>
#include <set>
#include <string>

#include "MHO_ClassIdentity.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerJSONConverter.hh"
#include "MHO_Message.hh"
#include "MHO_ObjectTags.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

// C1 - Scalar at eJSONBasic

int test_c1_scalar_basic()
{
    MHO_ScalarContainer< double > s;
    s.SetValue(42.5);
    s.Insert("note", std::string("hello"));

    MHO_ContainerJSONConverter< MHO_ScalarContainer< double > > conv;
    conv.SetObjectToConvert(&s);
    conv.SetLevelOfDetail(eJSONBasic);
    conv.ConstructJSONRepresentation();

    mho_json* jptr = conv.GetJSON();
    REQUIRE(jptr != nullptr);
    mho_json& j = *jptr;

    // Must contain basic keys
    REQUIRE(j.contains("class_name"));
    REQUIRE(j.contains("class_uuid"));
    REQUIRE(j.contains("rank"));
    REQUIRE(j.contains("total_size"));

    // Verify values
    REQUIRE(j.at("rank") == 0);
    REQUIRE(j.at("total_size") == 1);
    REQUIRE(j.at("class_name") == MHO_ClassIdentity::ClassName< MHO_ScalarContainer< double > >());

    std::string expected_uuid = MHO_ClassIdentity::GetUUIDFromClass< MHO_ScalarContainer< double > >().as_string();
    REQUIRE(j.at("class_uuid") == expected_uuid);

    // At eJSONBasic, must NOT contain "tags" or "data"
    REQUIRE(j.contains("tags") == false);
    REQUIRE(j.contains("data") == false);
    return 0;
}

// C2 - Scalar at eJSONTags adds tags

int test_c2_scalar_tags()
{
    MHO_ScalarContainer< double > s;
    s.SetValue(42.5);
    s.Insert("note", std::string("hello"));

    MHO_ContainerJSONConverter< MHO_ScalarContainer< double > > conv;
    conv.SetObjectToConvert(&s);
    conv.SetLevelOfDetail(eJSONTags);
    conv.ConstructJSONRepresentation();

    mho_json& j = *conv.GetJSON();

    // Must now contain "tags"
    REQUIRE(j.contains("tags"));
    REQUIRE(j.at("tags").contains("note"));
    REQUIRE(j.at("tags").at("note") == "hello");

    // Still no "data"
    REQUIRE(j.contains("data") == false);
    return 0;
}

// C3 - Scalar at eJSONAll adds data

int test_c3_scalar_all()
{
    MHO_ScalarContainer< double > s;
    s.SetValue(42.5);

    MHO_ContainerJSONConverter< MHO_ScalarContainer< double > > conv;
    conv.SetObjectToConvert(&s);
    conv.SetLevelOfDetail(eJSONAll);
    conv.ConstructJSONRepresentation();

    mho_json& j = *conv.GetJSON();

    REQUIRE(j.contains("data"));
    REQUIRE(j.at("data").is_array());
    REQUIRE(static_cast< std::size_t >(j.at("data").size()) == 1);
    REQUIRE(j.at("data").at(0) == 42.5);
    return 0;
}

// C4 - Vector LOD progression

int test_c4_vector_lod()
{
    MHO_VectorContainer< double > v(4);
    for(std::size_t i = 0; i < 4; i++)
    {
        v[i] = static_cast< double >(i) * 1.5;
    }
    v.Insert("len", 4);

    MHO_ContainerJSONConverter< MHO_VectorContainer< double > > conv;
    conv.SetObjectToConvert(&v);

    // Basic
    conv.SetLevelOfDetail(eJSONBasic);
    conv.ConstructJSONRepresentation();
    {
        mho_json& j = *conv.GetJSON();
        REQUIRE(j.contains("class_name"));
        REQUIRE(j.at("class_name") == MHO_ClassIdentity::ClassName< MHO_VectorContainer< double > >());
        REQUIRE(j.contains("rank"));
        REQUIRE(j.at("rank") == 1);
        REQUIRE(j.contains("total_size"));
        REQUIRE(j.at("total_size") == 4);
    }

    // Tags
    conv.SetLevelOfDetail(eJSONTags);
    conv.ConstructJSONRepresentation();
    {
        mho_json& j = *conv.GetJSON();
        REQUIRE(j.contains("tags"));
        REQUIRE(j.at("tags").contains("len"));
        // CBOR encodes small positive ints as unsigned; accept either 4 or 4u
        bool len_match = (j.at("tags").at("len") == 4) || (j.at("tags").at("len") == 4u);
        REQUIRE(len_match);
    }

    // All
    conv.SetLevelOfDetail(eJSONAll);
    conv.ConstructJSONRepresentation();
    {
        mho_json& j = *conv.GetJSON();
        REQUIRE(j.contains("data"));
        REQUIRE(j.at("data").is_array());
        REQUIRE(static_cast< std::size_t >(j.at("data").size()) == 4);
        REQUIRE(j.at("data").at(0) == 0.0);
        REQUIRE(j.at("data").at(1) == 1.5);
        REQUIRE(j.at("data").at(2) == 3.0);
        REQUIRE(j.at("data").at(3) == 4.5);
    }
    return 0;
}

// C5 - Table with axes (eJSONWithAxes)

int test_c5_table_axes()
{
    // weight_type is MHO_TableContainer<double, baseline_axis_pack>
    // baseline_axis_pack has 4 axes: polprod, channel, time, frequency
    std::size_t dim[4] = {1, 2, 2, 2};
    weight_type t(dim);

    // Set axis coordinates
    auto* pol_axis = &(std::get< 0 >(t));
    for(std::size_t i = 0; i < pol_axis->GetSize(); i++)
        (*pol_axis)[i] = std::string("pol_") + std::to_string(i);

    auto* chan_axis = &(std::get< 1 >(t));
    for(std::size_t i = 0; i < chan_axis->GetSize(); i++)
        (*chan_axis)[i] = static_cast< double >(i) * 100.0;

    auto* time_axis = &(std::get< 2 >(t));
    for(std::size_t i = 0; i < time_axis->GetSize(); i++)
        (*time_axis)[i] = static_cast< double >(i) * 10.0;

    auto* freq_axis = &(std::get< 3 >(t));
    for(std::size_t i = 0; i < freq_axis->GetSize(); i++)
        (*freq_axis)[i] = static_cast< double >(i) * 1000.0;

    t.Insert("baseline", std::string("GsWf"));

    MHO_ContainerJSONConverter< weight_type > conv;
    conv.SetObjectToConvert(&t);
    conv.SetLevelOfDetail(eJSONWithAxes);
    conv.ConstructJSONRepresentation();

    mho_json& j = *conv.GetJSON();

    // Basic fields
    REQUIRE(j.contains("class_name"));
    REQUIRE(j.at("class_name") == MHO_ClassIdentity::ClassName< weight_type >());
    REQUIRE(j.contains("rank"));
    REQUIRE(j.at("rank") == 4);
    REQUIRE(j.contains("total_size"));
    REQUIRE(j.at("total_size") == 8);

    // Axes should be present as axis_0, axis_1, axis_2, axis_3
    std::size_t expected_sizes[4] = {1, 2, 2, 2};
    for(std::size_t idx = 0; idx < 4; idx++)
    {
        std::string axis_key = "axis_" + std::to_string(idx);
        REQUIRE(j.contains(axis_key));
        mho_json& axis_j = j.at(axis_key);
        REQUIRE(axis_j.contains("total_size"));
        REQUIRE(axis_j.at("total_size") == expected_sizes[idx]);
    }
    return 0;
}

// C6 - MHO_ObjectTags specialization

int test_c6_object_tags()
{
    MHO_ObjectTags ot;
    ot.Insert("root", std::string("abcdef"));

    MHO_ContainerJSONConverter< MHO_ObjectTags > conv;
    conv.SetObjectToConvert(&ot);
    conv.SetLevelOfDetail(eJSONTags);
    conv.ConstructJSONRepresentation();

    mho_json& j = *conv.GetJSON();

    REQUIRE(j.contains("class_name"));
    REQUIRE(j.at("class_name") == MHO_ClassIdentity::ClassName< MHO_ObjectTags >());
    REQUIRE(j.contains("class_uuid"));
    REQUIRE(j.contains("tags"));
    REQUIRE(j.at("tags").contains("root"));
    REQUIRE(j.at("tags").at("root") == "abcdef");
    return 0;
}

// C7 - LOD monotonicity (key superset)

int test_c7_lod_monotonicity()
{
    MHO_VectorContainer< double > v(4);
    for(std::size_t i = 0; i < 4; i++)
        v[i] = static_cast< double >(i) * 1.5;
    v.Insert("len", 4);

    MHO_ContainerJSONConverter< MHO_VectorContainer< double > > conv;
    conv.SetObjectToConvert(&v);

    std::set< std::string > keys_basic, keys_tags, keys_all;

    // Collect keys at each LOD
    conv.SetLevelOfDetail(eJSONBasic);
    conv.ConstructJSONRepresentation();
    for(auto it = conv.GetJSON()->begin(); it != conv.GetJSON()->end(); ++it)
        keys_basic.insert(it.key());

    conv.SetLevelOfDetail(eJSONTags);
    conv.ConstructJSONRepresentation();
    for(auto it = conv.GetJSON()->begin(); it != conv.GetJSON()->end(); ++it)
        keys_tags.insert(it.key());

    conv.SetLevelOfDetail(eJSONAll);
    conv.ConstructJSONRepresentation();
    for(auto it = conv.GetJSON()->begin(); it != conv.GetJSON()->end(); ++it)
        keys_all.insert(it.key());

    // keys_basic subset-of keys_tags
    for(auto& k : keys_basic)
        REQUIRE(keys_tags.count(k) == 1);

    // keys_tags subset-of keys_all
    for(auto& k : keys_tags)
        REQUIRE(keys_all.count(k) == 1);

    // Also verify that higher LOD has strictly more keys
    REQUIRE(keys_basic.size() < keys_tags.size());
    REQUIRE(keys_tags.size() < keys_all.size());
    return 0;
}

// C8 - Wrong-type SetObjectToConvert is safe

int test_c8_wrong_type_safe()
{
    MHO_ScalarContainer< double > s;
    s.SetValue(42.5);

    MHO_ContainerJSONConverter< MHO_VectorContainer< double > > conv;
    conv.SetObjectToConvert(&s); // wrong type - dynamic_cast will fail

    // Should not crash
    conv.ConstructJSONRepresentation();

    // fContainer is nullptr after failed dynamic_cast, so fJSON remains default-constructed (empty)
    mho_json& j = *conv.GetJSON();
    REQUIRE(j.is_null() || j.empty());
    return 0;
}

// Main driver

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    if(test_c1_scalar_basic())
        return 1;
    if(test_c2_scalar_tags())
        return 1;
    if(test_c3_scalar_all())
        return 1;
    if(test_c4_vector_lod())
        return 1;
    if(test_c5_table_axes())
        return 1;
    if(test_c6_object_tags())
        return 1;
    if(test_c7_lod_monotonicity())
        return 1;
    if(test_c8_wrong_type_safe())
        return 1;

    return 0;
}
