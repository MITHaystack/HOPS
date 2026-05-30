#include <iostream>
#include <string>
#include <cstdint>

#include "MHO_Serializable.hh"
#include "MHO_MD5HashGenerator.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

/* Concrete subclasses (DetermineTypeUUID is private in base) */
class FooSer : public MHO_Serializable {
    public:
        FooSer() : MHO_Serializable() {}
        FooSer(const MHO_UUID& u) : MHO_Serializable(u) {}
        uint64_t GetSerializedSize() const override { return 16; }
    private:
        MHO_UUID DetermineTypeUUID() const override {
            MHO_MD5HashGenerator gen;
            gen.Initialize();
            std::string name = "FooSer";
            gen << name;
            gen.Finalize();
            return gen.GetDigestAsUUID();
        }
};

class BarSer : public MHO_Serializable {
    public:
        BarSer() : MHO_Serializable() {}
        uint64_t GetSerializedSize() const override { return 99; }
    private:
        MHO_UUID DetermineTypeUUID() const override {
            MHO_MD5HashGenerator gen;
            gen.Initialize();
            std::string name = "BarSer";
            gen << name;
            gen.Finalize();
            return gen.GetDigestAsUUID();
        }
};

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // Test 1: Default ctor generates a non-empty object UUID
    {
        FooSer a;
        MHO_UUID u = a.GetObjectUUID();
        REQUIRE(!u.is_empty());
    }

    // Test 2: Distinct default-constructed instances have distinct object UUIDs
    {
        FooSer a;
        FooSer b;
        REQUIRE(a.GetObjectUUID() != b.GetObjectUUID());
    }

    // Test 3: Object UUID is stable across repeated reads
    {
        FooSer a;
        MHO_UUID u1 = a.GetObjectUUID();
        MHO_UUID u2 = a.GetObjectUUID();
        REQUIRE(u1 == u2);
    }

    // Test 4: SetObjectUUID overrides
    {
        FooSer a;
        FooSer b;
        MHO_UUID u = b.GetObjectUUID();
        a.SetObjectUUID(u);
        REQUIRE(a.GetObjectUUID() == u);
        REQUIRE(a.GetObjectUUID() == b.GetObjectUUID());
    }

    // Test 5: UUID-adopting ctor
    {
        FooSer src;
        MHO_UUID u = src.GetObjectUUID();
        FooSer dst(u);
        REQUIRE(dst.GetObjectUUID() == u);
    }

    // Test 6: GetVersion default
    {
        FooSer a;
        REQUIRE(a.GetVersion() == 0);
    }

    // Test 7: GetSerializedSize returns the subclass value
    {
        FooSer a;
        BarSer c;
        REQUIRE(a.GetSerializedSize() == 16);
        REQUIRE(c.GetSerializedSize() == 99);
    }

    // Test 8: GetTypeUUID is stable and cached
    {
        FooSer a;
        MHO_UUID t1 = a.GetTypeUUID();
        MHO_UUID t2 = a.GetTypeUUID();
        REQUIRE(t1 == t2);
        REQUIRE(!t1.is_empty());
    }

    // Test 9: Type UUID is type-distinct and object-shared
    {
        FooSer a;
        FooSer b;
        BarSer c;
        REQUIRE(a.GetTypeUUID() == b.GetTypeUUID());
        REQUIRE(a.GetTypeUUID() != c.GetTypeUUID());
        REQUIRE(a.GetTypeUUID() != a.GetObjectUUID());
    }

    return 0;
}
