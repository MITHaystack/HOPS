#include "MHO_Mk4InputConverter.hh"

//does the mark4 -> hops conversion work
#include "MHO_MK4ScanConverter.hh"

//POSIX includes for mkdtemp and nftw (used by convert_mk4_input)
//if we were using C++17, we could alternatively use std::filesystem
#include <ftw.h>
#include <unistd.h>

namespace hops
{

// File-scope helper for nftw recursive delete (cannot be a lambda due to C linkage requirement)
static int remove_entry(const char* path, const struct stat* /*sb*/, int /*typeflag*/, struct FTW* /*ftwbuf*/)
{
    return remove(path);
}

std::string MHO_Mk4InputConverter::convert_mk4_input(MHO_ParameterStore* paramStore)
{
    //the input directory
    std::string input_dir = paramStore->GetAs< std::string >("/cmdline/input_directory");

    int dir_type = MHO_MK4ScanConverter::DetermineDirectoryType(input_dir);

    // Read the baseline filter; "??" means all baselines
    std::string baseline = paramStore->GetAs< std::string >("/cmdline/baseline");

    // Create a unique temp directory,
    // we prefer RAM-backed /dev/shm (tmpfs) to avoid disk I/O, but only for single-scans!
    // falling back to /tmp if /dev/shm is not available, or if we were passed a full experiment
    std::string temp_root;
    {
        char shm_buf[] = "/dev/shm/hops_mk4_XXXXXX";
        if(mkdtemp(shm_buf) != nullptr && dir_type == MK4_SCANDIR )
        {
            temp_root = shm_buf;
        }
        else
        {
            char tmp_buf[] = "/tmp/hops_mk4_XXXXXX";
            if(mkdtemp(tmp_buf) == nullptr)
            {
                msg_fatal("fringe", "convert_mk4_input: could not create a temporary directory" << eom);
                return "";
            }
            temp_root = tmp_buf;
        }
    }

    if(dir_type == MK4_SCANDIR)
    {
        std::string scan_name = MHO_DirectoryInterface::GetTrailingDirectory(input_dir);
        std::string temp_scan = temp_root + "/" + scan_name;
        msg_status("fringe", "converting mark4 scan directory: " << input_dir << " -> " << temp_scan
                                                                  << " (baseline filter: " << baseline << ")" << eom);
        MHO_MK4ScanConverter::ProcessScan(input_dir, temp_scan, baseline);
        msg_info("fringe", "redirecting input directory to : "<<  temp_scan + "/" << eom);
        paramStore->Set("/cmdline/input_directory", temp_scan + "/");
    }
    else if(dir_type == MK4_EXPDIR)
    {
        msg_warn("fringe", "on-the-fly mark4-to-hops conversion of mark4 experiment directories is not recommended, use mark42hops first" << eom);
        msg_status("fringe", "converting mark4 experiment directory: " << input_dir
                                                                       << " (baseline filter: " << baseline << ")" << eom);
        MHO_DirectoryInterface dirInterface;
        dirInterface.SetCurrentDirectory(input_dir);
        dirInterface.ReadCurrentDirectory();
        std::vector< std::string > subDirs;
        dirInterface.GetSubDirectoryList(subDirs);

        for(std::size_t i = 0; i < subDirs.size(); i++)
        {
            //strip out any sub-directories that are not mark4 scan directories
            dirInterface.SetCurrentDirectory(subDirs[i]);
            std::vector< std::string > subDirFiles;
            dirInterface.ReadCurrentDirectory();
            dirInterface.GetFileList(subDirFiles);
            std::string root_file_name = "";
            dirInterface.GetRootFile(subDirFiles, root_file_name);
            if(root_file_name != "")
            {
                std::string scan_name = MHO_DirectoryInterface::GetBasename(subDirs[i]);
                // Strip trailing '/' if present
                if(!scan_name.empty() && scan_name.back() == '/')
                {
                    scan_name.erase(scan_name.size() - 1);
                }
                std::string temp_scan = temp_root + "/" + scan_name;
                msg_status("fringe", "converting mark4 scan: " << subDirs[i] << " -> " << temp_scan << eom);
                MHO_MK4ScanConverter::ProcessScan(subDirs[i], temp_scan, baseline);
            }
        }
        paramStore->Set("/cmdline/input_directory", temp_root + "/");
        msg_info("fringe", "redirecting input directory to : "<<  temp_root + "/" << eom);
    }
    else
    {
        msg_fatal("fringe",
                  "convert_mk4_input: input directory is not a recognized mark4 scan or experiment directory: "
                      << input_dir << eom);
        // Clean up the empty temp dir we just created
        nftw(temp_root.c_str(), remove_entry, 64, FTW_DEPTH | FTW_PHYS);
        return "";
    }

    return temp_root;
}

void MHO_Mk4InputConverter::cleanup_mk4_temp_dir(const std::string& temp_dir)
{
    if(temp_dir.empty())
    {
        return;
    }
    msg_status("fringe", "removing mark4 input temp directory: " << temp_dir << eom);
    nftw(temp_dir.c_str(), remove_entry, 64, FTW_DEPTH | FTW_PHYS);
}

}//end namespace
