#include "MHO_Message.hh"
#include "MHO_Operator.hh"
#include "MHO_TestAssertions.hh"
#include <iostream>
#include <limits>
#include <string>

using namespace hops;

//Minimal concrete subclass to satisfy the pure-virtual contract.
class DummyOp: public MHO_Operator
{
    public:
        DummyOp(): fInitCount(0), fExecCount(0), fInitReturn(true), fExecReturn(true) {}

        bool Initialize() override
        {
            ++fInitCount;
            return fInitReturn;
        }

        bool Execute() override
        {
            ++fExecCount;
            return fExecReturn;
        }

        int fInitCount;
        int fExecCount;
        bool fInitReturn;
        bool fExecReturn;
};

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // --- Test 1: Defaults ---
    {
        DummyOp op;
        REQUIRE(op.GetName() == "unknown");
        REQUIRE(op.Priority() == std::numeric_limits< double >::max());
    }

    // --- Test 2: Name set/get round-trip ---
    {
        DummyOp op;
        const std::string name = "my_op";
        op.SetName(name);
        REQUIRE(op.GetName() == "my_op");

        // Empty string is allowed.
        op.SetName("");
        REQUIRE(op.GetName() == "");
    }

    // --- Test 3: Priority set/get round-trip ---
    {
        DummyOp op;
        op.SetPriority(-1.5);
        double a = op.Priority();
        REQUIRE(a == -1.5);

        op.SetPriority(0.0);
        double b = op.Priority();
        REQUIRE(b == 0.0);

        op.SetPriority(1e9);
        double c = op.Priority();
        REQUIRE(c == 1e9);
    }

    // --- Test 4: Initialize / Execute dispatch ---
    {
        DummyOp op;
        op.fInitReturn = true;
        op.fExecReturn = true;
        bool i1 = op.Initialize();
        bool e1 = op.Execute();
        REQUIRE(i1 == true);
        REQUIRE(e1 == true);
        REQUIRE(op.fInitCount == 1);
        REQUIRE(op.fExecCount == 1);
    }

    // --- Test 5: Re-initialization / idempotency contract ---
    {
        DummyOp op;
        op.fInitReturn = true;
        op.fExecReturn = true;
        REQUIRE(op.Initialize() == true);
        REQUIRE(op.Initialize() == true);
        REQUIRE(op.fInitCount == 2);

        REQUIRE(op.Execute() == true);
        REQUIRE(op.Execute() == true);
        REQUIRE(op.fExecCount == 2);
    }

    // --- Test 6: Failure-status propagation ---
    {
        DummyOp op;
        op.fInitReturn = false;
        op.fExecReturn = false;
        bool i = op.Initialize();
        bool e = op.Execute();
        REQUIRE(i == false);
        REQUIRE(e == false);
    }

    // --- Test 7: Polymorphic destruction ---
    {
        MHO_Operator* p = new DummyOp();
        delete p;
        // Success = no crash, no undefined behavior.
    }

    return 0;
}
