#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "MHO_Message.hh"
#include "MHO_ParameterStore.hh"
#include "MHO_TestAssertions.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    // Case 1: Scalar round-trip (happy path)
    {
        MHO_ParameterStore store;

        bool ok = store.Set("/a/b/c", 42);
        REQUIRE(ok);
        ok = store.Set("/a/b/d", std::string("hello"));
        REQUIRE(ok);
        ok = store.Set("/x", 3.14);
        REQUIRE(ok);

        int ivalue = 0;
        ok = store.Get("/a/b/c", ivalue);
        REQUIRE(ok);
        REQUIRE(ivalue == 42);

        std::string sval;
        ok = store.Get("/a/b/d", sval);
        REQUIRE(ok);
        REQUIRE(sval == "hello");

        double dvalue = 0.0;
        ok = store.Get("/x", dvalue);
        REQUIRE(ok);
        CHECK_CLOSE(dvalue, 3.14, 1e-12);
    }

    // Case 2: Vector round-trip
    {
        MHO_ParameterStore store;

        std::vector<double> vinput{1.0, 2.0, 3.0};
        bool ok = store.Set("/global/dr_win", vinput);
        REQUIRE(ok);

        std::vector<double> vout;
        ok = store.Get("/global/dr_win", vout);
        REQUIRE(ok);
        REQUIRE(vout.size() == 3);
        CHECK_CLOSE(vout[0], 1.0, 1e-12);
        CHECK_CLOSE(vout[1], 2.0, 1e-12);
        CHECK_CLOSE(vout[2], 3.0, 1e-12);
    }

    // Case 3: mho_json overload (array/object)
    {
        MHO_ParameterStore store;

        mho_json arr = {1, 2, 3};
        mho_json obj = {{"k", "v"}};

        bool ok = store.Set("/arr", arr);
        REQUIRE(ok);
        ok = store.Set("/obj", obj);
        REQUIRE(ok);

        std::vector<int> v;
        ok = store.Get("/arr", v);
        REQUIRE(ok);
        REQUIRE(v.size() == 3);
        REQUIRE(v[0] == 1);
        REQUIRE(v[1] == 2);
        REQUIRE(v[2] == 3);

        std::string s;
        ok = store.Get("/obj/k", s);
        REQUIRE(ok);
        REQUIRE(s == "v");
    }

    // Case 4: Missing-path Get
    {
        MHO_ParameterStore store;

        int readback = 999;
        bool found = store.Get("/no/such/path", readback);
        REQUIRE(found == false);
        REQUIRE(readback == 999);
    }

    // Case 5: GetAs default on missing path
    {
        MHO_ParameterStore store;

        int i = store.GetAs<int>("/missing");
        REQUIRE(i == 0);

        std::string s = store.GetAs<std::string>("/missing");
        REQUIRE(s == "");

        double d = store.GetAs<double>("/missing");
        CHECK_CLOSE(d, 0.0, 1e-15);
    }

    // Case 6: Type coercion: int stored, read as double
    {
        MHO_ParameterStore store;

        bool ok = store.Set("/n", 7);
        REQUIRE(ok);

        double d = 0.0;
        ok = store.Get("/n", d);
        REQUIRE(ok);
        CHECK_CLOSE(d, 7.0, 1e-12);
    }

    // Case 7: Overwrite / idempotency
    {
        MHO_ParameterStore store;

        store.Set("/k", 1);
        store.Set("/k", 2);
        store.Set("/k", 2);

        REQUIRE(store.GetAs<int>("/k") == 2);
    }

    // Case 8: Path sanitization
    {
        MHO_ParameterStore store;

        bool ok = store.Set("  /a/b/  ", 5);
        REQUIRE(ok);

        REQUIRE(store.GetAs<int>("/a/b") == 5);
        REQUIRE(store.GetAs<int>("/a/b/") == 5);
        REQUIRE(store.IsPresent("/a/b") == true);
    }

    // Case 9: Deep nesting (>=4 levels)
    {
        MHO_ParameterStore store;

        bool ok = store.Set("/control/station/E/pc_mode/value", std::string("manual"));
        REQUIRE(ok);

        REQUIRE(store.GetAs<std::string>("/control/station/E/pc_mode/value") == "manual");
        REQUIRE(store.IsPresent("/control/station/E") == true);
    }

    // Case 10: IsPresent semantics
    {
        MHO_ParameterStore store;

        store.Set("/p/q", 1);

        REQUIRE(store.IsPresent("/p") == true);
        REQUIRE(store.IsPresent("/p/q") == true);
        REQUIRE(store.IsPresent("/p/zzz") == false);
        REQUIRE(store.IsPresent("/totally/absent") == false);
    }

    // Case 11: CopyFrom / FillData / DumpData / ClearData round-trip
    {
        MHO_ParameterStore storeA;
        storeA.Set("/alpha", 10);
        storeA.Set("/beta", std::string("world"));
        storeA.Set("/gamma/x", 3.14);

        // DumpData / FillData round-trip
        mho_json snapshot;
        storeA.DumpData(snapshot);
        MHO_ParameterStore storeB;
        storeB.FillData(snapshot);

        REQUIRE(storeB.GetAs<int>("/alpha") == 10);
        REQUIRE(storeB.GetAs<std::string>("/beta") == "world");
        CHECK_CLOSE(storeB.GetAs<double>("/gamma/x"), 3.14, 1e-12);

        // CopyFrom
        MHO_ParameterStore storeC;
        storeC.CopyFrom(storeA);

        REQUIRE(storeC.GetAs<int>("/alpha") == 10);
        REQUIRE(storeC.GetAs<std::string>("/beta") == "world");
        CHECK_CLOSE(storeC.GetAs<double>("/gamma/x"), 3.14, 1e-12);

        // ClearData
        storeA.ClearData();
        REQUIRE(storeA.IsPresent("/alpha") == false);
        REQUIRE(storeA.IsPresent("/beta") == false);
        REQUIRE(storeA.IsPresent("/gamma/x") == false);
    }

    return 0;
}
