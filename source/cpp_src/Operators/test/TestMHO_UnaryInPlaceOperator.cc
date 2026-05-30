#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TestAssertions.hh"
#include "MHO_UnaryInPlaceOperator.hh"
#include <cmath>
#include <complex>
#include <cstddef>
#include <iostream>

using namespace hops;

template< typename T > class ScaleInPlace: public hops::MHO_UnaryInPlaceOperator< hops::MHO_NDArrayWrapper< T, 1 > >
{
    public:
        ScaleInPlace(): fFactor(T(1)), fInitCalls(0), fExecCalls(0) {}

        T fFactor;
        int fInitCalls;
        int fExecCalls;

    protected:
        bool InitializeInPlace(hops::MHO_NDArrayWrapper< T, 1 >* in) override
        {
            ++fInitCalls;
            return in != nullptr;
        }

        bool ExecuteInPlace(hops::MHO_NDArrayWrapper< T, 1 >* in) override
        {
            ++fExecCalls;
            if(!in)
                return false;
            for(auto it = in->begin(); it != in->end(); ++it)
                *it = (*it) * fFactor;
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
        Arr a(std::size_t(4));
        a.SetArray(2.0);
        ScaleInPlace< double > op;
        op.fFactor = 3.0;
        op.SetArgs(&a);
        bool i = op.Initialize();
        bool e = op.Execute();
        REQUIRE(i == true);
        REQUIRE(e == true);
        for(std::size_t idx = 0; idx < 4; ++idx)
            REQUIRE(std::fabs(a(idx) - 6.0) < 1e-12);
        REQUIRE(op.fInitCalls == 1);
        REQUIRE(op.fExecCalls == 1);
    }

    // --- Test 2: Happy path (complex) ---
    {
        CArr a(std::size_t(3));
        a.SetArray(std::complex< double >(1.0, 2.0));
        ScaleInPlace< std::complex< double > > op;
        op.fFactor = std::complex< double >(0.0, 1.0); // multiply by i
        op.SetArgs(&a);
        op.Initialize();
        op.Execute();
        for(std::size_t idx = 0; idx < 3; ++idx)
        {
            REQUIRE(std::fabs(a(idx).real() - (-2.0)) < 1e-12);
            REQUIRE(std::fabs(a(idx).imag() - 1.0) < 1e-12);
        }
    }

    // --- Test 3: Null pointer on Initialize ---
    {
        ScaleInPlace< double > op;
        op.SetArgs(static_cast< Arr* >(nullptr));
        bool i = op.Initialize();
        REQUIRE(i == false);
    }

    // --- Test 4: Default state without SetArgs ---
    {
        ScaleInPlace< double > op;
        bool i = op.Initialize();
        REQUIRE(i == false);
    }

    // --- Test 5: Re-initialization / repeated Execute ---
    {
        ScaleInPlace< double > op;
        op.fFactor = 2.0;
        Arr a(std::size_t(2));
        a.SetArray(1.0);
        op.SetArgs(&a);
        op.Initialize();
        op.Execute();
        op.Execute();
        op.Execute();
        for(std::size_t idx = 0; idx < 2; ++idx)
            REQUIRE(std::fabs(a(idx) - 8.0) < 1e-12);
        REQUIRE(op.fInitCalls == 1);
        REQUIRE(op.fExecCalls == 3);
    }

    // --- Test 6: SetArgs swap to a different array ---
    {
        ScaleInPlace< double > op;
        op.fFactor = 2.0;
        Arr a(std::size_t(2));
        a.SetArray(1.0);
        Arr b(std::size_t(3));
        b.SetArray(5.0);
        op.SetArgs(&a);
        op.Initialize();
        op.Execute();
        op.SetArgs(&b);
        op.Initialize();
        op.Execute();
        for(std::size_t idx = 0; idx < 2; ++idx)
            REQUIRE(std::fabs(a(idx) - 2.0) < 1e-12);
        for(std::size_t idx = 0; idx < 3; ++idx)
            REQUIRE(std::fabs(b(idx) - 10.0) < 1e-12);
    }

    // --- Test 7: Polymorphic destruction ---
    {
        MHO_Operator* p = new ScaleInPlace< double >();
        delete p;
        // Success = no crash
    }

    return 0;
}
