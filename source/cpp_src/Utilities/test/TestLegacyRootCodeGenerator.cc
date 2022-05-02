#include <iostream>

#include "MHO_LegacyRootCodeGenerator.hh"

using namespace hops;


int main(int argc, char** argv)
{

    MHO_LegacyRootCodeGenerator rcode_gen;

    std::string code = rcode_gen.GetCode();

    std::cout<<" first code (in isolation) = "<< code <<std::endl;

    std::size_t n = 10;
    std::vector<std::string> codes = rcode_gen.GetCodes(n);

    for(std::size_t i=0; i<n; i++)
    {
        std::cout<<" code sequence @ "<<i<<" = "<<codes[i]<<std::endl;
    }

    return 0;
}
