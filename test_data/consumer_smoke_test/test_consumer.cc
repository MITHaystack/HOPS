// Smoke test for the installed HOPS development files (hops4.pc /
// HopsConfig.cmake). Instantiates one symbol from each of MHO_Containers
// and MHO_Utilities so the test fails if a header is missing or if the
// shared libraries can't be linked/loaded.


#include "MHO_ContainerStore.hh"
#include "MHO_Tokenizer.hh"
#include "MHO_Message.hh"

#include <iostream>

int main()
{
    hops::MHO_ContainerStore store;
    (void)store;

    hops::MHO_Tokenizer tok;
    std::string input = "a b c";
    std::vector<std::string> out;
    tok.SetString(&input);
    tok.GetTokens(&out);

    std::cout << "ran tokenizer (got " << out.size() << " tokens)" << std::endl;

    return (out.size() == 3) ? 0 : 1;
}
