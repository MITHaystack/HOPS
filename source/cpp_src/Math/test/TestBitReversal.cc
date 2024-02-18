#include "MHO_Message.hh"
#include "MHO_BitReversalPermutation.hh"


using namespace hops;

typedef double FP_Type;

int main(int /*argc*/, char** /*argv*/)
{
    unsigned int N = 1024;
    unsigned int log2N = MHO_BitReversalPermutation::LogBaseTwo(N);
    unsigned int* index_arr = new unsigned int[N];
    MHO_BitReversalPermutation::ComputeBitReversedIndicesBaseTwo(N,index_arr);

    std::cout<<"[index, Buneman, alt-method ]: "<<std::endl;
    int ok = 0;
    for(unsigned int i=0; i<N; i++)
    {
        unsigned int bit_reversed = MHO_BitReversalPermutation::ReverseIndexBits(log2N, i);
        std::cout<<i<<", "<<index_arr[i]<<", "<<bit_reversed<<std::endl;
        ok += index_arr[i] - bit_reversed;
    }

    delete[] index_arr;

    return ok;
}
