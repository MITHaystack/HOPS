#include <iostream>

#include "MHO_ClassIdentity.hh"
#include "MHO_FileKey.hh"
#include "MHO_MD5HashGenerator.hh"
#include "MHO_TemplateTypenameDeduction.hh"
#include "MHO_UUIDGenerator.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
#if defined(__clang__)
    std::cout << "The compiler is: clang." << std::endl;
#elif defined(__GNUC__)
    std::cout << "The compiler is: GCC." << std::endl;
#endif

    std::cout << " MHO_RawCompilerName< std::string >() = " << MHO_RawCompilerName< std::string >() << std::endl;
    std::cout << " MHO_TupleElementNameWithoutSpaces< std::string >() = " << MHO_TupleElementNameWithoutSpaces< std::string >()
              << std::endl;
    std::cout << " MHO_ClassName<std::string>()  = " << MHO_ClassName< std::string >() << std::endl;
    std::cout << " MHO_ClassName< std::tuple< std::string > >()  = " << MHO_ClassName< std::tuple< std::string > >()
              << std::endl;

    return 0;
}
