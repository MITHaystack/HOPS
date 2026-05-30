#include <iostream>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "MHO_ClassIdentity.hh"
#include "MHO_ContainerDefinitions.hh"
#include "MHO_ContainerDictionary.hh"
#include "MHO_ContainerStore.hh"
#include "MHO_ContainerTypeTags.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"
#include "MHO_UUID.hh"

using namespace hops;

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    //  Build objects in memory (store takes ownership)
    visibility_type* v1 = new visibility_type();
    v1->Resize(1, 2, 2, 2);

    visibility_type* v2 = new visibility_type();
    v2->Resize(1, 1, 1, 1);

    weight_type* w1 = new weight_type();
    w1->Resize(1, 2, 2, 2);

    MHO_UUID id1 = v1->GetObjectUUID();
    MHO_UUID id2 = v2->GetObjectUUID();
    MHO_UUID id3 = w1->GetObjectUUID();

    MHO_ContainerStore store;

    //  C1: AddObject and counts
    REQUIRE(store.AddObject(v1) == true);
    REQUIRE(store.AddObject(v2) == true);
    REQUIRE(store.AddObject(w1) == true);

    REQUIRE(store.GetNObjects() == 3);
    REQUIRE(store.GetNObjects< visibility_type >() == 2);
    REQUIRE(store.GetNObjects< weight_type >() == 1);

    //  C2: AddObject(nullptr)
    {
        visibility_type* n = nullptr;
        bool result = store.AddObject(n);
        REQUIRE(result == false);
        REQUIRE(store.GetNObjects() == 3); // unchanged
    }

    //  C3: Retrieval by object UUID
    {
        // typed retrieval
        visibility_type* retrieved = store.GetObject< visibility_type >(id1);
        REQUIRE(retrieved == v1);

        // base (untyped) retrieval
        MHO_Serializable* base = store.GetObject(id1);
        REQUIRE(base == static_cast< MHO_Serializable* >(v1));

        // bogus UUID
        MHO_UUID bogus; // zeroed
        visibility_type* miss = store.GetObject< visibility_type >(bogus);
        REQUIRE(miss == nullptr);

        // IsObjectPresent
        REQUIRE(store.IsObjectPresent(id1) == true);
        REQUIRE(store.IsObjectPresent(bogus) == false);
    }

    //  C4: Retrieval by index
    {
        visibility_type* ptr0 = store.GetObject< visibility_type >(0);
        visibility_type* ptr1 = store.GetObject< visibility_type >(1);
        REQUIRE(ptr0 != nullptr);
        REQUIRE(ptr1 != nullptr);
        REQUIRE(ptr0 != ptr1);

        // set membership: {ptr0, ptr1} == {v1, v2}
        std::set< visibility_type* > actual;
        actual.insert(ptr0);
        actual.insert(ptr1);
        std::set< visibility_type* > expected;
        expected.insert(v1);
        expected.insert(v2);
        REQUIRE(actual == expected);

        // out of range
        visibility_type* oob = store.GetObject< visibility_type >(2);
        REQUIRE(oob == nullptr);
    }

    //  C5: Short-name lifecycle
    {
        REQUIRE(store.SetShortName(id1, "vis_a") == true);
        REQUIRE(store.SetShortName(id3, "wt") == true);

        // duplicate short name should fail
        REQUIRE(store.SetShortName(id2, "vis_a") == false);

        REQUIRE(store.GetObjectUUID("vis_a") == id1);
        REQUIRE(store.GetShortName(id1) == "vis_a");

        visibility_type* byName = store.GetObject< visibility_type >("vis_a");
        REQUIRE(byName == v1);

        std::vector< std::string > names;
        store.GetAllShortNames(names);
        std::set< std::string > nameSet(names.begin(), names.end());
        REQUIRE(nameSet.count("vis_a") == 1);
        REQUIRE(nameSet.count("wt") == 1);
        REQUIRE(names.size() == 2);
    }

    //  C6: RenameObject
    {
        store.RenameObject("vis_a", "vis_renamed");

        MHO_UUID emptyUUID;
        REQUIRE(store.GetObjectUUID("vis_a") == emptyUUID);
        REQUIRE(store.GetObjectUUID("vis_renamed") == id1);
        REQUIRE(store.GetShortName(id1) == "vis_renamed");
    }

    //  C7: Type enumeration helpers
    {
        std::vector< MHO_UUID > types;
        store.GetAllTypeUUIDs(types);
        REQUIRE(types.size() == 2);

        MHO_UUID visType = store.GetTypeUUID< visibility_type >();
        MHO_UUID wtType = store.GetTypeUUID< weight_type >();

        std::set< MHO_UUID > typeSet(types.begin(), types.end());
        REQUIRE(typeSet.count(visType) == 1);
        REQUIRE(typeSet.count(wtType) == 1);

        std::vector< MHO_UUID > visIds;
        store.GetAllObjectUUIDsOfType(visType, visIds);
        REQUIRE(visIds.size() == 2);
        std::set< MHO_UUID > visIdSet(visIds.begin(), visIds.end());
        REQUIRE(visIdSet.count(id1) == 1);
        REQUIRE(visIdSet.count(id2) == 1);

        // GetObjectTypeUUID
        REQUIRE(store.GetObjectTypeUUID(id1) == visType);

        // GetAllObjectInfo
        std::vector< std::tuple< std::string, std::string, std::string > > info;
        info = store.GetAllObjectInfo();
        REQUIRE(info.size() == 3);

        // Check shortname fields in info: vis_renamed for v1, empty for v2, wt for w1
        bool foundRenamed = false, foundWt = false, foundEmpty = false;
        for(auto& t : info)
        {
            std::string sn = std::get< 2 >(t);
            if(sn == "vis_renamed")
                foundRenamed = true;
            if(sn == "wt")
                foundWt = true;
            if(sn.empty())
                foundEmpty = true;
        }
        REQUIRE(foundRenamed == true);
        REQUIRE(foundWt == true);
        REQUIRE(foundEmpty == true);
    }

    //  C8: DeleteObject(ptr) and DeleteObject(uuid)
    {
        // Delete w1 by pointer
        REQUIRE(store.DeleteObject(w1) == true);
        REQUIRE(store.GetNObjects() == 2);

        // "wt" short name should be gone
        MHO_UUID emptyUUID;
        REQUIRE(store.GetObjectUUID("wt") == emptyUUID);

        // Delete v2 by UUID
        REQUIRE(store.DeleteObject(id2) == true);
        REQUIRE(store.GetNObjects() == 1);

        // Double delete should fail
        REQUIRE(store.DeleteObject(id2) == false);

        // Do NOT dereference w1 or v2 here - they've been freed.
    }

    //  C9: Clear()
    {
        store.Clear();
        REQUIRE(store.GetNObjects() == 0);
        REQUIRE(store.GetNObjects< visibility_type >() == 0);
        REQUIRE(store.IsObjectPresent(id1) == false);
    }

    return 0;
}
