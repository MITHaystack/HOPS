#include "MHO_Message.hh"
#include "MHO_NDArrayWrapper.hh"
#include "MHO_TestAssertions.hh"
#include "MHO_TransformingOperator.hh"
#include <cmath>
#include <complex>
#include <cstddef>
#include <iostream>

using namespace hops;

class RealToComplex: public hops::MHO_TransformingOperator< hops::MHO_NDArrayWrapper< double, 1 >,
                                                            hops::MHO_NDArrayWrapper< std::complex< double >, 1 > >
{
    public:
        RealToComplex(): fInitCalls(0), fExecCalls(0) {}

        int fInitCalls;
        int fExecCalls;

    protected:
        bool InitializeImpl(const hops::MHO_NDArrayWrapper< double, 1 >* in,
                            hops::MHO_NDArrayWrapper< std::complex< double >, 1 >* out) override
        {
            ++fInitCalls;
            if(!in || !out)
                return false;
            std::size_t d[1];
            in->GetDimensions(d);
            std::size_t od[1];
            out->GetDimensions(od);
            if(od[0] != d[0])
                out->Resize(d);
            return true;
        }

        bool ExecuteImpl(const hops::MHO_NDArrayWrapper< double, 1 >* in,
                         hops::MHO_NDArrayWrapper< std::complex< double >, 1 >* out) override
        {
            ++fExecCalls;
            if(!in || !out)
                return false;
            auto ii = in->cbegin(), iie = in->cend();
            auto oo = out->begin();
            while(ii != iie)
            {
                *oo = std::complex< double >(*ii, 0.0);
                ++ii;
                ++oo;
            }
            return true;
        }
};

class IdentityD
    : public hops::MHO_TransformingOperator< hops::MHO_NDArrayWrapper< double, 1 >, hops::MHO_NDArrayWrapper< double, 1 > >
{
    public:
        IdentityD(): fInitCalls(0), fExecCalls(0) {}

        int fInitCalls;
        int fExecCalls;

    protected:
        bool InitializeImpl(const hops::MHO_NDArrayWrapper< double, 1 >* in,
                            hops::MHO_NDArrayWrapper< double, 1 >* out) override
        {
            ++fInitCalls;
            if(!in || !out)
                return false;
            std::size_t d[1];
            in->GetDimensions(d);
            std::size_t od[1];
            out->GetDimensions(od);
            if(od[0] != d[0])
                out->Resize(d);
            return true;
        }

        bool ExecuteImpl(const hops::MHO_NDArrayWrapper< double, 1 >* in, hops::MHO_NDArrayWrapper< double, 1 >* out) override
        {
            ++fExecCalls;
            if(!in || !out)
                return false;
            auto ii = in->cbegin(), iie = in->cend();
            auto oo = out->begin();
            while(ii != iie)
            {
                *oo = *ii;
                ++ii;
                ++oo;
            }
            return true;
        }
};

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    using Arr = hops::MHO_NDArrayWrapper< double, 1 >;
    using CArr = hops::MHO_NDArrayWrapper< std::complex< double >, 1 >;

    // --- Test 1: Real -> complex transform ---
    {
        Arr in(std::size_t(4));
        in.SetArray(2.5);
        CArr out;
        RealToComplex op;
        op.SetArgs(&in, &out);
        bool i = op.Initialize();
        bool e = op.Execute();
        REQUIRE(i == true);
        REQUIRE(e == true);
        REQUIRE(out.GetSize() == 4);
        for(std::size_t idx = 0; idx < 4; ++idx)
        {
            REQUIRE(std::fabs(out(idx).real() - 2.5) < 1e-12);
            REQUIRE(std::fabs(out(idx).imag() - 0.0) < 1e-12);
        }
        REQUIRE(op.fInitCalls == 1);
        REQUIRE(op.fExecCalls == 1);
    }

    // --- Test 2: Default-init tuple (no SetArgs) ---
    {
        RealToComplex op;
        bool i = op.Initialize();
        REQUIRE(i == false);
    }

    // --- Test 3: Identity (double, double) ---
    {
        Arr in(std::size_t(3));
        in.SetArray(7.0);
        Arr out;
        IdentityD op;
        op.SetArgs(&in, &out);
        op.Initialize();
        op.Execute();
        REQUIRE(out.GetSize() == 3);
        for(std::size_t idx = 0; idx < 3; ++idx)
            REQUIRE(std::fabs(out(idx) - 7.0) < 1e-12);
    }

    // --- Test 4: Resize-on-init ---
    {
        Arr in(std::size_t(5));
        in.SetArray(1.0);
        CArr out; // empty
        RealToComplex op;
        op.SetArgs(&in, &out);
        op.Initialize();
        op.Execute();
        REQUIRE(out.GetSize() == 5);
        for(std::size_t idx = 0; idx < 5; ++idx)
        {
            REQUIRE(std::fabs(out(idx).real() - 1.0) < 1e-12);
            REQUIRE(std::fabs(out(idx).imag() - 0.0) < 1e-12);
        }
    }

    // --- Test 5: SetArgs swap ---
    {
        RealToComplex op;
        Arr in1(2);
        in1.SetArray(1.0);
        CArr out1;
        Arr in2(3);
        in2.SetArray(4.0);
        CArr out2;
        op.SetArgs(&in1, &out1);
        op.Initialize();
        op.Execute();
        op.SetArgs(&in2, &out2);
        op.Initialize();
        op.Execute();
        for(std::size_t idx = 0; idx < 2; ++idx)
            REQUIRE(std::fabs(out1(idx).real() - 1.0) < 1e-12);
        for(std::size_t idx = 0; idx < 3; ++idx)
            REQUIRE(std::fabs(out2(idx).real() - 4.0) < 1e-12);
    }

    // --- Test 6: Re-execution preserves last-set args ---
    {
        Arr in(std::size_t(4));
        in.SetArray(2.5);
        CArr out;
        RealToComplex op;
        op.SetArgs(&in, &out);
        op.Initialize();
        op.Execute();
        // Mutate in, then Execute again
        in.SetArray(8.5);
        op.Execute();
        for(std::size_t idx = 0; idx < 4; ++idx)
            REQUIRE(std::fabs(out(idx).real() - 8.5) < 1e-12);
        REQUIRE(op.fExecCalls == 2);
    }

    // --- Test 7: Polymorphic destruction through base pointer ---
    {
        MHO_Operator* p = new RealToComplex();
        delete p;
        // Success = no crash
    }

    return 0;
}
