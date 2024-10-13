#include "MHO_FastFourierTransformUtilities.hh"
#include "MHO_Message.hh"
#include <iomanip>

using namespace hops;

typedef double FP_Type;

std::complex< FP_Type > compute_twiddle_factor(unsigned int log2N, unsigned int index, std::complex< FP_Type >* basis)
{
    unsigned int bit = 1;
    std::complex< FP_Type > val(1.0, 0.0);
    std::complex< FP_Type > unit(1.0, 0.0);
    bool ok, nok;
    double dok, dnok;
    for(unsigned int i = 0; i < log2N; i++)
    {
        ok = (index & bit);
        nok = !ok;
        dok = ok;
        dnok = nok;
        // if( (index & bit) ){val *=  basis[i];}
        val *= (dok * basis[i] + dnok * unit);
        bit *= 2;
    }
    return val;
}

int main(int /*argc*/, char** /*argv*/)
{
    unsigned int N = 1048576;
    unsigned log2N = MHO_BitReversalPermutation::LogBaseTwo(N);
    std::complex< FP_Type >* twiddle = new std::complex< FP_Type >[N];
    std::complex< FP_Type >* twid_basis = new std::complex< FP_Type >[log2N];

    MHO_FastFourierTransformUtilities< FP_Type >::ComputeTwiddleFactors(N, twiddle);
    MHO_FastFourierTransformUtilities< FP_Type >::ComputeTwiddleFactorBasis(log2N, twid_basis);

    int ok = 0;
    FP_Type tol = 1.1e-15;
    for(unsigned int x = 0; x < N; x++)
    {
        std::complex< FP_Type > val = compute_twiddle_factor(log2N, x, twid_basis);
        std::cout << std::setprecision(16);
        FP_Type delta = std::abs(val - twiddle[x]);
        if(delta > tol)
        {
            ok += 1;
            std::cout << "index, delta = " << x << "," << delta << std::endl;
        }
    }
    return ok;
}
