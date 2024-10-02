#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>

#include "MHO_DirectoryInterface.hh"
#include "MHO_Message.hh"

using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "TestDirectoryInterface -d <directory>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string dir;

    static struct option longOptions[] = {
        {"help",      no_argument,       0, 'h'},
        {"directory", required_argument, 0, 'd'}
    };

    static const char* optString = "hd:";

    while(true)
    {
        char optId = getopt_long(argc, argv, optString, longOptions, NULL);
        if(optId == -1)
            break;
        switch(optId)
        {
            case('h'): // help
                std::cout << usage << std::endl;
                return 0;
            case('d'):
                dir = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    std::vector< std::string > allFiles;
    std::vector< std::string > allDirectories;

    MHO_DirectoryInterface dirInterface;
    dirInterface.SetCurrentDirectory(dir);
    dirInterface.ReadCurrentDirectory();
    dirInterface.GetFileList(allFiles);
    dirInterface.GetSubDirectoryList(allDirectories);

    for(auto it = allFiles.begin(); it != allFiles.end(); it++)
    {
        std::cout << "file: " << *it << std::endl;
    }

    for(auto it = allDirectories.begin(); it != allDirectories.end(); it++)
    {
        std::cout << "dir: " << *it << std::endl;
    }

    return 0;
}
