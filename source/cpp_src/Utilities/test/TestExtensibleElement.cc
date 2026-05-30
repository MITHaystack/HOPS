#include <iostream>
#include <string>

#include "MHO_ExtensibleElement.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

/* Extension payload types */
struct PayloadA {
    PayloadA(MHO_ExtensibleElement* /*p*/) {}
    int    a = 0;
    void   set(int v){ a = v; }
};

struct PayloadB {
    PayloadB(MHO_ExtensibleElement* /*p*/) {}
    std::string b;
};

/* Concrete host */
struct Host : public MHO_ExtensibleElement {};

/* Visitor targeting PayloadA */
struct PayloadAVisitor :
    public MHO_ExtendedElement<PayloadA>::ExtendedVisitor {
    int visited = 0;
    int last_a  =1;
    void VisitExtendedElement(MHO_ExtendedElement<PayloadA>* el) override {
        visited++;
        last_a = el->a;
    }
};

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // Test 1: No extensions initially
    {
        Host h;
        bool has = h.HasExtension<PayloadA>();
        auto* p = h.AsExtension<PayloadA>();
        REQUIRE(has == false);
        REQUIRE(p == nullptr);
    }

    // Test 2: MakeExtension creates and is queryable
    {
        Host h;
        auto* ea = h.MakeExtension<PayloadA>();
        ea->set(7);
        REQUIRE(ea != nullptr);
        REQUIRE(h.HasExtension<PayloadA>() == true);
        REQUIRE(h.AsExtension<PayloadA>() == ea);
        REQUIRE(h.AsExtension<PayloadA>()->a == 7);
    }

    // Test 3: Distinct extension types coexist
    {
        Host h;
        auto* ea = h.MakeExtension<PayloadA>();
        ea->set(3);
        auto* eb = h.MakeExtension<PayloadB>();
        eb->b = "hi";
        REQUIRE(h.HasExtension<PayloadA>() == true);
        REQUIRE(h.HasExtension<PayloadB>() == true);
        REQUIRE(h.AsExtension<PayloadB>()->b == "hi");
        REQUIRE(static_cast<const void*>(ea) != static_cast<const void*>(eb));
    }

    // Test 4: MakeExtension replaces same-type extension
    {
        Host h;
        auto* e1 = h.MakeExtension<PayloadA>();
        e1->set(1);
        auto* e2 = h.MakeExtension<PayloadA>();
        REQUIRE(e2 != nullptr);
        REQUIRE(h.AsExtension<PayloadA>() == e2);
        REQUIRE(e2->a == 0);  // freshly default-constructed
        // e1 is dangling do not dereference
        REQUIRE(h.HasExtension<PayloadA>() == true);
    }

    // Test 5: Accept dispatches to matching ExtendedVisitor exactly once
    {
        Host h;
        auto* ea = h.MakeExtension<PayloadA>();
        ea->set(42);
        h.MakeExtension<PayloadB>();  // unrelated, should be skipped
        PayloadAVisitor v;
        h.Accept(&v);
        REQUIRE(v.visited == 1);
        REQUIRE(v.last_a == 42);
    }

    // Test 6: Accept with no matching extension is a no-op
    {
        Host h;
        h.MakeExtension<PayloadB>();
        PayloadAVisitor v;
        h.Accept(&v);
        REQUIRE(v.visited == 0);
    }

    // Test 7: Destructor frees extensions without crash
    {
        Host* host = new Host();
        host->MakeExtension<PayloadA>();
        host->MakeExtension<PayloadB>();
        delete host;
    }

    return 0;
}
