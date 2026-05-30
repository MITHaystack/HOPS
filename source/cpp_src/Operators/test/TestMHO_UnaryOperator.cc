#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TestAssertions.hh"
#include "MHO_UnaryOperator.hh"
#include <cmath>
#include <complex>
#include <cstddef>
#include <iostream>

using namespace hops;

template< typename T > class AddDelta: public hops::MHO_UnaryOperator< hops::MHO_NDArrayWrapper< T, 1 > >
{
    public:
        AddDelta(): fDelta(T(0)), fInitInCalls(0), fInitOutCalls(0), fExecInCalls(0), fExecOutCalls(0) {}

        T fDelta;
        int fInitInCalls;
        int fInitOutCalls;
        int fExecInCalls;
        int fExecOutCalls;

    protected:
        bool InitializeInPlace(hops::MHO_NDArrayWrapper< T, 1 >* in) override
        {
            ++fInitInCalls;
            return in != nullptr;
        }

        bool ExecuteInPlace(hops::MHO_NDArrayWrapper< T, 1 >* in) override
        {
            ++fExecInCalls;
            if(!in)
                return false;
            for(auto it = in->begin(); it != in->end(); ++it)
                *it = *it + fDelta;
            return true;
        }
};

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    using Arr = hops::MHO_NDArrayWrapper< double, 1 >;
    using CArr = hops::MHO_NDArrayWrapper< std::complex< double >, 1 >;

    // --- Test 1: In-place dispatch (real double) ---
    {
        Arr a(std::size_t(4));
        a.SetArray(1.0);
        AddDelta< double > op;
        op.fDelta = 2.5;
        op.SetArgs(&a);
        bool i = op.Initialize();
        bool e = op.Execute();
        REQUIRE(i == true);
        REQUIRE(e == true);
        REQUIRE(op.fInitInCalls == 1);
        REQUIRE(op.fExecInCalls == 1);
        REQUIRE(op.fInitOutCalls == 0);
        REQUIRE(op.fExecOutCalls == 0);
        for(std::size_t idx = 0; idx < 4; ++idx)
            REQUIRE(std::fabs(a(idx) - 3.5) < 1e-12);
    }

    // --- Test 2: Out-of-place dispatch via default Copy+ExecuteInPlace ---
    {
        Arr in(std::size_t(4));
        in.SetArray(1.0);
        Arr out;
        AddDelta< double > op;
        op.fDelta = 2.5;
        op.SetArgs(static_cast< const Arr* >(&in), &out);
        op.Initialize();
        op.Execute();
        REQUIRE(out.GetSize() == 4);
        for(std::size_t idx = 0; idx < 4; ++idx)
            REQUIRE(std::fabs(out(idx) - 3.5) < 1e-12);
        // in unchanged
        for(std::size_t idx = 0; idx < 4; ++idx)
            REQUIRE(std::fabs(in(idx)-1.0) < 1e-12);
        REQUIRE(op.fExecInCalls == 1);
        REQUIRE(op.fExecOutCalls == 0);
    }

    // --- Test 3: Mode switch (out-of-place -> in-place) ---
    {
        Arr a(std::size_t(3));
        a.SetArray(1.0);
        AddDelta< double > op;
        op.fDelta = 2.5;
        // First out-of-place
        Arr out;
        op.SetArgs(static_cast< const Arr* >(&a), &out);
        op.Initialize();
        op.Execute();
        // Then switch to in-place
        Arr b(std::size_t(3));
        b.SetArray(1.0);
        op.SetArgs(&b);
        op.Initialize();
        op.Execute();
        for(std::size_t idx = 0; idx < 3; ++idx)
            REQUIRE(std::fabs(b(idx) - 3.5) < 1e-12);
        REQUIRE(op.fExecInCalls > 0);
    }

    // --- Test 4: Mode switch (in-place -> out-of-place) ---
    {
        AddDelta< double > op;
        op.fDelta = 1.0;
        Arr a(std::size_t(2));
        a.SetArray(7.0);
        op.SetArgs(&a);
        op.Initialize();
        op.Execute(); // a -> 8.0
        REQUIRE(std::fabs(a(0) - 8.0) < 1e-12);
        Arr b;
        op.SetArgs((const Arr*)&a, &b);
        op.Initialize();
        op.Execute();
        // a unchanged
        REQUIRE(std::fabs(a(0) - 8.0) < 1e-12);
        REQUIRE(std::fabs(a(1) - 8.0) < 1e-12);
        REQUIRE(b.GetSize() == 2);
        REQUIRE(std::fabs(b(0) - 9.0) < 1e-12);
        REQUIRE(std::fabs(b(1) - 9.0) < 1e-12);
    }

    // --- Test 5: Complex template instantiation ---
    {
        CArr a(std::size_t(3));
        a.SetArray(std::complex< double >(1.0, -1.0));
        AddDelta< std::complex< double > > op;
        op.fDelta = std::complex< double >(0.5, 0.25);
        op.SetArgs(&a);
        op.Initialize();
        op.Execute();
        for(std::size_t idx = 0; idx < 3; ++idx)
        {
            REQUIRE(std::fabs(a(idx).real() - 1.5) < 1e-12);
            REQUIRE(std::fabs(a(idx).imag() - (-0.75)) < 1e-12);
        }
    }

    // --- Test 6: Null-pointer handling on in-place path ---
    {
        AddDelta< double > op;
        op.SetArgs(static_cast< Arr* >(nullptr));
        bool i = op.Initialize();
        REQUIRE(i == false);
    }

    // --- Test 7: Re-initialization ---
    {
        AddDelta< double > op;
        op.fDelta = 1.0;
        Arr a(std::size_t(2));
        a.SetArray(0.0);
        op.SetArgs(&a);
        op.Initialize();
        op.Execute();
        op.Initialize();
        op.Execute();
        for(std::size_t idx = 0; idx < 2; ++idx)
            REQUIRE(std::fabs(a(idx) - 2.0) < 1e-12);
        REQUIRE(op.fInitInCalls == 2);
        REQUIRE(op.fExecInCalls == 2);
    }

    return 0;
}
