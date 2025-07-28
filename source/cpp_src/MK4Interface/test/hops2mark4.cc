#include "MHO_MK4ScanConverterReversed.hh"
#include "MHO_Message.hh"
#include "MHO_DirectoryInterface.hh"

#include <iostream>
#include <string>

#ifndef HOPS3_USE_CXX
extern "C"
{
#endif
    #include "msg.h"
#ifndef HOPS3_USE_CXX
}
#endif


using namespace hops;


void print_usage()
{
    std::cout << "Usage: hops2mark4 <input_dir> [output_dir] \n";
    std::cout << "  input_dir   - Directory containing HOPS4 files (.cor, .sta, .root.json)\n";
    std::cout << "  output_dir  - Output directory for Mark4 files (optional, defaults to input_dir)\n";
}

int main(int argc, char** argv)
{
    //set_msglev(-2);


    std::string in_dir = "";
    std::string out_dir = "";

    if(argc < 2 || argc > 4)
    {
        print_usage();
        return 1;
    }

    in_dir = argv[1];
    if(argc >= 3){out_dir = argv[2];}

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eDebug);

    if(out_dir == "")
    {
        out_dir = in_dir;
        msg_status("main", "no output directory passed, writing output files to input directory" << eom);
    }

    // Check if input directory exists and contains HOPS4 files
    MHO_DirectoryInterface dirInterface;
    std::string input_dir = dirInterface.GetDirectoryFullPath(in_dir);
    std::string output_dir = dirInterface.GetDirectoryFullPath(out_dir);

    if(!dirInterface.DoesDirectoryExist(input_dir))
    {
        msg_fatal("main", "input directory does not exist: " << input_dir << eom);
        return 1;
    }

    MHO_MK4ScanConverterReversed scan_converter;
    scan_converter.ProcessScan(input_dir, output_dir);

    return 0;
}