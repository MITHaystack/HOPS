#include <iostream>

#include "MHO_FileKey.hh"
#include "MHO_MD5HashGenerator.hh"
#include "MHO_UUIDGenerator.hh"

using namespace hops;

int main(int /*argc*/, char** /*argv*/)
{
    MHO_UUIDGenerator* gen = new MHO_UUIDGenerator();

    std::cout << "1 uuid = " << gen->GenerateUUIDAsString() << std::endl;
    std::cout << "2 uuid = " << gen->GenerateUUIDAsString() << std::endl;
    std::cout << "3 uuid = " << gen->GenerateUUIDAsString() << std::endl;
    std::cout << "4 uuid = " << gen->GenerateUUIDAsString() << std::endl;
    std::cout << "5 uuid = " << gen->GenerateUUIDAsString() << std::endl;
    std::cout << "6 uuid = " << gen->GenerateUUIDAsString() << std::endl;
    std::cout << "7 uuid = " << gen->GenerateUUIDAsString() << std::endl;
    std::cout << "8 uuid = " << gen->GenerateUUIDAsString() << std::endl;
    std::cout << "9 uuid = " << gen->GenerateUUIDAsString() << std::endl;
    //
    // MHO_FileKey aKey;

    // aKey.fUUID = gen->GenerateUUID();
    // aKey.fClassIndex = 1;
    // aKey.fClassVersion = 1;
    // aKey.fSize = 1;
    //std::cout << "file obj key "<< std::endl;
    //std::cout << aKey <<std::endl;

    MHO_MD5HashGenerator* md5gen = new MHO_MD5HashGenerator();
    md5gen->Initialize();

    int data1 = 32349482;
    int data2 = 238;
    //double x = 23e52;
    //std::complex<double> x(1.1, 2.2);

    std::string tmp("this is a string lkjdlk");
    *md5gen << data1;
    *md5gen << data2;
    //*md5gen << x;
    *md5gen << tmp;
    md5gen->Finalize();

    std::cout << std::hex << md5gen->GetDigest() << std::endl;

    delete gen;
    delete md5gen;

    return 0;
}
