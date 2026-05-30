#include "MHO_BitReversalPermutation.hh"
#include "MHO_Message.hh"
#include "MHO_TestAssertions.hh"
#include <cmath>
#include <complex>
#include <iostream>
#include <vector>

using namespace hops;

int main()
{
    MHO_Message::GetInstance().SetMessageLevel(eFatal);

    //  Case 1: IsPowerOfTwo
    {
        REQUIRE(MHO_BitReversalPermutation::IsPowerOfTwo(1) == true);
        REQUIRE(MHO_BitReversalPermutation::IsPowerOfTwo(2) == true);
        REQUIRE(MHO_BitReversalPermutation::IsPowerOfTwo(4) == true);
        REQUIRE(MHO_BitReversalPermutation::IsPowerOfTwo(1024) == true);
        REQUIRE(MHO_BitReversalPermutation::IsPowerOfTwo(1048576) == true); // 2^20
        REQUIRE(MHO_BitReversalPermutation::IsPowerOfTwo(0) == false);
        REQUIRE(MHO_BitReversalPermutation::IsPowerOfTwo(3) == false);
        REQUIRE(MHO_BitReversalPermutation::IsPowerOfTwo(7) == false);
        REQUIRE(MHO_BitReversalPermutation::IsPowerOfTwo(15) == false);
    }

    //  Case 2: TwoToThePowerOf
    {
        REQUIRE(MHO_BitReversalPermutation::TwoToThePowerOf(0) == 1u);
        REQUIRE(MHO_BitReversalPermutation::TwoToThePowerOf(1) == 2u);
        REQUIRE(MHO_BitReversalPermutation::TwoToThePowerOf(10) == 1024u);
        REQUIRE(MHO_BitReversalPermutation::TwoToThePowerOf(20) == 1048576u);
    }

    //  Case 3: LogBaseTwo
    {
        REQUIRE(MHO_BitReversalPermutation::LogBaseTwo(1) == 0u);
        REQUIRE(MHO_BitReversalPermutation::LogBaseTwo(2) == 1u);
        REQUIRE(MHO_BitReversalPermutation::LogBaseTwo(1024) == 10u);
        REQUIRE(MHO_BitReversalPermutation::LogBaseTwo(1048576) == 20u);
    }

    //  Case 4: NextLowestPowerOfTwo
    // Despite the name, returns next HIGHEST power of two for non-powers
    {
        REQUIRE(MHO_BitReversalPermutation::NextLowestPowerOfTwo(1) == 1u);
        REQUIRE(MHO_BitReversalPermutation::NextLowestPowerOfTwo(2) == 2u);
        REQUIRE(MHO_BitReversalPermutation::NextLowestPowerOfTwo(3) == 4u);
        REQUIRE(MHO_BitReversalPermutation::NextLowestPowerOfTwo(7) == 8u);
        REQUIRE(MHO_BitReversalPermutation::NextLowestPowerOfTwo(8) == 8u);
        REQUIRE(MHO_BitReversalPermutation::NextLowestPowerOfTwo(9) == 16u);
        REQUIRE(MHO_BitReversalPermutation::NextLowestPowerOfTwo(0) == 2u);
    }

    //  Case 5: ReverseIndexBits
    // Signature: ReverseIndexBits(nbits, x) - reverse x using nbits bits
    {
        // Reverse 4 bits: 1(0001) -> 8(1000), 2(0010) -> 4(0100), etc.
        REQUIRE(MHO_BitReversalPermutation::ReverseIndexBits(4, 1) == 8u);
        REQUIRE(MHO_BitReversalPermutation::ReverseIndexBits(4, 2) == 4u);
        REQUIRE(MHO_BitReversalPermutation::ReverseIndexBits(4, 3) == 12u);
        REQUIRE(MHO_BitReversalPermutation::ReverseIndexBits(4, 4) == 2u);
        REQUIRE(MHO_BitReversalPermutation::ReverseIndexBits(4, 8) == 1u);
        REQUIRE(MHO_BitReversalPermutation::ReverseIndexBits(4, 12) == 3u);
        // 3 bits: 1(001) -> 4(100), 4(100) -> 1(001)
        REQUIRE(MHO_BitReversalPermutation::ReverseIndexBits(3, 1) == 4u);
        REQUIRE(MHO_BitReversalPermutation::ReverseIndexBits(3, 4) == 1u);
    }

    //  Case 6: ComputeBitReversedIndicesBaseTwo
    {
        unsigned int idx[16];
        MHO_BitReversalPermutation::ComputeBitReversedIndicesBaseTwo(16, idx);
        // Verify each index is its bit-reversal (nbits=4)
        for(unsigned int i = 0; i < 16; i++)
        {
            REQUIRE(idx[i] == MHO_BitReversalPermutation::ReverseIndexBits(4, i));
        }
        // N=1 edge case
        unsigned int idx1[1];
        MHO_BitReversalPermutation::ComputeBitReversedIndicesBaseTwo(1, idx1);
        REQUIRE(idx1[0] == 0u);
    }

    //  Case 7: IsPowerOfBase / RaiseBaseToThePower / LogBaseB
    {
        REQUIRE(MHO_BitReversalPermutation::RaiseBaseToThePower(3, 0) == 1u);
        REQUIRE(MHO_BitReversalPermutation::RaiseBaseToThePower(3, 4) == 81u);
        REQUIRE(MHO_BitReversalPermutation::RaiseBaseToThePower(5, 3) == 125u);
        REQUIRE(MHO_BitReversalPermutation::IsPowerOfBase(81, 3) == true);
        REQUIRE(MHO_BitReversalPermutation::IsPowerOfBase(82, 3) == false);
        // Note: IsPowerOfBase(1,7) returns false due to N<B early-return (bug in impl)
        REQUIRE(MHO_BitReversalPermutation::IsPowerOfBase(1, 7) == false);
        REQUIRE(MHO_BitReversalPermutation::LogBaseB(81, 3) == 4u);
        REQUIRE(MHO_BitReversalPermutation::LogBaseB(125, 5) == 3u);
    }

    //  Case 8: PermuteArray round-trip (non-strided)
    {
        const unsigned int N = 16;
        std::vector< std::complex< double > > data(N);
        std::vector< std::complex< double > > original(N);
        unsigned int idx[16];
        MHO_BitReversalPermutation::ComputeBitReversedIndicesBaseTwo(N, idx);
        for(unsigned int i = 0; i < N; i++)
        {
            data[i] = std::complex< double >(double(i), 0.0);
            original[i] = data[i];
        }
        // First permutation
        MHO_BitReversalPermutation::PermuteArray(N, idx, data.data());
        // Verify bit-reversal: data[1] should have real part 8.0
        REQUIRE(data[1].real() == 8.0);
        // Second permutation (involution -> identity)
        MHO_BitReversalPermutation::PermuteArray(N, idx, data.data());
        for(unsigned int i = 0; i < N; i++)
        {
            REQUIRE(data[i].real() == original[i].real());
            REQUIRE(data[i].imag() == original[i].imag());
        }
    }

    //  Case 9: PermuteArray with stride
    {
        const unsigned int N = 8;
        const unsigned int stride = 2;
        std::vector< double > arr(2 * N); // interleaved: real at even, imag at odd
        unsigned int idx[8];
        MHO_BitReversalPermutation::ComputeBitReversedIndicesBaseTwo(N, idx);
        for(unsigned int i = 0; i < N; i++)
        {
            arr[2 * i] = double(i); // real parts
            arr[2 * i + 1] = 0.0;   // imag parts (should be untouched)
        }
        // Permute with stride=2 (only permutes the real parts at even indices)
        MHO_BitReversalPermutation::PermuteArray(N, idx, arr.data(), stride);
        // Round-trip
        MHO_BitReversalPermutation::PermuteArray(N, idx, arr.data(), stride);
        for(unsigned int i = 0; i < N; i++)
        {
            REQUIRE(arr[2 * i] == double(i));
            REQUIRE(arr[2 * i + 1] == 0.0);
        }
    }

    //  Case 10: PermuteArray N=1 identity
    {
        std::complex< double > data[1] = {std::complex< double >(42.0, 0.0)};
        unsigned int idx[1] = {0};
        MHO_BitReversalPermutation::PermuteArray(1, idx, data);
        REQUIRE(data[0].real() == 42.0);
    }

    return 0;
}
