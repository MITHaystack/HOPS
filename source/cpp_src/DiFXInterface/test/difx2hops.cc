#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <utility>
#include <map>
#include <getopt.h>

//needed for listing/navigating files/directories on *nix
#include <dirent.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>

#include "difxio/difx_input.h"
#include "difxio/parsevis.h"

#include "MHO_Message.hh"
#include "MHO_Tokenizer.hh"

#include "MHO_Visibilities.hh"
#include "MHO_ChannelizedVisibilities.hh"
#include "MHO_VisibilityChannelizer.hh"
#include "MHO_WeightChannelizer.hh"
#include "MHO_StationCoordinates.hh"

#include "MHO_BinaryFileStreamer.hh"
#include "MHO_BinaryFileInterface.hh"
#include "MHO_ClassIdentityMap.hh"

#include "MHO_DirectoryInterface.hh"

using namespace hops;




int main(int argc, char** argv)
{
    std::string usage = "difx2hops -i <input_directory> -o <output_directory>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    std::string input_dir;
    std::string output_dir, odir;

    static struct option longOptions[] = {{"help", no_argument, 0, 'h'},
                                          {"input_directory", required_argument, 0, 'i'},
                                          {"output_directory", required_argument, 0, 'o'}};

    static const char* optString = "hi:o:";

    while(true)
    {
        char optId = getopt_long(argc, argv, optString, longOptions, NULL);
        if (optId == -1)
            break;
        switch(optId)
        {
            case ('h'):  // help
                std::cout << usage << std::endl;
                return 0;
            case ('i'):
                input_dir = std::string(optarg);
                break;
            case ('o'):
                odir = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }


    //directory interface
    MHO_DirectoryInterface dirInterface;
    output_dir = dirInterface.GetDirectoryFullPath(odir);
    input_dir = dirInterface.GetDirectoryFullPath(input_dir);

    msg_info("main", "input directory: " << input_dir << eom);
    msg_info("main", "output directory: " << output_dir << eom);


    //get list of all the files (and directories) in directory
    std::vector< std::string > allFiles;
    std::vector< std::string > allDirs;

    dirInterface.SetCurrentDirectory(input_dir);
    dirInterface.ReadCurrentDirectory();
    dirInterface.GetFileList(allFiles);
    dirInterface.GetSubDirectoryList(allDirs);

    //debug
    for(auto it=allFiles.begin(); it != allFiles.end(); it++)
    {
        std::cout<<"file: "<<*it<<std::endl;
    }

    //debug
    for(auto it=allDirs.begin(); it != allDirs.end(); it++)
    {
        std::cout<<"dir: "<<*it<<std::endl;
    }


    //grab all of the .difx directories 
    std::vector< std::string > difxDirs;
    dirInterface.GetSubDirectoriesMatchingExtention(difxDirs, "difx");
    for(auto it=difxDirs.begin(); it != difxDirs.end(); it++)
    {
        std::cout<<"difx sub-dir: "<<*it<<std::endl;
    }

    MHO_DirectoryInterface subdirInterface;
    //loop over the difx sub-directories 
    for(auto it=difxDirs.begin(); it != difxDirs.end(); it++)
    {
        //conver the difx data
    }




    // //create the output directory if needed
    // if( !dirInterface.DoesDirectoryExist(output_dir) )
    // {
    //     dirInterface.CreateDirectory(output_dir);
    // }



    DifxInput* din;

    return 0;
}
