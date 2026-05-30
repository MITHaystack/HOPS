#include <iostream>
#include <memory>
#include <string>

#include "MHO_Message.hh"
#include "MHO_Operator.hh"
#include "MHO_OperatorToolbox.hh"

using namespace hops;

// Inline operator subclasses for type discrimination

class OpA: public MHO_Operator
{
    public:
        bool Initialize() override { return true; }

        bool Execute() override { return true; }
};

class OpB: public MHO_Operator
{
    public:
        bool Initialize() override { return true; }

        bool Execute() override { return true; }
};

#include "MHO_TestAssertions.hh"

int main()
{
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // ---- Test Case 1: Empty toolbox ----
    {
        MHO_OperatorToolbox tb;
        REQUIRE(tb.GetNOperators() == 0);
        REQUIRE(tb.GetOperator("foo") == nullptr);
        REQUIRE(tb.GetAllOperatorsByName("foo").size() == 0);
        REQUIRE(tb.GetAllOperators().size() == 0);
        REQUIRE(tb.GetOperatorsByPriorityRange(-1.0, 1.0).size() == 0);
        REQUIRE(tb.GetOperatorsByCategory("anything").size() == 0);
        REQUIRE(tb.GetNOperatorsInCategory("x") == 0);
    }

    // ---- Test Case 2: Single insert + lookup by name (string and C-string) ----
    {
        MHO_OperatorToolbox tb;
        std::unique_ptr< MHO_Operator > p(new OpA());
        MHO_Operator* raw = p.get();
        tb.AddOperator(std::move(p), "alpha", "cat1");

        MHO_Operator* g1 = tb.GetOperator(std::string("alpha"));
        MHO_Operator* g2 = tb.GetOperator("alpha"); // const char* overload
        MHO_Operator* miss = tb.GetOperator("bogus");

        REQUIRE(g1 == raw);
        REQUIRE(g2 == raw);
        REQUIRE(miss == nullptr);
        REQUIRE(tb.GetNOperators() == 1);
    }

    // ---- Test Case 3: GetOperatorAs<T>() casting ----
    {
        MHO_OperatorToolbox tb;
        std::unique_ptr< MHO_Operator > p(new OpA());
        tb.AddOperator(std::move(p), "alpha", "cat1");

        OpA* a = tb.GetOperatorAs< OpA >("alpha");
        OpB* b = tb.GetOperatorAs< OpB >("alpha");
        OpA* miss = tb.GetOperatorAs< OpA >("bogus");

        REQUIRE(a != nullptr);
        REQUIRE(b == nullptr);
        REQUIRE(miss == nullptr);
    }

    // ---- Test Case 4: Category lookup ----
    {
        MHO_OperatorToolbox tb;

        std::unique_ptr< MHO_Operator > opA1(new OpA());
        tb.AddOperator(std::move(opA1), "a1", "catA");

        std::unique_ptr< MHO_Operator > opA2(new OpA());
        tb.AddOperator(std::move(opA2), "a2", "catA");

        std::unique_ptr< MHO_Operator > opB1(new OpA());
        tb.AddOperator(std::move(opB1), "b1", "catB");

        auto va = tb.GetOperatorsByCategory("catA");
        auto vb = tb.GetOperatorsByCategory("catB");
        auto vc = tb.GetOperatorsByCategory("catC");

        REQUIRE(va.size() == 2);
        REQUIRE(vb.size() == 1);
        REQUIRE(vc.size() == 0);
        REQUIRE(tb.GetNOperatorsInCategory("catA") == 2);
        REQUIRE(tb.GetNOperatorsInCategory("catB") == 1);
        REQUIRE(tb.GetNOperatorsInCategory("catC") == 0);
    }

    // ---- Test Case 5: Priority-range lookup AND sorted-by-priority ordering ----
    {
        MHO_OperatorToolbox tb;

        std::unique_ptr< MHO_Operator > op1(new OpA());
        op1.get()->SetPriority(10.0);
        tb.AddOperator(std::move(op1), "a", "c");

        std::unique_ptr< MHO_Operator > op2(new OpA());
        op2.get()->SetPriority(5.0);
        tb.AddOperator(std::move(op2), "b", "c");

        std::unique_ptr< MHO_Operator > op3(new OpA());
        op3.get()->SetPriority(1.0);
        tb.AddOperator(std::move(op3), "c", "c");

        // GetAllOperators should be sorted ascending by priority: [op3(1), op2(5), op1(10)]
        auto all = tb.GetAllOperators();
        REQUIRE(all.size() == 3);

        MHO_Operator* op3ptr = tb.GetOperator("c"); // priority 1.0
        MHO_Operator* op2ptr = tb.GetOperator("b"); // priority 5.0
        MHO_Operator* op1ptr = tb.GetOperator("a"); // priority 10.0
        REQUIRE(all[0] == op3ptr);
        REQUIRE(all[1] == op2ptr);
        REQUIRE(all[2] == op1ptr);

        // Priority range [2.0, 9.0): only op2 (5.0) should match
        auto range = tb.GetOperatorsByPriorityRange(2.0, 9.0);
        REQUIRE(range.size() == 1);
        REQUIRE(range[0] == op2ptr);
    }

    // ---- Test Case 6: GetAllOperatorsByName with shared name (replace_duplicate=false) ----
    {
        MHO_OperatorToolbox tb;

        std::unique_ptr< MHO_Operator > op1(new OpA());
        op1.get()->SetPriority(5.0);
        op1.get()->SetName("shared");
        tb.AddOperator(std::move(op1), "shared", "cat", false);

        std::unique_ptr< MHO_Operator > op2(new OpA());
        op2.get()->SetPriority(1.0);
        op2.get()->SetName("shared");
        tb.AddOperator(std::move(op2), "shared", "cat", false);

        auto v = tb.GetAllOperatorsByName("shared");
        MHO_Operator* byName = tb.GetOperator("shared");

        REQUIRE(v.size() == 2);
        REQUIRE(v[0]->Priority() == 1.0);
        REQUIRE(v[1]->Priority() == 5.0);
        REQUIRE(byName != nullptr);
        REQUIRE(tb.GetNOperators() == 2);
    }

    // ---- Test Case 7: Replace-on-duplicate (default true) ----
    {
        MHO_OperatorToolbox tb;

        std::unique_ptr< MHO_Operator > opA(new OpA());
        tb.AddOperator(std::move(opA), "x", "c");

        std::unique_ptr< MHO_Operator > opB(new OpB());
        tb.AddOperator(std::move(opB), "x", "c"); // default replace_duplicate=true

        MHO_Operator* g = tb.GetOperator("x");

        REQUIRE(tb.GetNOperators() == 1);
        REQUIRE(tb.GetNOperatorsInCategory("c") == 1);
        REQUIRE(g != nullptr);
        REQUIRE(dynamic_cast< OpB* >(g) != nullptr);
        REQUIRE(dynamic_cast< OpA* >(g) == nullptr);
    }

    // ---- Test Case 8: Destructor / ownership ----
    {
        {
            MHO_OperatorToolbox tb;
            std::unique_ptr< MHO_Operator > opA1(new OpA());
            tb.AddOperator(std::move(opA1), "d1", "dc");
            std::unique_ptr< MHO_Operator > opB1(new OpB());
            tb.AddOperator(std::move(opB1), "d2", "dc");
            std::unique_ptr< MHO_Operator > opA2(new OpA());
            tb.AddOperator(std::move(opA2), "d3", "dc");
            // tb goes out of scope here -- destructor should clean up all owned pointers
        }
        // If we reach here without crash, ownership is correct
        REQUIRE(true);
    }

    // ---- Test Case 9: Non-copyable / non-assignable (compile-time) ----
    // Verified by inspection of MHO_OperatorToolbox.hh:
    //   MHO_OperatorToolbox(const MHO_OperatorToolbox&) = delete;
    //   MHO_OperatorToolbox& operator=(const MHO_OperatorToolbox&) = delete;
    // Attempting to copy or assign would be a compile error; no runtime test.
    {
        REQUIRE(true);
    }

    return 0;
}
