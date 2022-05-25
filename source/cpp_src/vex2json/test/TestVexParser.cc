#include <string>
#include <iostream>

#include "MHO_VexParser.hh"

using namespace hops;

int main(int argc, char** argv)
{
    std::string vexfile(argv[1]);

    MHO_VexParser vparser;
    vparser.SetVexFile(vexfile);

    vparser.ReadFile();
    vparser.RemoveComments();

    std::list< std::string >* vlines = vparser.GetLines();

    int i=0;
    for(auto it = vlines->begin(); it != vlines->end(); it++)
    {
        if((*it)[0] == '$'){std::cout<<*it<<std::endl;}
    }

    return 0;
}
