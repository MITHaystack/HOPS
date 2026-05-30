#include <iostream>
#include <string>
#include <vector>

#include "MHO_ClassIdentity.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerTypeTags.hh"
#include "MHO_Message.hh"
#include "MHO_Serializable.hh"
#include "MHO_TestAssertions.hh"
#include "MHO_UUID.hh"

using namespace hops;

// Probe types from the minimal (always-registered) set.
typedef visibility_type T1;
typedef weight_type T2;
typedef mbd_dr_type T3;
typedef station_coord_type T4;
typedef baseline_axis_pack T5;
typedef MHO_ObjectTags T6;

// C1 - Core types registered

template< typename T > bool test_c1(MHO_ContainerDictionary& dict)
{
    MHO_UUID u = dict.GetUUIDFor< T >();
    if(u.is_empty())
        return false;
    if(dict.IsTypePresent(u) != true)
        return false;
    return true;
}

// C2 - Name <-> UUID round-trip

template< typename T > bool test_c2(MHO_ContainerDictionary& dict)
{
    std::string name = MHO_ClassIdentity::ClassName< T >();
    MHO_UUID u = dict.GetUUIDFromClassName(name);
    MHO_UUID uDirect = dict.GetUUIDFor< T >();
    if(u != uDirect)
        return false;

    std::string name2 = dict.GetClassNameFromUUID(u);
    if(name2 != name)
        return false;
    return true;
}

// C3 - Frozen-UUID agreement with live objects

template< typename T > bool test_c3(MHO_ContainerDictionary& dict)
{
    T obj;
    MHO_UUID dictUUID = dict.GetUUIDFor< T >();
    MHO_UUID objUUID = obj.GetTypeUUID();
    return (dictUUID == objUUID);
}

// C4 - GenerateContainerFromUUID

template< typename T > bool test_c4a(MHO_ContainerDictionary& dict)
{
    MHO_UUID u = dict.GetUUIDFor< T >();
    MHO_Serializable* obj = dict.GenerateContainerFromUUID(u);
    if(obj == nullptr)
        return false;

    T* tp = dynamic_cast< T* >(obj);
    bool ok = (tp != nullptr);
    delete obj; // caller owns the returned pointer
    return ok;
}

bool test_c4b(MHO_ContainerDictionary& dict)
{
    MHO_UUID bogus; // zeroed UUID
    MHO_Serializable* obj = dict.GenerateContainerFromUUID(bogus);
    return (obj == nullptr);
}

// C5 - Unknown-lookup behavior

bool test_c5(MHO_ContainerDictionary& dict)
{
    MHO_UUID unknownUUID = dict.GetUUIDFromClassName("NoSuchType");
    std::string unknownName = dict.GetClassNameFromUUID(unknownUUID);
    bool present = dict.IsTypePresent(unknownUUID);

    // GetUUIDFromClassName for unknown name returns a zeroed/empty UUID
    if(unknownUUID.is_empty() != true)
        return false;

    // GetClassNameFromUUID for unknown UUID returns "unknown"
    if(unknownName != "unknown")
        return false;

    // IsTypePresent must be false for unknown UUID
    if(present != false)
        return false;

    return true;
}

// C6 - Determinism across multiple instances

template< typename T > bool test_c6(const MHO_ContainerDictionary& d1, const MHO_ContainerDictionary& d2)
{
    return (d1.GetUUIDFor< T >() == d2.GetUUIDFor< T >());
}

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    MHO_ContainerDictionary dict;

    //  C1: Core types registered
    REQUIRE(test_c1< T1 >(dict));
    REQUIRE(test_c1< T2 >(dict));
    REQUIRE(test_c1< T3 >(dict));
    REQUIRE(test_c1< T4 >(dict));
    REQUIRE(test_c1< T5 >(dict));
    REQUIRE(test_c1< T6 >(dict));

    //  C2: Name <-> UUID round-trip
    REQUIRE(test_c2< T1 >(dict));
    REQUIRE(test_c2< T2 >(dict));
    REQUIRE(test_c2< T3 >(dict));
    REQUIRE(test_c2< T4 >(dict));
    REQUIRE(test_c2< T5 >(dict));
    REQUIRE(test_c2< T6 >(dict));

    //  C3: Frozen-UUID agreement with live objects
    REQUIRE(test_c3< T1 >(dict));
    REQUIRE(test_c3< T2 >(dict));
    REQUIRE(test_c3< T3 >(dict));
    REQUIRE(test_c3< T4 >(dict));
    REQUIRE(test_c3< T5 >(dict));
    REQUIRE(test_c3< T6 >(dict));

    //  C4a: GenerateContainerFromUUID (registered type)
    REQUIRE(test_c4a< T1 >(dict));
    REQUIRE(test_c4a< T2 >(dict));
    REQUIRE(test_c4a< T3 >(dict));
    REQUIRE(test_c4a< T4 >(dict));
    REQUIRE(test_c4a< T6 >(dict));
    // T5 (baseline_axis_pack) is an MHO_AxisPack subclass, not
    // directly an MHO_Serializable, so skip for dynamic_cast test

    //  C4b: GenerateContainerFromUUID (unknown UUID)
    REQUIRE(test_c4b(dict));

    //  C5: Unknown-lookup behavior
    REQUIRE(test_c5(dict));

    //  C6: Determinism across instances
    {
        MHO_ContainerDictionary dict2;
        REQUIRE(test_c6< T1 >(dict, dict2));
        REQUIRE(test_c6< T2 >(dict, dict2));
        REQUIRE(test_c6< T3 >(dict, dict2));
        REQUIRE(test_c6< T4 >(dict, dict2));
        REQUIRE(test_c6< T5 >(dict, dict2));
        REQUIRE(test_c6< T6 >(dict, dict2));
    }

    return 0;
}
