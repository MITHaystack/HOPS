//option parsing and help text library
#include "CLI11.hpp"

#include "MHO_MK4ScanConverter.hh"

using namespace hops;

int main(int argc, char** argv)
{
    std::string in_dir = "";
    std::string out_dir = "";
    int message_level = 0;

    CLI::App app{"mark42hops"};

    app.add_option("input_dir,-i,--input-dir", in_dir, "name of the input directory")->required();
    app.add_option("output_dir,-o,--output-dir", out_dir, "name of the output directory, if not passed the output files will be written out to the input directory");
    app.add_option("-m,--message-level", message_level, "message level to be used, range: -2 (debug) to 5 (silent)");

    CLI11_PARSE(app, argc, argv);

    //clamp message level
    if(message_level > 5){message_level = 5;}
    if(message_level < -2){message_level = -2;}
    MHO_Message::GetInstance().AcceptAllKeys();
    MHO_Message::GetInstance().SetLegacyMessageLevel(message_level);

    if(in_dir == "" )
    {
        msg_fatal("main", "must specify input directory" << eom );
        return 1;
    }

    if(out_dir == "")
    {
        out_dir = in_dir;
        msg_status("main", "no output directory passed, writing output files to input directory" << eom );
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
