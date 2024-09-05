#include <string>
#include <iostream>
#include <fstream>

#include "MHO_Message.hh"

#include "MHO_AFileDefinitions.hh"
#include "MHO_AFileInfoExtractor.hh"

using namespace hops;

int main(int argc, char** argv)
{
    if(argc != 2){std::cout<<"filename argument missing"<<std::endl; std::exit(1);}

    std::string filename(argv[1]);

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    MHO_AFileInfoExtractor ext;

    mho_json fsum;
    bool ok = ext.SummarizeFringeFile(filename, fsum);

    if(ok){std::cout<<fsum.dump(2)<<std::endl;}
    else{std::cout<<"could not extract file: "<< filename<<std::endl;}

    return 0;
}
