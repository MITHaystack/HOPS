#include <getopt.h>

#include "MHO_MK4ScanConverter.hh"

using namespace hops;

int main(int argc, char** argv)
{
    std::string usage = "mark42hops -i <input_directory> -o <output_directory>";

    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetMessageLevel(eStatus);

    std::string in_dir;
    std::string out_dir;

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
                in_dir = std::string(optarg);
                break;
            case ('o'):
                out_dir = std::string(optarg);
                break;
            default:
                std::cout << usage << std::endl;
                return 1;
        }
    }

    if(in_dir == "" || out_dir == "")
    {
        msg_fatal("main", "must specify input and output directories" << eom );
        return 1;
    }
    
    int dir_type = MHO_MK4ScanConverter::DetermineDirectoryType(in_dir);

    if(dir_type == MK4_UNKNOWNDIR)
    {
        msg_fatal("main", "could not determine the directory type of: "<<in_dir << 
            " it is not a mk4 experiment or scan directory." << eom);
        return 1;
    }

    if(dir_type == MK4_SCANDIR)
    {
        msg_status("main", "will process a single scan from directory: "<< in_dir << eom);
        MHO_MK4ScanConverter::ProcessScan(in_dir, out_dir);
        return 0;
    }
    
    if(dir_type == MK4_EXPDIR)
    {
        //directory interface
        MHO_DirectoryInterface dirInterface;
        std::string exp_dir = dirInterface.GetDirectoryFullPath(in_dir);
        std::string output_dir = dirInterface.GetDirectoryFullPath(out_dir);
        //we need to get a list of all the scan directories and process them one-by-one
        std::vector< std::string > allDirs;
        dirInterface.SetCurrentDirectory(exp_dir);
        dirInterface.ReadCurrentDirectory();
        dirInterface.GetSubDirectoryList(allDirs);
        msg_status("main", "will process "<< allDirs.size() <<" scans from experiment directory: "<< exp_dir << eom);

        //make sure output parent directory exists
        if( !dirInterface.DoesDirectoryExist(output_dir) )
        {
            dirInterface.CreateDirectory(output_dir);
        }
        
        //TODO we may want to sanitize the sub directory list (to make sure we only have scan directories for sure)

        std::vector< std::string > scanOutputDirs;
        for(std::size_t i=0; i<allDirs.size(); i++)
        {
            std::string scan_dir = allDirs[i];
            std::string scan_name = MHO_DirectoryInterface::GetBasename(scan_dir);
            //last char cannot be '/', strip just in case
            if( scan_name.back() == '/'){scan_name.erase(scan_name.size()-1);}
            std::string scan_output_dir = output_dir + "/" + scan_name ;
            scanOutputDirs.push_back(scan_output_dir);
        }

        //TODO PARALLELIZE THIS with std::threads
        for(std::size_t i=0; i<allDirs.size(); i++)
        {
            MHO_MK4ScanConverter::ProcessScan(allDirs[i], scanOutputDirs[i]);
        }

        return 0;
    }
    
    return 0;
}
