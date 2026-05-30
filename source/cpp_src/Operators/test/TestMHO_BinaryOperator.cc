#include "MHO_BinaryOperator.hh"
#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TestAssertions.hh"
#include <cmath>
#include <complex>
#include <cstddef>
#include <iostream>

using namespace hops;

template< typename T >
class AddArrays: public hops::MHO_BinaryOperator< hops::MHO_NDArrayWrapper< T, 1 >, hops::MHO_NDArrayWrapper< T, 1 >,
                                                  hops::MHO_NDArrayWrapper< T, 1 > >
{
    public:
        AddArrays(): fInitCalls(0), fExecCalls(0) {}

        int fInitCalls;
        int fExecCalls;

    protected:
        bool InitializeImpl(const hops::MHO_NDArrayWrapper< T, 1 >* in1, const hops::MHO_NDArrayWrapper< T, 1 >* in2,
                            hops::MHO_NDArrayWrapper< T, 1 >* out) override
        {
            ++fInitCalls;
            if(!in1 || !in2 || !out)
                return false;
            std::size_t d1[1];
            std::size_t d2[1];
            in1->GetDimensions(d1);
            in2->GetDimensions(d2);
            if(d1[0] != d2[0])
                return false;
            std::size_t od[1];
            out->GetDimensions(od);
            if(od[0] != d1[0])
                out->Resize(d1);
            return true;
        }

        bool ExecuteImpl(const hops::MHO_NDArrayWrapper< T, 1 >* in1, const hops::MHO_NDArrayWrapper< T, 1 >* in2,
                         hops::MHO_NDArrayWrapper< T, 1 >* out) override
        {
            ++fExecCalls;
            if(!in1 || !in2 || !out)
                return false;
            auto i1 = in1->cbegin(), i1e = in1->cend();
            auto i2 = in2->cbegin();
            auto o = out->begin();
            while(i1 != i1e)
            {
                *o = (*i1) + (*i2);
                ++i1;
                ++i2;
                ++o;
            }
            return true;
        }
};

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    using Arr = hops::MHO_NDArrayWrapper< double, 1 >;
    using CArr = hops::MHO_NDArrayWrapper< std::complex< double >, 1 >;

    // --- Test 1: Happy path (double) ---
    {
        Arr a(4), b(4), c;
        a.SetArray(1.0);
        b.SetArray(2.5);
        AddArrays< double > op;
        op.SetArgs(&a, &b, &c);
        bool i = op.Initialize();
        bool e = op.Execute();
        REQUIRE(i == true);
        REQUIRE(e == true);
        REQUIRE(c.GetSize() == 4);
        for(std::size_t idx = 0; idx < 4; ++idx)
            REQUIRE(std::fabs(c(idx) - 3.5) < 1e-12);
        REQUIRE(op.fInitCalls == 1);
        REQUIRE(op.fExecCalls == 1);
    }

    // --- Test 2: Happy path (complex) ---
    {
        CArr a(2), b(2), c;
        a.SetArray(std::complex< double >(1, 2));
        b.SetArray(std::complex< double >(3, -1));
        AddArrays< std::complex< double > > op;
        op.SetArgs(&a, &b, &c);
        op.Initialize();
        op.Execute();
        for(std::size_t idx = 0; idx < 2; ++idx)
        {
            REQUIRE(std::fabs(c(idx).real() - 4.0) < 1e-12);
            REQUIRE(std::fabs(c(idx).imag() - 1.0) < 1e-12);
        }
    }

    // --- Test 3: Default-constructed tuple (no SetArgs) ---
    {
        AddArrays< double > op;
        bool i = op.Initialize();
        REQUIRE(i == false);
    }

    // --- Test 4: Size mismatch ---
    {
        Arr a(4), b(5), c;
        AddArrays< double > op;
        op.SetArgs(&a, &b, &c);
        bool i = op.Initialize();
        REQUIRE(i == false);
    }

    // --- Test 5: Output auto-resize ---
    {
        Arr a(3), b(3), c; // c default-constructed -> empty
        a.SetArray(1.0);
        b.SetArray(2.0);
        AddArrays< double > op;
        op.SetArgs(&a, &b, &c);
        op.Initialize();
        op.Execute();
        REQUIRE(c.GetSize() == 3);
        for(std::size_t idx = 0; idx < 3; ++idx)
            REQUIRE(std::fabs(c(idx) - 3.0) < 1e-12);
    }

    // --- Test 6: SetArgs swap ---
    {
        AddArrays< double > op;
        Arr a1(2), b1(2), c1;
        a1.SetArray(0.0);
        b1.SetArray(1.0);
        Arr a2(3), b2(3), c2;
        a2.SetArray(10.0);
        b2.SetArray(20.0);
        op.SetArgs(&a1, &b1, &c1);
        op.Initialize();
        op.Execute();
        op.SetArgs(&a2, &b2, &c2);
        op.Initialize();
        op.Execute();
        for(std::size_t idx = 0; idx < 2; ++idx)
            REQUIRE(std::fabs(c1(idx) - 1.0) < 1e-12);
        for(std::size_t idx = 0; idx < 3; ++idx)
            REQUIRE(std::fabs(c2(idx) - 30.0) < 1e-12);
    }

    // --- Test 7: Re-execution ---
    {
        Arr a(4), b(4), c;
        a.SetArray(1.0);
        b.SetArray(2.5);
        AddArrays< double > op;
        op.SetArgs(&a, &b, &c);
        op.Initialize();
        op.Execute();
        // Mutate a, then Execute again
        a.SetArray(7.0);
        op.Execute();
        for(std::size_t idx = 0; idx < 4; ++idx)
            REQUIRE(std::fabs(c(idx) - 9.5) < 1e-12);
        REQUIRE(op.fExecCalls == 2);
    }

    return 0;
}
